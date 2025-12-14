#include <Eina.h>
#include <Eet.h>
#include "entrance_event.h"

#define EET_LOGIN_ADD(NAME, TYPE) \
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Login, # NAME, NAME, TYPE);

#define ENTRANCE_EVENT_ACTION_NAME "EntranceEventAction"
#define ENTRANCE_EVENT_ACTIONS_NAME "EntranceEventActions"
#define ENTRANCE_EVENT_AUTH_NAME "EntranceEventAuth"
#define ENTRANCE_EVENT_CONF_GUI_NAME "EntranceEventConfGui"
#define ENTRANCE_EVENT_CONF_USER_NAME "EntranceEventConfUser"
#define ENTRANCE_EVENT_MAXTRIES_NAME "EntranceEventMaxtries"
#define ENTRANCE_EVENT_PID_NAME "EntranceEventPid"
#define ENTRANCE_EVENT_POOLS_NAME "EntranceEventPools"
#define ENTRANCE_EVENT_STATUS_NAME "EntranceEventStatus"
#define ENTRANCE_EVENT_THEMES_NAME "EntranceEventThemes"
#define ENTRANCE_EVENT_USERS_NAME "EntranceEventUsers"
#define ENTRANCE_EVENT_XSESSIONS_NAME "EntranceEventSession"

static const char *_entrance_event_type_get(const void *data, Eina_Bool *unknown);
static Eina_Bool _entrance_event_type_set(const char *type, void *data, Eina_Bool unknown);
static Eet_Data_Descriptor *_entrance_event_action_dd(void);
static Eet_Data_Descriptor *_entrance_event_actions_dd(void);
static Eet_Data_Descriptor *_entrance_event_auth_dd(void);
static Eet_Data_Descriptor *_entrance_event_conf_gui_dd(void);
static Eet_Data_Descriptor *_entrance_event_conf_user_dd(Eina_Bool stream);
static Eet_Data_Descriptor *_entrance_event_image_dd(void);
static Eet_Data_Descriptor *_entrance_event_maxtries_dd(void);
static Eet_Data_Descriptor *_entrance_event_new(void);
static Eet_Data_Descriptor *_entrance_event_pid_dd(void);
static Eet_Data_Descriptor *_entrance_event_status_dd(void);
static Eet_Data_Descriptor *_entrance_event_users_dd(void);
static Eet_Data_Descriptor *_entrance_event_xsessions_dd(void);
static Eina_Bool _entrance_event_read_cb(const void *data, size_t size, void *user_data);


typedef struct _Entrance_Event_Private {
    Eet_Data_Descriptor *event_descriptor;
    Eet_Connection *event_connection;
    Eet_Read_Cb *func_read_cb;
} Entrance_Event_Private;

static Entrance_Event_Private *_eep = NULL;

