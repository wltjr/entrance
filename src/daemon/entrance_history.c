#include <errno.h>

#include "entrance.h"

#define ENTRANCE_SESSION_KEY "session"
#define ENTRANCE_HISTORY_FILE "entrance.hst"
#define ENTRANCE_USERS_CONFIG_CONF_FILE "users.conf"

static void _entrance_history_read(void);
static void _entrance_history_write(void);
static void _entrance_users_conf_get(void);
static void _entrance_user_init(void);
static void _entrance_user_shutdown(void);
Entrance_Login *_entrance_history_match(const char *login);


static Eet_Data_Descriptor *_eddh;
static Entrance_History *_entrance_history;
static Eina_List *_lusers = NULL;
static Eina_Bool _history_update = EINA_FALSE;

static void
_entrance_users_conf_get(void)
{
    Eet_File *ef;
    FILE *f;
    int textlen;
    char *text = NULL;

    ef = eet_open(PACKAGE_CACHE "/" ENTRANCE_HISTORY_FILE,
                  EET_FILE_MODE_READ_WRITE);
    if (!ef)
        ef = eet_open(PACKAGE_CACHE "/" ENTRANCE_HISTORY_FILE,
                      EET_FILE_MODE_WRITE);
    f = fopen(SYSTEM_CONFIG_DIR "/entrance/" ENTRANCE_USERS_CONFIG_CONF_FILE, "rb");
    if (!f)
    {
        PT("Failed to open " SYSTEM_CONFIG_DIR "/entrance/" ENTRANCE_USERS_CONFIG_CONF_FILE);
        return;
    }

    fseek(f, 0, SEEK_END);
    textlen = (int)ftell(f);
    rewind(f);
    if (textlen > 0)
    {
        text = (char *)malloc(textlen);
        if (!text)
            goto cleanup;
    }
    else
        goto cleanup;

    if (fread(text, textlen, 1, f) != 1)
        goto cleanup;

    if (eet_data_undump(ef, ENTRANCE_SESSION_KEY, text, textlen, 1))
        PT("Generating users history cache file");

cleanup:
    fclose(f);
    if (text)
        free(text);
    eet_close(ef);
}

void
entrance_history_init(void)
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor_Class eddc;
   struct stat cache;
   struct stat conf;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Login);
   edd = entrance_event_user_dd();

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_History);
   _eddh = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_LIST(_eddh, Entrance_History, "history", history, edd);

    if (stat( PACKAGE_CACHE"/"ENTRANCE_HISTORY_FILE, &cache) == -1)
        _entrance_users_conf_get();
    else
    {
        int stat_conf = stat(SYSTEM_CONFIG_DIR"/entrance/"ENTRANCE_USERS_CONFIG_CONF_FILE, &conf);
        if (stat_conf == 0 && cache.st_mtime < conf.st_mtime)
            _entrance_users_conf_get();
    }

   _entrance_history_read();
   _entrance_user_init();
}

Eina_List
*entrance_history_get(void)
{
   return _lusers;
}

void
entrance_history_shutdown(void)
{
   Entrance_Login *el;

   _entrance_history_write();
   _entrance_user_shutdown();
   EINA_LIST_FREE(_entrance_history->history, el)
     {
        eina_stringshare_del(el->login);
        eina_stringshare_del(el->image.path);
        eina_stringshare_del(el->image.group);
        eina_stringshare_del(el->bg.path);
        eina_stringshare_del(el->bg.group);
        eina_stringshare_del(el->session);
        free(el);
     }
   free(_entrance_history);
}

static void
_entrance_history_read(void)
{
   Eet_File *ef;

   ef = eet_open(PACKAGE_CACHE"/"ENTRANCE_HISTORY_FILE,
                 EET_FILE_MODE_READ_WRITE);
   if (ef)
     {
       _entrance_history = eet_data_read(ef, _eddh, ENTRANCE_SESSION_KEY);
       if(!_entrance_history)
         {
           PT("Error on reading last session login");
           _entrance_history = calloc(1, sizeof(Entrance_History));
         }
       eet_close(ef);
    }
}

