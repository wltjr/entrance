#include "entrance.h"
#include <sys/stat.h>

#define ENTRANCE_CONFIG_KEY "config"

static void _defaults_set(Entrance_Config *config);
static void _users_get(void);
static void _config_free(Entrance_Config *config);
static Entrance_Config *_cache_get(void);

static Eet_Data_Descriptor *_entrance_config_descriptor;

Entrance_Config *entrance_config;

static void
_defaults_set(Entrance_Config *config)
{
   config->port = 42;
   config->session = eina_stringshare_add("Enlightenment");
   config->session_path = eina_stringshare_add("/bin:/usr/bin:/usr/local/bin");
   config->command.xinit_path = eina_stringshare_add("/usr/bin/X");
   config->command.xinit_args = eina_stringshare_add("-nolisten tcp -br vt7");
   config->command.xauth_path = eina_stringshare_add("/usr/bin/xauth");
   config->command.xauth_file = eina_stringshare_add("/var/run/entrance.auth");
   config->command.xdisplay = eina_stringshare_add(":0.0");
   config->command.vtnr = 7;
   config->command.session_start = eina_stringshare_add("/usr/bin/sessreg -a -l" );
   config->command.session_login = eina_stringshare_add(SYSTEM_CONFIG_DIR"/entrance/Xsession");
   config->command.session_stop = eina_stringshare_add("/usr/bin/sessreg -d -l");
   config->command.shutdown = eina_stringshare_add("/usr/bin/shutdown -h now");
   config->command.reboot = eina_stringshare_add("/usr/bin/shutdown -r now");
   config->command.suspend = eina_stringshare_add("/usr/sbin/suspend");
   config->daemonize = EINA_TRUE;
   config->numlock = EINA_FALSE;
   config->xsessions = EINA_TRUE;
   config->autologin = EINA_FALSE;
   config->custom_conf = EINA_FALSE;
   config->req_passwd = EINA_TRUE;
   config->userlogin = eina_stringshare_add("mylogintouse");
   config->lockfile = eina_stringshare_add("/var/run/entrance.pid");
   config->logfile = eina_stringshare_add("/var/log/entrance.log");
   config->theme = eina_stringshare_add("default");
}


static void
_users_get(void)
{
   Eet_File *ef;
   FILE *f;
   int textlen;
   char *text = NULL;

   if (!ecore_file_is_dir(PACKAGE_CACHE))
     ecore_file_mkdir(PACKAGE_CACHE);
   ef = eet_open(PACKAGE_CACHE"/"ENTRANCE_CONFIG_FILE,
                 EET_FILE_MODE_READ_WRITE);
   if (!ef)
     ef = eet_open(PACKAGE_CACHE"/"ENTRANCE_CONFIG_FILE,
                   EET_FILE_MODE_WRITE);
   f = fopen(SYSTEM_CONFIG_DIR"/entrance/entrance.conf", "rb");
   if (!f)
     {
        PT(PACKAGE": Could not open "SYSTEM_CONFIG_DIR"/entrance/entrance.conf");
        return;
     }

   fseek(f, 0, SEEK_END);
   textlen = ftell(f);
   rewind(f);
   if(textlen > 0)
     {
       text = (char *)malloc(textlen);
       if (!text)
         {
           fclose(f);
           eet_close(ef);
           return;
         }
     }
   else
     {
       fclose(f);
       eet_close(ef);
       return;
     }

   if (fread(text, textlen, 1, f) != 1)
     {
        free(text);
        fclose(f);
        eet_close(ef);
        return;
     }

   fclose(f);
   if (eet_data_undump(ef, ENTRANCE_CONFIG_KEY, text, textlen, 1))
     PT(PACKAGE": Updating configuration");
   free(text);
   eet_close(ef);
}

static Entrance_Config *
_cache_get(void)
{
   Entrance_Config *config = NULL;
   Eet_File *file;

   file = eet_open(PACKAGE_CACHE"/"ENTRANCE_CONFIG_FILE,
                   EET_FILE_MODE_READ);
   if (!file)
     {
        PT("Error!!! On read "PACKAGE_CACHE"/"ENTRANCE_CONFIG_FILE);
        return NULL;
     }

   config = eet_data_read(file, _entrance_config_descriptor,
                          ENTRANCE_CONFIG_KEY);
   if (!config)
     {
        PT(": Warning no configuration found! This must "
           "not append, we will go back to default configuration");
        config = (Entrance_Config *) calloc(1, sizeof(Entrance_Config));
        _defaults_set(config);
     }

   eet_close(file);

   return config;
}

