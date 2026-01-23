#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "entrance.h"
#include "entrance_pam.h"

static int _entrance_pam_conv(int num_msg,
                              const struct pam_message **msg,
                              struct pam_response **resp,
                              void *appdata_ptr);

static pam_handle_t* _pam_handle = NULL;
static int last_result;
static char *_passwd = NULL;

static int
_entrance_pam_conv(int num_msg,
                   const struct pam_message **msg,
                   struct pam_response **resp,
                   void *appdata_ptr EINA_UNUSED)
{
   *resp = (struct pam_response *) calloc(num_msg, sizeof(struct pam_response));
   if (!*resp) return PAM_CONV_ERR;
   for (int i = 0; i < num_msg; i++)
     {
        (*resp)[i].resp_retcode=0;
        switch(msg[i]->msg_style)
          {
           case PAM_PROMPT_ECHO_ON:
              break;
           case PAM_PROMPT_ECHO_OFF:
              PT("echo off");
              (*resp)[i].resp = _passwd;
              break;
           case PAM_ERROR_MSG:
              PT("error msg %s", msg[i]->msg);
              break;
           case PAM_TEXT_INFO:
              PT("info %s", msg[i]->msg);
              break;
           default:
              break;
          }
     }
   return PAM_SUCCESS;
}

int
entrance_pam_open_session(void)
{
   last_result = pam_setcred(_pam_handle, PAM_ESTABLISH_CRED);
   switch (last_result)
     {
      case PAM_CRED_ERR:
      case PAM_USER_UNKNOWN:
         PT("PAM user unknow");
         return 1;
      case PAM_AUTH_ERR:
      case PAM_PERM_DENIED:
         PT("PAM error on login password");
         return 1;
      case PAM_SUCCESS:
         break;
      default:
         PT("PAM open warning unknown error");
         return 1;
     }
   last_result = pam_open_session(_pam_handle, 0);
   if(last_result!=PAM_SUCCESS)
     {
       pam_setcred(_pam_handle, PAM_DELETE_CRED);
       entrance_pam_end();
     }
   return 0;
}

void
entrance_pam_close_session(const Eina_Bool opened)
{
   PT("PAM close session");
   last_result = pam_close_session(_pam_handle, PAM_SILENT);
   if(last_result!=PAM_SUCCESS)
     {
       PT("error on close session");
       pam_setcred(_pam_handle, PAM_DELETE_CRED);
       entrance_pam_end();
     }
   if (opened)
     {
       last_result = pam_setcred(_pam_handle, PAM_DELETE_CRED);
       if(last_result!=PAM_SUCCESS)
         entrance_pam_end();
     }
}

int
entrance_pam_end(void)
{
   int result;
   result = pam_end(_pam_handle, last_result);
   _pam_handle = NULL;
   return result;
}

int
entrance_pam_authenticate(void)
{
   last_result = pam_authenticate(_pam_handle, 0);
   switch (last_result)
     {
      case PAM_ABORT:
      case PAM_AUTHINFO_UNAVAIL:
         PT("PAM error !");
         entrance_pam_end();
         return 1;
      case PAM_USER_UNKNOWN:
         PT("PAM user unknown error !");
         return 1;
      case PAM_MAXTRIES:
         PT("PAM max tries error !");
         entrance_server_client_wait();
         return 1;
      case PAM_CRED_INSUFFICIENT:
         PT("PAM don't have sufficient credential to authenticate !");
         return 1;
      case PAM_AUTH_ERR:
         PT("PAM authenticate error !");
         return 1;
      case PAM_PERM_DENIED:
         PT("PAM permission denied !");
         return 1;
      case PAM_SUCCESS:
         break;
      default:
         PT("PAM auth warning unknown error");
         return 1;
     }
   last_result=pam_acct_mgmt(_pam_handle, PAM_SILENT);
   switch(last_result)
     {
      case PAM_ACCT_EXPIRED:
         PT("PAM user acct expired error");
         entrance_pam_end();
         return 1;
      case PAM_USER_UNKNOWN:
         PT("PAM user unknown error");
         entrance_pam_end();
         return 1;
      case PAM_AUTH_ERR:
         PT("PAM auth error");
         return 1;
      case PAM_PERM_DENIED:
         PT("PAM perm_denied error");
         return 1;
      case PAM_SUCCESS:
         break;
      default:
         PT("PAM auth warning unknown error");
         return 1;
     }

   return 0;
}

int
entrance_pam_init(const char *service, const char *tty, const char *user)
{
   int status;

   if (!tty || !*tty) goto pam_error;

   struct pam_conv pam_conversation = { _entrance_pam_conv, NULL };

   if (_pam_handle) entrance_pam_end();
   status = pam_start(service ? service : PACKAGE, user, &pam_conversation, &_pam_handle);
   if (status != 0) goto pam_error;
   status = entrance_pam_item_set(ENTRANCE_PAM_ITEM_TTY, tty);
   if (status != 0) goto pam_error;
   status = entrance_pam_item_set(ENTRANCE_PAM_ITEM_RUSER, user);
   if (status != 0) goto pam_error;
   return 0;

pam_error:
   PT("PAM error !!!");
   return 1;
}

int
entrance_pam_item_set(ENTRANCE_PAM_ITEM_TYPE type, const void *value)
{
   last_result = pam_set_item(_pam_handle, type, value);
   if (last_result == PAM_SUCCESS) {
      return 0;
   }

   PT("PAM error: %d on %d", last_result, type);
   return 1;
}

const void *
entrance_pam_item_get(ENTRANCE_PAM_ITEM_TYPE type)
{
   const void *data;
   last_result = pam_get_item(_pam_handle, type, &data);
   if(last_result!=PAM_SUCCESS)
     {
        PT("error on pam item get");
        entrance_pam_end();
     }
   return data;
}

int
entrance_pam_env_set(const char *env, const char *value)
{
   char buf[1024];
   if (!env || !value) return 1;
   snprintf(buf, sizeof(buf), "%s=%s", env, value);
   last_result = pam_putenv(_pam_handle, buf);
   if(last_result!=PAM_SUCCESS)
     {
       entrance_pam_end();
       return 1;
     }
   return 0;

}

char **
entrance_pam_env_list_get(void)
{
   return pam_getenvlist(_pam_handle);
}

void
entrance_pam_shutdown(void)
{
   if(_passwd)
      free(_passwd);
    _passwd = NULL;
}

int
entrance_pam_passwd_set(const char *passwd)
{
   _passwd = strdup(passwd);
   if (!_passwd)
     return 1;
   return 0;
}

