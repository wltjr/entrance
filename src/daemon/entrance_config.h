#ifndef ENTRANCE_CONFIG_H_
#define ENTRANCE_CONFIG_H_

#define ENTRANCE_CONFIG_FILE        "entrance.cfg"

typedef struct _Entrance_Config Entrance_Config;

struct _Entrance_Config
{
   unsigned short port;
   const char *session;
   const char *session_path;
   struct
     {
        const char *xinit_path;
        const char *xinit_args;
        const char *xauth_path;
        const char *xauth_file;
        const char *xdisplay;
        const char *session_start;
        const char *session_login;
        const char *session_stop;
        const char *shutdown;
        const char *reboot;
        const char *suspend;
        unsigned short vtnr;
     } command;
   const char *userlogin;
   const char *lockfile;
   const char *logfile;
   const char *theme;
   const char *elm_profile;
   struct
     {
        const char *path;
        const char *group;
     } bg;
   Eina_Bool xsessions;
   Eina_Bool daemonize;
   Eina_Bool numlock;
   Eina_Bool autologin;
   Eina_Bool custom_conf;
   Eina_Bool vkbd_enabled;
   Eina_Bool req_passwd;
   Eina_Bool autoselect_last_user;
   const char *start_user;
};

void entrance_config_init(void);
void entrance_config_shutdown(void);
void entrance_config_set(const Entrance_Conf_Gui_Event *conf);

extern Entrance_Config *entrance_config;

#endif /* ENTRANCE_CONFIG_H_ */