static void
_config_free(Entrance_Config *config)
{
   PT("Config free");
   eina_stringshare_del(config->session);
   eina_stringshare_del(config->session_path);
   eina_stringshare_del(config->command.xinit_path);
   eina_stringshare_del(config->command.xinit_args);
   eina_stringshare_del(config->command.xauth_path);
   eina_stringshare_del(config->command.xauth_file);
   eina_stringshare_del(config->command.xdisplay);
   eina_stringshare_del(config->command.session_start);
   eina_stringshare_del(config->command.session_login);
   eina_stringshare_del(config->command.session_stop);
   eina_stringshare_del(config->command.shutdown);
   eina_stringshare_del(config->command.reboot);
   eina_stringshare_del(config->command.suspend);
   eina_stringshare_del(config->userlogin);
   eina_stringshare_del(config->lockfile);
   eina_stringshare_del(config->logfile);
   eina_stringshare_del(config->theme);
   free(config);
}

void
entrance_config_init()
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor_Class eddc;
   struct stat cache;
   struct stat conf;


   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Config);
   edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "port", port, EET_T_USHORT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "session", session, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "session_path", session_path, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "xinit_path", command.xinit_path, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "xinit_args", command.xinit_args, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "xauth_path", command.xauth_path, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "xauth_file", command.xauth_file, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "xdisplay", command.xdisplay, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "vtnr", command.vtnr, EET_T_USHORT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "session_start", command.session_start, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "session_login", command.session_login, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "session_stop", command.session_stop, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "shutdown", command.shutdown, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "reboot", command.reboot, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "suspend", command.suspend, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "daemonize", daemonize, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "numlock", numlock, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "xsessions", xsessions, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "autologin", autologin, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "userlogin", userlogin, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "lockfile", lockfile, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "logfile", logfile, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "theme", theme, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "background_path", bg.path, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "background_group", bg.group, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "elementary_profile", elm_profile, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "virtual_keyboard", vkbd_enabled, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "custom_conf", custom_conf, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "start_user", start_user, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "require_password", req_passwd, EET_T_UCHAR);
   _entrance_config_descriptor = edd;

   if (stat( PACKAGE_CACHE"/"ENTRANCE_CONFIG_FILE, &cache) == -1)
     _users_get();
   else
     {
       int stat_cache = stat(SYSTEM_CONFIG_DIR"/entrance/entrance.conf", &conf);
       if (stat_cache == 0 && cache.st_mtime < conf.st_mtime)
         _users_get();
     }
   entrance_config = _cache_get();
}


void
entrance_config_shutdown()
{
   _config_free(entrance_config);
   eet_data_descriptor_free(_entrance_config_descriptor);
}

void
entrance_config_set(const Entrance_Conf_Gui_Event *conf)
{
   Eina_Bool update = EINA_FALSE;
   if (conf->bg.path && conf->bg.path != entrance_config->bg.path)
     {
        if (entrance_config->bg.path)
          eina_stringshare_replace(&entrance_config->bg.path, conf->bg.path);
        else
          entrance_config->bg.path = eina_stringshare_add(conf->bg.path);
        update = EINA_TRUE;
     }
   if (conf->bg.group && conf->bg.group != entrance_config->bg.group)
     {
        if (entrance_config->bg.group)
          eina_stringshare_replace(&entrance_config->bg.group, conf->bg.group);
        else
          entrance_config->bg.group = eina_stringshare_add(conf->bg.group);
        if (!update)
          update = EINA_TRUE;
     }
   if (conf->req_passwd != entrance_config->req_passwd)
     {
        entrance_config->req_passwd = conf->req_passwd;
        if (!update)
          update = EINA_TRUE;
     }
   if (conf->vkbd_enabled != entrance_config->vkbd_enabled)
     {
        entrance_config->vkbd_enabled = conf->vkbd_enabled;
        if (!update)
          update = EINA_TRUE;
     }
   if (conf->theme != entrance_config->theme)
     {
        entrance_config->theme = conf->theme;
        if (!update)
          update = EINA_TRUE;
     }
   if (update)
     {
        Eet_File *file;

        PT("Config save");
        if (!ecore_file_is_dir(PACKAGE_CACHE))
          ecore_file_mkdir(PACKAGE_CACHE);
        file = eet_open(PACKAGE_CACHE"/"ENTRANCE_CONFIG_FILE,
                        EET_FILE_MODE_READ_WRITE);
        if (!file)
          file = eet_open(PACKAGE_CACHE"/"ENTRANCE_CONFIG_FILE,
                        EET_FILE_MODE_WRITE);
        if (!file)
          {
             PT("Warning can't open "PACKAGE_CACHE"/"ENTRANCE_CONFIG_FILE);
             return;
          }
        eet_data_write(file, _entrance_config_descriptor, ENTRANCE_CONFIG_KEY,
                       entrance_config, 1);
        eet_close(file);
     }
}

