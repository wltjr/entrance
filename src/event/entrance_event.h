#ifndef ENTRANCE_EVENT_
#define ENTRANCE_EVENT_

typedef enum Entrance_Event_Type_
{
   ENTRANCE_EVENT_ACTIONS,
   ENTRANCE_EVENT_ACTION,
   ENTRANCE_EVENT_AUTH,
   ENTRANCE_EVENT_CONF_GUI,
   ENTRANCE_EVENT_CONF_USER,
   ENTRANCE_EVENT_MAXTRIES,
   ENTRANCE_EVENT_PID,
   ENTRANCE_EVENT_POOLS,
   ENTRANCE_EVENT_STATUS,
   ENTRANCE_EVENT_THEMES,
   ENTRANCE_EVENT_UNKNOWN,
   ENTRANCE_EVENT_USERS,
   ENTRANCE_EVENT_XSESSIONS
} Entrance_Event_Type;

typedef struct Entrance_Action_
{
   unsigned char id;
   const char *label;
   const char *icon;
} Entrance_Action;

typedef struct Entrance_Action_Event_
{
   unsigned char action;
} Entrance_Action_Event;

typedef struct Entrance_Actions_Event_
{
   Eina_List *actions;
} Entrance_Actions_Event;

typedef struct Entrance_Auth_Event_
{
   const char *login;
   const char *password;
   const char *session;
   Eina_Bool open_session;
} Entrance_Auth_Event;

typedef struct Entrance_Conf_Gui_Event_
{
   Eina_Bool enabled;
   const char *theme;
   struct
     {
        const char *group;
        const char *path;
     } bg;
   Eina_Bool req_passwd;
   Eina_Bool vkbd_enabled;
} Entrance_Conf_Gui_Event;

typedef struct Entrance_Image_
{
   const char *group;
   const char *path;
} Entrance_Image;

typedef struct Entrance_Login_
{
   const char *login;
   const char *lsess;
   Entrance_Image bg;
   Entrance_Image image;
   Eina_Bool remember_session;
   Eina_Bool tmp_icon;
   Eina_List *icon_pool;
   Eina_List *background_pool;
} Entrance_Login;

typedef struct Entrance_Maxtries_Event_
{
   int maxtries;
} Entrance_Maxtries_Event;

typedef struct Entrance_Pid_Event_
{
   pid_t pid;
} Entrance_Pid_Event;

typedef struct Entrance_Pools_
{
   Eina_List *icon_pool;
   Eina_List *background_pool;
} Entrance_Pools;

typedef struct Entrance_Status_Event_
{
   const char *login;
   int granted;
} Entrance_Status_Event;

typedef struct Entrance_Themes_
{
   Eina_List *themes;
} Entrance_Themes;

typedef struct Entrance_Users_Event_
{
   Eina_List *users;
} Entrance_Users_Event;

typedef struct Entrance_Xsession_
{
    const char *name;
    const char *command;
    const char *icon;
    Eina_Bool is_wayland;  /* TRUE for wayland sessions, FALSE for X11 */
} Entrance_Xsession;

typedef struct Entrance_Xsessions_Event_
{
   Eina_List *xsessions;
} Entrance_Xsessions_Event;

typedef struct Entrance_Event_
{
   Entrance_Event_Type type;
   union
     {
        Entrance_Action_Event action;
        Entrance_Actions_Event actions;
        Entrance_Auth_Event auth;
        Entrance_Conf_Gui_Event conf_gui;
        Entrance_Login conf_user;
        Entrance_Maxtries_Event maxtries;
        Entrance_Pid_Event pid;
        Entrance_Pools pools;
        Entrance_Status_Event status;
        Entrance_Themes themes;
        Entrance_Users_Event users;
        Entrance_Xsessions_Event xsessions;
     } event;
} Entrance_Event;

void entrance_event_init(Eet_Read_Cb func_read_cb, Eet_Write_Cb func_write_cb, void *func_data);
void entrance_event_shutdown(void);
void entrance_event_send(const Entrance_Event *data);
void entrance_event_received(const void *data, size_t size);
Eet_Data_Descriptor *entrance_event_user_dd(void);
#endif /* ENTRANCE_EVENT_ */