static Eina_Bool
_entrance_event_type_set(const char *type, void *data, Eina_Bool unknown)
{
   Entrance_Event_Type *ev = data;
   if (unknown)
     return EINA_FALSE;
   if (!strcmp(type, ENTRANCE_EVENT_ACTION_NAME))
     *ev = ENTRANCE_EVENT_ACTION;
   else if (!strcmp(type, ENTRANCE_EVENT_ACTIONS_NAME))
     *ev = ENTRANCE_EVENT_ACTIONS;
   else if (!strcmp(type, ENTRANCE_EVENT_AUTH_NAME))
     *ev = ENTRANCE_EVENT_AUTH;
   else if (!strcmp(type, ENTRANCE_EVENT_CONF_GUI_NAME))
     *ev = ENTRANCE_EVENT_CONF_GUI;
   else if (!strcmp(type, ENTRANCE_EVENT_CONF_USER_NAME))
     *ev = ENTRANCE_EVENT_CONF_USER;
   else if (!strcmp(type, ENTRANCE_EVENT_MAXTRIES_NAME))
     *ev = ENTRANCE_EVENT_MAXTRIES;
   else if (!strcmp(type, ENTRANCE_EVENT_PID_NAME))
     *ev = ENTRANCE_EVENT_PID;
   else if (!strcmp(type, ENTRANCE_EVENT_POOLS_NAME))
     *ev = ENTRANCE_EVENT_POOLS;
   else if (!strcmp(type, ENTRANCE_EVENT_STATUS_NAME))
     *ev = ENTRANCE_EVENT_STATUS;
   else if (!strcmp(type, ENTRANCE_EVENT_THEMES_NAME))
     *ev = ENTRANCE_EVENT_THEMES;
   else if (!strcmp(type, ENTRANCE_EVENT_USERS_NAME))
     *ev = ENTRANCE_EVENT_USERS;
   else if (!strcmp(type, ENTRANCE_EVENT_XSESSIONS_NAME))
     *ev = ENTRANCE_EVENT_XSESSIONS;
   else
     {
        printf("error on type set %s\n", type);
        *ev = ENTRANCE_EVENT_UNKNOWN;
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static const char *
_entrance_event_type_get(const void *data, Eina_Bool *unknown)
{
  const Entrance_Event_Type *ev = data;
  switch(*ev)
    {
      case ENTRANCE_EVENT_ACTION:
        return ENTRANCE_EVENT_ACTION_NAME;
      case ENTRANCE_EVENT_ACTIONS:
        return ENTRANCE_EVENT_ACTIONS_NAME;
      case ENTRANCE_EVENT_AUTH:
        return ENTRANCE_EVENT_AUTH_NAME;
      case ENTRANCE_EVENT_CONF_GUI:
        return ENTRANCE_EVENT_CONF_GUI_NAME;
      case ENTRANCE_EVENT_CONF_USER:
        return ENTRANCE_EVENT_CONF_USER_NAME;
      case ENTRANCE_EVENT_MAXTRIES:
        return ENTRANCE_EVENT_MAXTRIES_NAME;
      case ENTRANCE_EVENT_PID:
        return ENTRANCE_EVENT_PID_NAME;
      case ENTRANCE_EVENT_POOLS:
        return ENTRANCE_EVENT_POOLS_NAME;
      case ENTRANCE_EVENT_STATUS:
        return ENTRANCE_EVENT_STATUS_NAME;
      case ENTRANCE_EVENT_THEMES:
        return ENTRANCE_EVENT_THEMES_NAME;
      case ENTRANCE_EVENT_USERS:
        return ENTRANCE_EVENT_USERS_NAME;
      case ENTRANCE_EVENT_XSESSIONS:
        return ENTRANCE_EVENT_XSESSIONS_NAME;
      default:
        printf("error on type get %d\n", *ev);
        if (unknown)
          *unknown = EINA_TRUE;
        break;
    }
  return NULL;
}

static Eet_Data_Descriptor *
_entrance_event_xsessions_dd(void)
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor *eddl;
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor_Class eddcl;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Xsession);
   edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Xsession, "name",
                                 name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Xsession, "icon",
                                 icon, EET_T_STRING);

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddcl, Entrance_Xsessions_Event);
   eddl = eet_data_descriptor_stream_new(&eddcl);
   EET_DATA_DESCRIPTOR_ADD_LIST(eddl, Entrance_Xsessions_Event, "xsessions",
                                xsessions, edd);
   return eddl;
}

static Eet_Data_Descriptor *
_entrance_event_auth_dd(void)
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor_Class eddc;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Auth_Event);
   edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Auth_Event, "login",
                                 login, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Auth_Event, "password",
                                 password, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Auth_Event, "session",
                                 session, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Auth_Event, "open_session",
                                 open_session, EET_T_UCHAR);
   return edd;

}

static Eet_Data_Descriptor *
_entrance_event_maxtries_dd(void)
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor_Class eddc;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Maxtries_Event);
   edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Maxtries_Event, "maxtries",
                                 maxtries, EET_T_INT);
   return edd;
}

static Eet_Data_Descriptor *
_entrance_event_conf_gui_dd(void)
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor_Class eddc;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc,
                                             Entrance_Conf_Gui_Event);
   edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Conf_Gui_Event, "enabled",
                                 enabled, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Conf_Gui_Event, "bg.path",
                                 bg.path, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Conf_Gui_Event, "bg.group",
                                 bg.group, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Conf_Gui_Event, "req_passwd",
                                 req_passwd, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Conf_Gui_Event, "vkbd_enabled",
                                 vkbd_enabled, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Conf_Gui_Event, "theme",
                                 theme, EET_T_STRING);
   return edd;
}

