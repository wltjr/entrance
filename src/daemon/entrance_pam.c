#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "entrance.h"
#include "entrance_pam.h"

typedef struct Entrance_Pam_
{
    long id;
    char *passwd;
    int last_result;
    Eina_Bool opened;
    pam_handle_t* handle;
} Entrance_Pam;

static int _entrance_pam_result_check(const char *event, int last_result);
static int _entrance_pam_conv(int num_msg,
                              const struct pam_message **msg,
                              struct pam_response **resp,
                              void *appdata_ptr);

static Entrance_Pam **_pams;

static int
_entrance_pam_result_check(const char *event, int last_result)
{
    switch (last_result)
    {
        case PAM_SUCCESS:
            return 0;
        case PAM_ABORT:
        case PAM_AUTHINFO_UNAVAIL:
            PT("PAM error!");
            break;
        case PAM_PERM_DENIED:
            PT("PAM permission denied!");
            break;
        case PAM_AUTH_ERR:
            PT("PAM authenticate error!");
            break;
        case PAM_CRED_INSUFFICIENT:
            PT("PAM insufficient credentials to authenticate!");
            break;
        case PAM_USER_UNKNOWN:
            PT("PAM user unknown error!");
            break;
        case PAM_MAXTRIES:
            PT("PAM max tries error !");
            break;
        case PAM_ACCT_EXPIRED:
            PT("PAM user acct expired error");
            break;
        default:
            PT("PAM %s unknown error %d", event, last_result);
    }
    return 1;
}

static int
_entrance_pam_conv(int num_msg,
                   const struct pam_message **msg,
                   struct pam_response **resp,
                   void *appdata_ptr)
{
   const long id = *(int *)appdata_ptr;
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
              /* PAM will free this, so we must duplicate it */
              (*resp)[i].resp = _pams[id]->passwd ? strdup(_pams[id]->passwd) : NULL;
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
entrance_pam_session_open(int id)
{
   _pams[id]->last_result = pam_setcred(_pams[id]->handle, PAM_ESTABLISH_CRED);
   switch (_pams[id]->last_result)
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
         PT("PAM open warning unknown error %d", _pams[id]->last_result);
         return 1;
     }
   _pams[id]->last_result = pam_open_session(_pams[id]->handle, 0);
   if(_pams[id]->last_result!=PAM_SUCCESS)
     {
       pam_setcred(_pams[id]->handle, PAM_DELETE_CRED);
       entrance_pam_end(id);
     }
   _pams[id]->opened = EINA_TRUE;
   return 0;
}

void
entrance_pam_session_close(int id)
{
   PT("PAM close session #%d", id);
   _pams[id]->last_result = pam_close_session(_pams[id]->handle, PAM_SILENT);
   if(_pams[id]->opened || _pams[id]->last_result!=PAM_SUCCESS)
     {
       _pams[id]->last_result = pam_setcred(_pams[id]->handle, PAM_DELETE_CRED);
       if(_pams[id]->last_result!=PAM_SUCCESS)
         entrance_pam_end(id);
     }
}

int
entrance_pam_end(int id)
{
   int result;
   result = pam_end(_pams[id]->handle, _pams[id]->last_result);
   _pams[id]->handle = NULL;
   return result;
}

int
entrance_pam_authenticate(int id)
{
   _pams[id]->last_result = pam_authenticate(_pams[id]->handle, 0);
   if(_entrance_pam_result_check("auth", _pams[id]->last_result))
      return 1;
   _pams[id]->last_result=pam_acct_mgmt(_pams[id]->handle, PAM_SILENT);
   if(_entrance_pam_result_check("auth", _pams[id]->last_result))
      return 1;

   return 0;
}

void
entrance_pams_init(int count)
{
    _pams = (Entrance_Pam**) calloc(count, sizeof(Entrance_Pam*));
    PT("Allocated memory for %d pam sessions", count);
}

Eina_Bool
entrance_pam_start(int id, const char *service, const char *tty, const char *user)
{
   int status;

   if (!tty || !*tty) goto pam_error;

   if (_pams[id] && _pams[id]->handle) entrance_pam_end(id);
   _pams[id] = calloc(1, sizeof(Entrance_Pam));
   _pams[id]->id = id;
   _pams[id]->opened = EINA_FALSE;

   struct pam_conv pam_conversation = { _entrance_pam_conv, &_pams[id]->id };

   status = pam_start(service ? service : PACKAGE, user, &pam_conversation, &_pams[id]->handle);
   if (status != 0) goto pam_error;
   status = entrance_pam_item_set(id, ENTRANCE_PAM_ITEM_TTY, tty);
   if (status != 0) goto pam_error;
   status = entrance_pam_item_set(id, ENTRANCE_PAM_ITEM_RUSER, user);
   if (status != 0) goto pam_error;
   return 0;

pam_error:
   PT("PAM error !!!");
   return 1;
}

int
entrance_pam_item_set(int id, ENTRANCE_PAM_ITEM_TYPE type, const void *value)
{
   _pams[id]->last_result = pam_set_item(_pams[id]->handle, type, value);
   if (_pams[id]->last_result == PAM_SUCCESS) {
      return 0;
   }

   PT("PAM error: %d on %d", _pams[id]->last_result, type);
   return 1;
}

const void *
entrance_pam_item_get(int id, ENTRANCE_PAM_ITEM_TYPE type)
{
   const void *data;
   _pams[id]->last_result = pam_get_item(_pams[id]->handle, type, &data);
   if(_pams[id]->last_result!=PAM_SUCCESS)
     {
        PT("error on pam item get");
        entrance_pam_end(id);
     }
   return data;
}

int
entrance_pam_env_set(int id, const char *env, const char *value)
{
   char buf[1024];
   if (!env || !value) return 1;
   snprintf(buf, sizeof(buf), "%s=%s", env, value);
   _pams[id]->last_result = pam_putenv(_pams[id]->handle, buf);
   if(_pams[id]->last_result!=PAM_SUCCESS)
     {
       entrance_pam_end(id);
       return 1;
     }
   return 0;
}

char **
entrance_pam_env_list_get(int id)
{
   return pam_getenvlist(_pams[id]->handle);
}

int
entrance_pam_last_result_get(int id)
{
   return _pams[id]->last_result;
}

void
entrance_pam_shutdown(int id)
{
    if(_pams[id])
    {
        free(_pams[id]->passwd);
        _pams[id]->passwd = NULL;
        free(_pams[id]);
    }
}

void
entrance_pams_shutdown()
{
    if(_pams)
        free(_pams);
}

int
entrance_pam_passwd_set(int id, const char *passwd)
{
    _pams[id]->passwd = strdup(passwd);
    if (!_pams[id]->passwd)
        return 1;
    return 0;
}