static void
_entrance_history_write(void)
{
   if (_history_update)
     {
        Eet_File *ef;

        PT("writing history file");
        ef = eet_open(PACKAGE_CACHE"/"ENTRANCE_HISTORY_FILE,
                      EET_FILE_MODE_READ_WRITE);
        if (!ef)
          ef = eet_open(PACKAGE_CACHE"/"ENTRANCE_HISTORY_FILE,
                        EET_FILE_MODE_WRITE);

        if (!eet_data_write(ef, _eddh, ENTRANCE_SESSION_KEY,
                            _entrance_history, 1))
          PT("Error on updating last session login");

        eet_close(ef);
     }
}

void
entrance_history_check(const char *login, const char *session)
{
   Eina_List *l;
   Entrance_Login *el;

   EINA_LIST_FOREACH(_entrance_history->history, l, el)
     {
        if (strcmp(login, el->login))
            continue;

        PT("History update entry for user %s session %s", login, session);
        if (el->remember_session)
        {
            if (!session)
            {
                eina_stringshare_del(el->session);
                el->session = NULL;
            }
            else if (el->session && strcmp(session, el->session))
            {
                eina_stringshare_replace(&el->session, session);
                _history_update = EINA_TRUE;
            }
        }
        break;
     }
   if (!el)
     {
        PT("History add entry for user %s session %s", login, session);
        el = calloc(1, sizeof(Entrance_Login));
        el->login = eina_stringshare_add(login);
        if (session)
            el->session = eina_stringshare_add(session);
        el->remember_session = EINA_TRUE;
        _entrance_history->history = eina_list_append(_entrance_history->history, el);
        _history_update = EINA_TRUE;
     }
}

Entrance_Login *
_entrance_history_match(const char *login)
{
   Eina_List *l;
   Entrance_Login *el = NULL;

   EINA_LIST_FOREACH(_entrance_history->history, l, el)
     {
        if (el->login == login)
          break;
     }
   return el;
}

static void
_entrance_user_init(void)
{
   FILE *f;
   Eina_List *lu = NULL;
   char *saveptr;
   const char *token;
   const char *user;

   PT("scan for users");
   f = fopen("/etc/passwd", "r");
   if (f)
     {
        char buf[PATH_MAX];
        int uid;

        while (fgets(buf, sizeof(buf), f))
          {
             user = strtok_r(buf, ":", &saveptr);
             if (!user) continue;
             strtok_r(NULL, ":", &saveptr);
             token = strtok_r(NULL, ":", &saveptr);
             if (!token) continue;
             uid = atoi(token);
             if (uid > 999 && uid < 65534)
                 lu = eina_list_append(lu, eina_stringshare_add(user));
             }
        fclose(f);
     }
   EINA_LIST_FREE(lu, user)
     {
        Entrance_Login *eu;

        eu = _entrance_history_match(user);
        if (!eu)
          {
            eu = (Entrance_Login *) calloc(1, sizeof(Entrance_Login));
            if (eu)
              {
                eu->login = eina_stringshare_add(user);
                eu->remember_session = EINA_TRUE;
              }
          }
        eina_stringshare_del(user);
        if(eu)
          {
            struct stat path;

            if(eu->image.path && stat(eu->image.path, &path) == -1)
              {
                PT("User image path %s error %s", eu->image.path, strerror(errno));
                eina_stringshare_del(eu->image.path);
                eu->image.path = NULL;
              }

            if(eu->bg.path && stat(eu->bg.path, &path) == -1)
              {
                PT("User background path %s error %s", eu->bg.path, strerror(errno));
                eina_stringshare_del(eu->bg.path);
                eu->bg.path = NULL;
              }
            eu->icon_pool = entrance_image_user_icons(eu->login);
            eu->background_pool = entrance_image_user_backgrounds(eu->login);
            _lusers = eina_list_append(_lusers, eu);
          }
     }
}

static void
_entrance_user_shutdown(void)
{
   Entrance_Login *eu;
   const char *buf;
   EINA_LIST_FREE(_lusers, eu)
     {
        if (!_entrance_history_match(eu->login))
          {
             eina_stringshare_del(eu->login);
             eina_stringshare_del(eu->session);
             eina_stringshare_del(eu->image.path);
             eina_stringshare_del(eu->image.group);
             eina_stringshare_del(eu->bg.path);
             eina_stringshare_del(eu->bg.group);
             EINA_LIST_FREE(eu->background_pool, buf)
               {
                 eina_stringshare_del(buf);
               }
             EINA_LIST_FREE(eu->icon_pool, buf)
               {
                 eina_stringshare_del(buf);
               }
             free(eu);
          }
     }
}

