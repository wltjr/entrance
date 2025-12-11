#include "entrance.h"

#define ENTRANCE_SESSION_KEY "session"
#define ENTRANCE_HISTORY_FILE "entrance.hst"

static void _entrance_history_read(void);
static void _entrance_history_write(void);
static void _entrance_user_init(void);
static void _entrance_user_shutdown(void);
Entrance_Login *_entrance_history_match(const char *login);
static void _entrance_history_user_set(Entrance_Login *el, const Entrance_Login *eu);


static Eet_Data_Descriptor *_eddh;
static Entrance_History *_entrance_history;
static Eina_List *_lusers = NULL;
static Eina_Bool _history_update = EINA_FALSE;

void
entrance_history_init(void)
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor_Class eddc;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Login);
   edd = entrance_event_user_dd();

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_History);
   _eddh = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_LIST(_eddh, Entrance_History, "history", history, edd);

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
        eina_stringshare_del(el->lsess);
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

const char *
entrance_history_user_session_get(const char *login)
{
   Eina_List *l;
   Entrance_Login *el;

   EINA_LIST_FOREACH(_entrance_history->history, l, el)
     {
        if (!strcmp(login, el->login))
          return el->lsess;
     }
   return NULL;
}

void
entrance_history_push(const char *login, const char *session)
{
   Eina_List *l;
   Entrance_Login *el;

   PT("history push for user %s session %s", login, session);
   EINA_LIST_FOREACH(_entrance_history->history, l, el)
     {
        if (!strcmp(login, el->login))
          {
             PT("History updating");
             if (el->remember_session)
               {
                  if (!session)
                    {
                       eina_stringshare_del(el->lsess);
                       el->lsess = NULL;
                    }
                  else if (el->lsess && strcmp(session, el->lsess))
                    {
                       eina_stringshare_replace(&el->lsess, session);
                       _history_update = EINA_TRUE;
                    }
               }
             break;
          }
     }
   if (!el)
     {
        PT("History create a new entry for %s", login);
        if ((el = calloc(1, sizeof(Entrance_Login))))
          {
             el->login = eina_stringshare_add(login);
             if (session) el->lsess = eina_stringshare_add(session);
             el->remember_session = EINA_TRUE;
             if (el->login)
               {
                 _entrance_history->history =
                    eina_list_append(_entrance_history->history, el);
                 _history_update = EINA_TRUE;
               }
             else
               {
                 /* Failed to add login, cleanup */
                 free(el);
               }
          }
     }
}

static void
_entrance_history_user_set(Entrance_Login *el, const Entrance_Login *eu)
{
   if (eu->lsess != el->lsess)
     eina_stringshare_replace(&el->lsess, eu->lsess);
   if (eu->image.path != el->image.path)
     eina_stringshare_replace(&el->image.path, eu->image.path);
   if (eu->image.group != el->image.group)
     eina_stringshare_replace(&el->image.group, eu->image.group);
   if (eu->bg.path != el->bg.path)
     eina_stringshare_replace(&el->bg.path, eu->bg.path);
   if (eu->bg.group != el->bg.group)
     eina_stringshare_replace(&el->bg.group, eu->bg.group);
   if (eu->remember_session != el->remember_session)
     el->remember_session = eu->remember_session;
}

void
entrance_history_user_update(const Entrance_Login *eu)
{
   Eina_List *l;
   Entrance_Login *el;

   PT("Updating user info");

   EINA_LIST_FOREACH(_entrance_history->history, l, el)
     {
        if (!strcmp(eu->login, el->login))
          {
             PT("Find user in history");
             _entrance_history_user_set(el, eu);
            break;
          }
     }
   if (!l)
     {
        EINA_LIST_FOREACH(_lusers, l, el)
          {
             if (!strcmp(eu->login, el->login))
               {
                  PT("Append user in history");
                  _entrance_history_user_set(el, eu);
                  _entrance_history->history = eina_list_append(_entrance_history->history, el);
                  break;
               }
          }
     }
   _history_update = !!l;
   _entrance_history_write();
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
   char *token;
   char *user;

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
                if (!eu->login)
                  {
                    /* Failed to add login, cleanup */
                    free(eu);
                    eu = NULL;
                  }
                else
                  eu->remember_session = EINA_TRUE;
              }
          }
        eina_stringshare_del(user);
        if(eu)
          {
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
   char *buf;
   EINA_LIST_FREE(_lusers, eu)
     {
        if (!_entrance_history_match(eu->login))
          {
             eina_stringshare_del(eu->login);
             eina_stringshare_del(eu->lsess);
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