static Eet_Data_Descriptor *
_entrance_event_themes_dd(void)
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor_Class eddc;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Themes);
   edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_LIST_STRING(edd, Entrance_Themes, "themes",
                                 themes);
   return edd;
}

static Eet_Data_Descriptor *
_entrance_event_status_dd(void)
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor_Class eddc;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Status_Event);
   edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Status_Event, "login",
                                 login, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Status_Event, "granted",
                                 granted, EET_T_INT);
   return edd;

}

static Eet_Data_Descriptor *
_entrance_event_image_dd(void)
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor_Class eddc;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Image);
   edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Image, "path",
                                 path, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Image, "group",
                                 group, EET_T_INT);
   return edd;

}

static Eet_Data_Descriptor *
_entrance_event_users_dd(void)
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor *eddl;
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor_Class eddcl;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Login);
   edd = _entrance_event_conf_user_dd(EINA_TRUE);
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddcl, Entrance_Users_Event);
   eddl = eet_data_descriptor_stream_new(&eddcl);
   EET_DATA_DESCRIPTOR_ADD_LIST(eddl, Entrance_Users_Event, "users",
                                users, edd);
   return eddl;
}

static Eet_Data_Descriptor *
_entrance_event_conf_user_dd(Eina_Bool stream)
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor *eddi;
   Eet_Data_Descriptor_Class eddc;
   eddi = _entrance_event_image_dd();
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Login);
   edd = eet_data_descriptor_stream_new(&eddc);
   EET_LOGIN_ADD(login, EET_T_STRING);
   EET_LOGIN_ADD(image.path, EET_T_STRING);
   EET_LOGIN_ADD(image.group, EET_T_STRING);
   EET_LOGIN_ADD(bg.path, EET_T_STRING);
   EET_LOGIN_ADD(bg.group, EET_T_STRING);
   EET_LOGIN_ADD(lsess, EET_T_STRING);
   EET_LOGIN_ADD(remember_session, EET_T_INT);

   if (stream)
     {
        EET_DATA_DESCRIPTOR_ADD_LIST(edd, Entrance_Login, "icon_pool",
                                     icon_pool, eddi);
        EET_DATA_DESCRIPTOR_ADD_LIST(edd, Entrance_Login, "background_pool",
                                     background_pool, eddi);
     }
   return edd;
}

static Eet_Data_Descriptor *
_entrance_event_actions_dd(void)
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor *eddl;
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor_Class eddcl;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Action);
   edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Action, "label",
                                 label, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Action, "id",
                                 id, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Action, "icon",
                                 icon, EET_T_STRING);
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddcl, Entrance_Actions_Event);
   eddl = eet_data_descriptor_stream_new(&eddcl);
   EET_DATA_DESCRIPTOR_ADD_LIST(eddl, Entrance_Actions_Event, "actions",
                                actions, edd);
   return eddl;
}

static Eet_Data_Descriptor *
_entrance_event_action_dd(void)
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor_Class eddc;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Status_Event);
   edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Action_Event, "action",
                                 action, EET_T_UCHAR);
   return edd;
}

static Eet_Data_Descriptor *
_entrance_event_pid_dd(void)
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor_Class eddc;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Pid_Event);
   edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Pid_Event, "pid", pid, EET_T_INT);
   return edd;
}

static Eet_Data_Descriptor *
_entrance_event_pools_dd(void)
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor *eddi;
   Eet_Data_Descriptor_Class eddc;
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Pools);
   edd = eet_data_descriptor_stream_new(&eddc);
   eddi = _entrance_event_image_dd();

   EET_DATA_DESCRIPTOR_ADD_LIST(edd, Entrance_Pools, "icon_pool",
                                icon_pool, eddi);
   EET_DATA_DESCRIPTOR_ADD_LIST(edd, Entrance_Pools, "background_pool",
                                background_pool, eddi);
   return edd;
}


