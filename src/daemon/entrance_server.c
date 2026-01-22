#include <Ecore_Con.h>
#include "entrance.h"
#include "../event/entrance_event.h"

static Eina_Bool _entrance_server_add(void *data, int type, void *event);
static Eina_Bool _entrance_server_del(void *data, int type, void *event);
static Eina_Bool _entrance_server_data(void *data, int type, void *event);


Ecore_Con_Server *_entrance_server = NULL;
Eina_List *_handlers = NULL;

static Eina_Bool
_entrance_server_add(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   Entrance_Event eev;

   PT("server client connected");
   PT("Sending users");
   eev.type = ENTRANCE_EVENT_USERS;
   eev.event.users.users = entrance_history_get();
   entrance_event_send(&eev);

   PT("Sending actions");
   eev.type = ENTRANCE_EVENT_ACTIONS;
   eev.event.actions.actions = entrance_action_get();
   entrance_event_send(&eev);
   if (entrance_config->xsessions)
     {
        PT("Sending xsessions");
        eev.type = ENTRANCE_EVENT_XSESSIONS;
        eev.event.xsessions.xsessions = entrance_session_list_get();
        entrance_event_send(&eev);
     }
   if (entrance_config->custom_conf)
     {
        PT("Sending custom settings is enabled");
        eev.type = ENTRANCE_EVENT_CONF_GUI;
        eev.event.conf_gui.enabled = EINA_TRUE;
        eev.event.conf_gui.bg.path = entrance_config->bg.path;
        eev.event.conf_gui.bg.group = entrance_config->bg.group;
        eev.event.conf_gui.req_passwd = entrance_config->req_passwd;
        eev.event.conf_gui.autoselect_last_user = entrance_config->autoselect_last_user;
        eev.event.conf_gui.theme = NULL;
        entrance_event_send(&eev);
     }
   PT("Sending pid");
   eev.type = ENTRANCE_EVENT_PID;
   eev.event.pid.pid = getpid();
   entrance_event_send(&eev);
   PT("Sending pools");
   eev.type = ENTRANCE_EVENT_POOLS;
   eev.event.pools.icon_pool = entrance_image_system_icons();
   eev.event.pools.background_pool = entrance_image_system_backgrounds();
   entrance_event_send(&eev);
   PT("Sending themes");
   eev.type = ENTRANCE_EVENT_THEMES;
   eev.event.themes.themes = entrance_theme_themes_get();
   entrance_event_send(&eev);
   return ECORE_CALLBACK_RENEW;
}


static Eina_Bool
_entrance_server_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   PT("server client disconnected");

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_entrance_server_data(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{

   Ecore_Con_Event_Client_Data *ev;

   ev = event;
   entrance_event_received(ev->data, ev->size);

   return ECORE_CALLBACK_RENEW;
}


static Eina_Bool
_entrance_server_read_cb(const void *data, size_t size EINA_UNUSED, void *user_data EINA_UNUSED)
{
   const Entrance_Event *eev;
   Entrance_Event neev;
   eev = data;
   if (eev->type == ENTRANCE_EVENT_AUTH)
     {
        neev.type = ENTRANCE_EVENT_STATUS;
        if (entrance_session_authenticate(eev->event.auth.login,
                                          eev->event.auth.password))
          {
             PT("server authenticate granted");
             neev.event.status.login = entrance_session_login_get();
             neev.event.status.granted = EINA_TRUE;
             if (eev->event.auth.open_session)
               {
                  PT("opening session now ...");
                  entrance_session_login(eev->event.auth.session, EINA_TRUE);
               }
#ifdef HAVE_PAM
             else
               entrance_pam_end();
#endif
          }
        else
          {
#ifdef HAVE_PAM
             entrance_pam_end();
#endif
             neev.event.status.login = NULL;
             neev.event.status.granted = EINA_FALSE;
             PT("server authenticate error");
          }
        entrance_event_send(&neev);

     }
   else if (eev->type == ENTRANCE_EVENT_ACTION)
     {
        PT("Action received");
        entrance_action_run(eev->event.action.action);
     }
   else if (eev->type == ENTRANCE_EVENT_CONF_GUI)
     {
        PT("Conf Gui received");
        entrance_config_set(&eev->event.conf_gui);
     }
   else if (eev->type == ENTRANCE_EVENT_CONF_USER)
     {
        PT("Conf user received");
        entrance_history_user_update(&eev->event.conf_user);
     }
   else if (eev->type == ENTRANCE_EVENT_PID)
     {
        PT("PID received");
        entrance_client_pid_set(eev->event.pid.pid);
     }
   else
     PT("UNKNOWN signal server");
   return EINA_TRUE;
}

static Eina_Bool
_entrance_server_write_cb(const void *data, size_t size, void *user_data EINA_UNUSED)
{
   const Eina_List *l;
   Ecore_Con_Client *ecc;
   EINA_LIST_FOREACH(ecore_con_server_clients_get(_entrance_server), l, ecc)
      ecore_con_client_send(ecc, data, (int)size);

   return EINA_TRUE;
}

#ifdef HAVE_LOGIND
/* Callback for logind seat/session changes */
static void
_entrance_logind_monitor_cb(void *data EINA_UNUSED)
{
   PT("Logind event detected - seat or session change");
   /* Could trigger UI updates, rescan seats, etc. */
}
#endif

void
entrance_server_init(gid_t uid, uid_t gid)
{
   Ecore_Event_Handler *h;
   char path[1024] = {0};

   ecore_con_init();
   entrance_event_init(_entrance_server_read_cb,
                       _entrance_server_write_cb,
                       NULL);

   _entrance_server = ecore_con_server_add(ECORE_CON_LOCAL_SYSTEM,
                                           "entrance", 
                                           entrance_config->port,
                                           NULL);
   snprintf(path, sizeof(path),
            "/tmp/.ecore_service|entrance|%d", entrance_config->port);
   if (!_entrance_server)
     PT("server init fail");
   else
     PT("chown %d:%d %s",uid,gid, path);
   if(chown(path,uid,gid)!=0) {
     if(errno==ENOENT)
       PT("chown failed, file does not exist %s", path);
     else
       PT("chown failed %s", path);
   }

   h = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD,
                               _entrance_server_add, NULL);
   _handlers = eina_list_append(_handlers, h);
   h = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL,
                               _entrance_server_del, NULL);
   _handlers = eina_list_append(_handlers, h);
   h = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA,
                               _entrance_server_data, NULL);
   _handlers = eina_list_append(_handlers, h);

#ifdef HAVE_LOGIND
   /* Note: logind_init() is called in entrance_session_init() */
   /* Start monitoring for seat/session changes */
   if (!entrance_logind_monitor_start(_entrance_logind_monitor_cb, NULL))
     PT("WARNING: Could not start logind monitor");
#endif
}

void
entrance_server_shutdown(void)
{
   Ecore_Event_Handler *h;
#ifdef HAVE_LOGIND
   /* Stop monitoring only - full shutdown in entrance_session_shutdown() */
   entrance_logind_monitor_stop();
#endif
   if (_entrance_server)
     ecore_con_server_del(_entrance_server);
   EINA_LIST_FREE(_handlers, h)
     ecore_event_handler_del(h);
   entrance_event_shutdown();
   ecore_con_shutdown();
}

void
entrance_server_client_wait(void)
{
   Entrance_Event eev;
   eev.type = ENTRANCE_EVENT_MAXTRIES;
   eev.event.maxtries.maxtries = EINA_TRUE;
   entrance_event_send(&eev);
}