static Eet_Data_Descriptor *
_entrance_event_new(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor *unified;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Event);
   edd = eet_data_descriptor_stream_new(&eddc);

   eddc.version = EET_DATA_DESCRIPTOR_CLASS_VERSION;
   eddc.func.type_get = _entrance_event_type_get;
   eddc.func.type_set = _entrance_event_type_set;
   unified = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_MAPPING(unified, ENTRANCE_EVENT_ACTION_NAME,
                                   _entrance_event_action_dd());
   EET_DATA_DESCRIPTOR_ADD_MAPPING(unified, ENTRANCE_EVENT_ACTIONS_NAME,
                                   _entrance_event_actions_dd());
   EET_DATA_DESCRIPTOR_ADD_MAPPING(unified, ENTRANCE_EVENT_AUTH_NAME,
                                   _entrance_event_auth_dd());
   EET_DATA_DESCRIPTOR_ADD_MAPPING(unified, ENTRANCE_EVENT_CONF_GUI_NAME,
                                   _entrance_event_conf_gui_dd());
   EET_DATA_DESCRIPTOR_ADD_MAPPING(unified, ENTRANCE_EVENT_CONF_USER_NAME,
                                   _entrance_event_conf_user_dd(EINA_TRUE));
   EET_DATA_DESCRIPTOR_ADD_MAPPING(unified, ENTRANCE_EVENT_MAXTRIES_NAME,
                                   _entrance_event_maxtries_dd());
   EET_DATA_DESCRIPTOR_ADD_MAPPING(unified, ENTRANCE_EVENT_PID_NAME,
                                   _entrance_event_pid_dd());
   EET_DATA_DESCRIPTOR_ADD_MAPPING(unified, ENTRANCE_EVENT_POOLS_NAME,
                                   _entrance_event_pools_dd());
   EET_DATA_DESCRIPTOR_ADD_MAPPING(unified, ENTRANCE_EVENT_STATUS_NAME,
                                   _entrance_event_status_dd());
   EET_DATA_DESCRIPTOR_ADD_MAPPING(unified, ENTRANCE_EVENT_USERS_NAME,
                                   _entrance_event_users_dd());
   EET_DATA_DESCRIPTOR_ADD_MAPPING(unified, ENTRANCE_EVENT_THEMES_NAME,
                                   _entrance_event_themes_dd());
   EET_DATA_DESCRIPTOR_ADD_MAPPING(unified, ENTRANCE_EVENT_XSESSIONS_NAME,
                                   _entrance_event_xsessions_dd());

   EET_DATA_DESCRIPTOR_ADD_UNION(edd, Entrance_Event, "event",
                                 event, type, unified);
   return edd;
}

static Eina_Bool
_entrance_event_read_cb(const void *data, size_t size, void *user_data EINA_UNUSED)
{
   void *ev;
   ev = eet_data_descriptor_decode(_eep->event_descriptor, data, (int)size);
   if (_eep->func_read_cb)
     _eep->func_read_cb(ev, size, user_data);
   return EINA_TRUE;
}

void
entrance_event_init(Eet_Read_Cb func_read_cb, Eet_Write_Cb func_write_cb, void *func_data)
{
   _eep = calloc(1, sizeof(Entrance_Event_Private));
   _eep->func_read_cb = func_read_cb;
   _eep->event_descriptor = _entrance_event_new();
   _eep->event_connection = eet_connection_new(_entrance_event_read_cb,
                                               func_write_cb,
                                               func_data);
}

void
entrance_event_shutdown(void)
{
   eet_connection_close(_eep->event_connection, NULL);
   eet_data_descriptor_free(_eep->event_descriptor);
   free(_eep);
}

void
entrance_event_send(const Entrance_Event *data)
{
   eet_connection_send(_eep->event_connection, _eep->event_descriptor,
                       data, NULL);
}

void
entrance_event_received(const void *data, size_t size)
{
   eet_connection_received(_eep->event_connection, data, size);
}

Eet_Data_Descriptor *
entrance_event_user_dd(void)
{
  //this is used extern for the history!
   return _entrance_event_conf_user_dd(EINA_FALSE);
}
