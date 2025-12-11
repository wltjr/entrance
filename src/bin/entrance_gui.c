#include "entrance_client.h"
#include "entrance_edje.h"
#include "Ecore_X.h"
#include "time.h"
#include <sys/utsname.h>


typedef struct Entrance_Gui_ Entrance_Gui;
typedef struct Entrance_Screen_ Entrance_Screen;

static void _entrance_gui_user_sel_cb(void *data, Evas_Object *obj, void *event_info);
static char *_entrance_gui_user_text_get(void *data, Evas_Object *obj, const char *part);
static Evas_Object *_entrance_gui_user_content_get(void *data, Evas_Object *obj, const char *part);
static Eina_Bool _entrance_gui_user_state_get(void *data, Evas_Object *obj, const char *part);
static void _entrance_gui_actions_populate();
static void _entrance_gui_conf_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _entrance_gui_update(void);
static Eina_List* _entrance_gui_theme_icons_cache_fill(Evas_Object *obj, const char *themename);
static void _entrance_gui_users_populate(void);

static Entrance_Gui *_gui;

enum {
     ENTRANCE_CONF_NONE = 0,
     ENTRANCE_CONF_STATE = (1 << 0),
     ENTRANCE_CONF_WALLPAPER = (1 << 1),
     ENTRANCE_CONF_REQ_PASSWD = (1 << 2),
     ENTRANCE_CONF_VKBD = (1 << 3)
};


struct Entrance_Gui_
{
   Evas_Object *win;
   Eina_List *screens;
   Eina_List *xsessions;
   Eina_List *users;
   Eina_List *actions;
   Eina_List *background_pool;
   Eina_List *icon_pool;
   Eina_List *theme_icon_pool;
   Eina_List *themes;
   Entrance_Xsession *selected_session;
   const char *theme;
   struct
     {
        const char *path;
        const char *group;
     } bg;
   unsigned char changed;
   Eina_Bool conf_enabled : 1;
   Eina_Bool req_passwd : 1;
   Eina_Bool vkbd_enabled : 1;

};

struct Entrance_Screen_
{
   Evas_Object *edj;
   Evas_Object *background;
   Evas_Object *transition;
   Evas_Object *login;
   Eina_Bool focused:1;
   Eina_Bool managed:1;
};

int
entrance_gui_init(const char *theme)
{

   Ecore_X_Window xw;
   int i;
   int j;
   int x;
   int y;
   int w;
   int h;
   int hh = 0;
   int ww = 0;

   PT("Gui init: %s", theme);
   _gui = calloc(1, sizeof(Entrance_Gui));
   if (!_gui)
     {
        PT("Not Enough memory");
        return 1;
     }
   _gui->theme = eina_stringshare_add(theme);

   i = ecore_x_xinerama_screen_count_get();
   if (i < 1) i = 1;
   _gui->win = elm_win_add(NULL, "main", ELM_WIN_BASIC);
   elm_win_fullscreen_set(_gui->win, EINA_TRUE);
   elm_win_title_set(_gui->win, PACKAGE);
   for(j = 0; j < i; ++j)
     {
        Entrance_Screen *screen;
        Evas_Object *o;

        screen = calloc(1, sizeof(Entrance_Screen));
        if (!screen) return 1;

        /* layout */
         o = entrance_gui_theme_get(_gui->win, ENTRANCE_EDJE_GROUP_WALLPAPER);
         screen->transition = o;
         if(j<1)
           {
             Evas_Object *ol;

             ol = entrance_gui_theme_get(_gui->win, ENTRANCE_EDJE_GROUP_ENTRANCE);
             if (!ol)
               {
                  PT("Tut Tut Tut no theme for entrance");
                  evas_object_del(screen->transition);
                  free(screen);
                  return j;
               }
             screen->edj = ol;
             elm_object_part_content_set(o, ENTRANCE_EDJE_PART_SCREEN, ol);

             /* date */
             time_t t = time(0);
             struct tm local_tm;
             struct tm tm = *localtime_r(&t, &local_tm);
             char date[64];
             strftime(date,64,"%B %d, %Y",&tm);
             elm_object_part_text_set(ol, ENTRANCE_EDJE_PART_DATE,date);

             /* clock */
             o = elm_clock_add(ol);
             elm_clock_show_am_pm_set(o, EINA_TRUE);
             elm_object_part_content_set(ol, ENTRANCE_EDJE_PART_CLOCK, o);

             /* uname */
             struct utsname uname_str;
             if(uname(&uname_str)==0)
               {
                 char uname_value[1024];
                 snprintf(uname_value,
                         1024,
                         "%s %s %s %s",
                         uname_str.sysname,
                         uname_str.nodename,
                         uname_str.release,
                         uname_str.machine);
                 elm_object_part_text_set (ol, ENTRANCE_EDJE_PART_UNAME, uname_value);
               }

             o = entrance_login_add(ol, screen);
             if (!o)
               {
                 /* Failed to create login, cleanup screen */
                 evas_object_del(screen->transition);
                 evas_object_del(screen->edj);
                 free(screen);
                 continue;
               }
             entrance_login_open_session_set(o, EINA_TRUE);
             screen->login = o;
             elm_object_part_content_set(ol, ENTRANCE_EDJE_PART_LOGIN, o);
             evas_object_smart_callback_add(
                ENTRANCE_GUI_GET(ol, ENTRANCE_EDJE_PART_CONF),
                "clicked",
                _entrance_gui_conf_clicked_cb,
                screen->transition);
             evas_object_show(screen->login);
             evas_object_show(screen->edj);
           }
         evas_object_show(screen->transition);

         _gui->screens = eina_list_append(_gui->screens, screen);
         ecore_x_xinerama_screen_geometry_get(j, &x, &y, &w, &h);
         evas_object_move(screen->transition, x, y);
         evas_object_resize(screen->transition, w, h);
         if ((x + w) > ww) ww = x + w;
         if ((y + h) > hh) hh = y + h;
     }
   _gui->theme_icon_pool =
      _entrance_gui_theme_icons_cache_fill(_gui->win, _gui->theme);
   _entrance_gui_update();
   xw = elm_win_xwindow_get(_gui->win);
   ecore_x_window_move(xw, 0, 0);
   ecore_x_event_mask_set(ecore_x_window_root_get(xw),
                          ECORE_X_EVENT_MASK_WINDOW_PROPERTY);
   evas_object_resize(_gui->win, ww, hh);
   evas_object_show(_gui->win);
   /* tricky situation. we are not normally running with a wm and thus
    * have to set focus to our window so things work right */
   Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(_gui->win));
   ecore_evas_override_set(ee,EINA_TRUE);
   ecore_evas_focus_set(ee, 1);
   /* need to hide and show the cursor */
   ecore_x_window_cursor_show(elm_win_xwindow_get(_gui->win),
                              EINA_FALSE);
   ecore_x_window_cursor_show(elm_win_xwindow_get(_gui->win),
                              EINA_TRUE);
   return j;
}

static void
_entrance_gui_theme_update(void)
{
   Eina_List *node;
   Entrance_Screen *screen;
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf),
            PACKAGE_DATA_DIR"/themes/%s.edj", _gui->theme);
   EINA_LIST_FOREACH(_gui->screens, node, screen)
     {
        elm_layout_file_set(screen->transition, buf, ENTRANCE_EDJE_GROUP_WALLPAPER);
        elm_layout_file_set(screen->edj, buf, ENTRANCE_EDJE_GROUP_ENTRANCE);
        elm_layout_file_set(screen->login, buf, ENTRANCE_EDJE_GROUP_LOGIN);
        evas_object_smart_callback_add(
                ENTRANCE_GUI_GET(screen->edj, ENTRANCE_EDJE_PART_CONF),
                "clicked",
                _entrance_gui_conf_clicked_cb,
                screen->transition);
     }
   _gui->theme_icon_pool =
          _entrance_gui_theme_icons_cache_fill(_gui->win, _gui->theme);
   _entrance_gui_actions_populate();
   _entrance_gui_users_populate();

}

void
entrance_gui_shutdown(void)
{
   Entrance_Screen *screen;
   Entrance_Xsession *xsession;
   Entrance_Image *img;
   PT("Gui shutdown");
   if (_gui)
     {
       evas_object_del(_gui->win);
       EINA_LIST_FREE(_gui->screens, screen)
         {
            free(screen);
         }
       eina_stringshare_del(_gui->theme);
       EINA_LIST_FREE(_gui->xsessions, xsession)
         {
            eina_stringshare_del(xsession->name);
            eina_stringshare_del(xsession->command);
            eina_stringshare_del(xsession->icon);
         }
       EINA_LIST_FREE(_gui->background_pool, img)
         {
           eina_stringshare_del(img->path);
           eina_stringshare_del(img->group);
           free(img);
         }
       EINA_LIST_FREE(_gui->icon_pool, img)
         {
           eina_stringshare_del(img->path);
           eina_stringshare_del(img->group);
           free(img);
         }
       EINA_LIST_FREE(_gui->theme_icon_pool, img)
         {
           eina_stringshare_del(img->path);
           eina_stringshare_del(img->group);
           free(img);
         }
       free(_gui);
     }
}

static Eina_List*
_entrance_gui_string_to_entrance_image(Eina_List *src, char *stdfile, char *mask)
{
   //If srdfile is NULL we will set the src string to file, if not we will set the stdfile. And the src as group.
   Eina_List *result = NULL;
   char *src_str;

   EINA_LIST_FREE(src, src_str)
     {
        Entrance_Image *img;

        img = calloc(1, sizeof(Entrance_Image));
        if (stdfile)
          {
            if (mask)
              {
                 char path[PATH_MAX];

                 snprintf(path, PATH_MAX, mask, src_str);
                 img->group = eina_stringshare_add(path);
                 eina_stringshare_del(src_str);
              }
            else
              img->group = src_str;
            img->path = eina_stringshare_add(stdfile);
          }
        else
          {
            if (!src_str || !src_str[0])
              {
                 /* Invalid image, cleanup */
                 free(img);
                 continue;
              }
            img->path = src_str;
          }
        result = eina_list_append(result,img);
     }
   return result;
}

Eina_List*
entrance_gui_theme_icons(void)
{
  return _gui->theme_icon_pool;
}

static Eina_List*
_entrance_gui_theme_icons_cache_fill(Evas_Object *obj, const char *themename)
{
   Evas_Object *edje;
   Evas_Object *o;
   char buf[PATH_MAX];
   Eina_List *icons = NULL;

   edje = elm_layout_add(obj);
   snprintf(buf, sizeof(buf),
            PACKAGE_DATA_DIR"/themes/%s.edj", themename);
   if (!elm_layout_file_set(edje, buf, ENTRANCE_EDJE_GROUP_USER))
     return NULL; //Can we get to this point ??
   o = elm_layout_edje_get(edje);
   if (!o) return NULL;
   icons = entrance_gui_stringlist_get(edje_object_data_get(o, "items"));
   evas_object_del(edje);
   return _entrance_gui_string_to_entrance_image(icons, buf, "entrance/user/%s");
}

Evas_Object *
entrance_gui_theme_get (Evas_Object *win, const char *group)
{
   Evas_Object *edje;

   edje = elm_layout_add(win);
   if (_gui->theme)
     {
        char buf[PATH_MAX];
        snprintf(buf, sizeof(buf),
                 PACKAGE_DATA_DIR"/themes/%s.edj", _gui->theme);
        if (!elm_layout_file_set(edje, buf, group))
          {
             PT("Can't load %s theme fallback to default", _gui->theme);
             elm_layout_file_set(edje, PACKAGE_DATA_DIR"/themes/default.edj",
                                 group);
          }
     }
   else
     elm_layout_file_set(edje, PACKAGE_DATA_DIR"/themes/default.edj", group);
   return edje;
}

Eina_List *
entrance_gui_stringlist_get(const char *str)
{
   Eina_List *list = NULL;
   const char *b;
   const char *s;
   if (!str) return NULL;
   for (b = s = str; 1; s++)
     {
        if ((*s == ' ') || (!*s))
          {
             char *t = malloc(s - b + 1);
             if (t)
               {
                  strncpy(t, b, s - b);
                  t[s - b] = 0;
                  list = eina_list_append(list, eina_stringshare_add(t));
                  free(t);
               }
             b = s + 1;
          }
        if (!*s) break;
     }
   return list;
}

void
entrance_gui_stringlist_free(Eina_List *list)
{
   const char *s;
   EINA_LIST_FREE(list, s)
      eina_stringshare_del(s);
}


void
entrance_gui_actions_set(Eina_List *actions)
{
   if (!actions) return;
   PT("Actions set");
   _gui->actions = actions;
   _entrance_gui_actions_populate();
}

void
entrance_gui_auth_max_tries(void)
{
   /*
   Evas_Object *o;
   Eina_List *l;
   Entrance_Screen *screen;

   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        o = ENTRANCE_GUI_GET(screen->edj, "entrance.login");
        elm_entry_entry_set(o, "");
        elm_object_disabled_set(o, EINA_TRUE);
        o = ENTRANCE_GUI_GET(screen->edj, "entrance.password");
        elm_entry_entry_set(o, "");
        elm_object_disabled_set(o, EINA_TRUE);
     }
   ecore_timer_add(5.0, _entrance_gui_auth_enable, NULL);
   */
}

static void
_entrance_gui_users_populate(void)
{
   Entrance_Screen *screen;
   Eina_List *l;
   Evas_Object *ol;
   Entrance_Fill *ef;
   const char *style;

   screen = eina_list_data_get(_gui->screens);

   style = edje_object_data_get(elm_layout_edje_get(screen->edj), "item_style_users");

   if (!style)
     style = "default"; //theme has not settet a style

   PT("Add users list, using item style: %s", style);
   ef = entrance_fill_new(style,
                          _entrance_gui_user_text_get,
                          _entrance_gui_user_content_get,
                          _entrance_gui_user_state_get);

   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        ol = ENTRANCE_GUI_GET(screen->edj, ENTRANCE_EDJE_PART_USERS);
        if (!ol) continue;
        entrance_fill(ol, ef, _gui->users, NULL,
                      _entrance_gui_user_sel_cb, screen->login);
        elm_object_signal_emit(screen->edj,
                               ENTRANCE_EDJE_SIGNAL_USERS_ENABLED, "");
     }
   entrance_fill_del(ef);
}

void
entrance_gui_users_set(Eina_List *users)
{
   _gui->users = users;
   _entrance_gui_users_populate();
}

const Eina_List *
entrance_gui_users_get(void)
{
   return _gui->users;
}

const Entrance_Login*
entrance_gui_user_get(const char* name)
{
   Entrance_Login *el;
   Eina_List *l;
   EINA_LIST_FOREACH(_gui->users, l, el)
     {
       if(!strcmp(name, el->login))
         {
            return el;
         }
     }
   return NULL;
}

void
entrance_gui_xsessions_set(Eina_List *xsessions)
{
   Entrance_Screen *screen;
   Eina_List *l;

   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        entrance_login_xsessions_populate(screen->login, xsessions);
     }
   _gui->xsessions = xsessions;
}

const Eina_List *
entrance_gui_xsessions_get(void)
{
   return _gui->xsessions;
}

void
entrance_gui_conf_set(const Entrance_Conf_Gui_Event *conf)
{
   if (_gui->bg.path != conf->bg.path)
     {
        if ((conf->bg.path) && (*conf->bg.path))
          eina_stringshare_replace(&_gui->bg.path, conf->bg.path);
        else
          {
             eina_stringshare_del(_gui->bg.path);
             _gui->bg.path = NULL;
          }
        _gui->changed &= ENTRANCE_CONF_WALLPAPER;
     }
   if (_gui->bg.group != conf->bg.group)
     {
        if ((conf->bg.group) && (*conf->bg.group))
          eina_stringshare_replace(&_gui->bg.group, conf->bg.group);
        else
          {
             eina_stringshare_del(_gui->bg.group);
             _gui->bg.group = NULL;
          }
        _gui->changed &= ENTRANCE_CONF_WALLPAPER;
     }

   if (_gui->req_passwd != conf->req_passwd)
     {
        _gui->req_passwd = conf->req_passwd;
        _gui->changed &= ENTRANCE_CONF_REQ_PASSWD;
     }
   if (_gui->vkbd_enabled != conf->vkbd_enabled)
     {
        _gui->vkbd_enabled = conf->vkbd_enabled;
        _gui->changed &= ENTRANCE_CONF_VKBD;
     }
   _gui->changed = ~(ENTRANCE_CONF_NONE);
   _entrance_gui_update();
}

void
entrance_gui_pools_set(const Entrance_Pools *pool)
{
   Entrance_Image *img;
   EINA_LIST_FREE(_gui->background_pool, img)
     {
        eina_stringshare_del(img->path);
        eina_stringshare_del(img->group);
        free(img);
     }
   _gui->background_pool = pool->background_pool;

   EINA_LIST_FREE(_gui->icon_pool, img)
     {
        eina_stringshare_del(img->path);
        eina_stringshare_del(img->group);
        free(img);
     }
   _gui->icon_pool = pool->icon_pool;
}

void
entrance_gui_themes_set(Eina_List *list)
{
   _gui->themes = list;
}

Eina_List*
entrance_gui_themes_get(void)
{
   return _gui->themes;
}

void
entrance_gui_theme_name_set(const char *theme)
{
   if (!_gui->theme)
     _gui->theme = eina_stringshare_add(theme);
   else
     eina_stringshare_replace(&_gui->theme, theme);
   _entrance_gui_theme_update();
}

const char *
entrance_gui_theme_name_get(void)
{
   return _gui->theme;
}

Eina_List*
entrance_gui_icon_pool_get(void)
{
  return _gui->icon_pool;
}

const char *
entrance_gui_theme_path_get(void)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf),
            PACKAGE_DATA_DIR"/themes/%s.edj", _gui->theme);
   return eina_stringshare_add(buf);
}

void
entrance_gui_background_get(const char **path, const char **group)
{
   if (path)
     *path = _gui->bg.path;
   if (group)
     *group = _gui->bg.group;
}

Eina_Bool
entrance_gui_req_passwd_get(void)
{
   return _gui->req_passwd;
}

Eina_Bool
entrance_gui_vkbd_enabled_get(void)
{
   return _gui->vkbd_enabled;
}

static Evas_Object *
_entrance_gui_background_obj_get(Evas_Object *par,
                                 const char *path,
                                 const char *group)
{
  Evas_Object *bg = NULL;
  if (path)
    {
      bg = elm_bg_add(par);
      elm_bg_file_set(bg, path, group);
    }
  return bg;
}

static void
_entrance_gui_update(void)
{
   Eina_Bool primary = EINA_TRUE;
   Eina_List *l;
   Entrance_Screen *screen;

   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        Evas_Object *bg = NULL;
        if (_gui->changed & ENTRANCE_CONF_WALLPAPER)
          {
             PT("Set background %s - %s", _gui->bg.path, _gui->bg.group);
             bg = _entrance_gui_background_obj_get(screen->transition,
                                                   _gui->bg.path,
                                                   _gui->bg.group);
             if (!bg)
               {
                  const char *path = PACKAGE_DATA_DIR"/../elementary/themes/default.edj";
                  const char *group = "e/desktop/background";
                  bg = elm_layout_add(screen->transition);
                  elm_layout_file_set(bg,path,group);
                  eina_stringshare_replace(&_gui->bg.path, path);
                  eina_stringshare_replace(&_gui->bg.group, group);
               }
             elm_object_part_content_set(screen->transition,
                                         ENTRANCE_EDJE_PART_WALLPAPER, bg);
             evas_object_del(screen->background);
             screen->background = bg;
          }
        if(primary)
          {
            primary = EINA_FALSE;
            if (_gui->conf_enabled)
              elm_object_signal_emit(screen->edj,
                                     ENTRANCE_EDJE_SIGNAL_CONF_ENABLED, "");
            else
              elm_object_signal_emit(screen->edj,
                                     ENTRANCE_EDJE_SIGNAL_CONF_DISABLED, "");
          }
     }
   _gui->changed = 0;
}

static void
_entrance_gui_conf_clicked_cb(void *data EINA_UNUSED,
                              Evas_Object *obj EINA_UNUSED,
                              void *event EINA_UNUSED)
{
   // FIXME: Do new conf UI
}
///////////////////////////////////////////////////
///////////////// USER ////////////////////////////
///////////////////////////////////////////////////


static Evas_Object*
_entrance_gui_user_icon_random_get(Evas_Object *obj, const char *username)
{
   unsigned int rnd = 0;
   Evas_Object *o = NULL;
   Entrance_Image *img;
   const Entrance_Login *el;
   Eina_List *sys_icons = NULL;
   Eina_List *theme_icons = NULL;
   Eina_List *user_icons = NULL;

   el = entrance_gui_user_get(username);
   if (el)
       user_icons = el->icon_pool;
   sys_icons = entrance_gui_icon_pool_get();
   theme_icons = entrance_gui_theme_icons();

   struct timespec time;
   clock_gettime(CLOCK_REALTIME, &time);
   rnd = (((eina_list_count(user_icons) + eina_list_count(sys_icons) + eina_list_count(theme_icons))
         * time.tv_nsec) / (INT_MAX + 1.0));
   if ((el) && (rnd < eina_list_count(user_icons)))
     {
        o = elm_icon_add(obj);
        img = eina_list_nth(user_icons, rnd);
        elm_image_file_set(o, img->path, NULL);

     }
   else if((rnd >= eina_list_count(user_icons)) && (rnd < (eina_list_count(user_icons)
            +eina_list_count(sys_icons))))
     {
        o = elm_icon_add(obj);
        img = eina_list_nth(sys_icons, (rnd - eina_list_count(user_icons)));
        elm_image_file_set(o, img->path, NULL);
     }
   else
     {
        img = eina_list_nth(theme_icons, (rnd - (eina_list_count(user_icons)
                                        + eina_list_count(sys_icons))));
        o = elm_icon_add(obj);
        elm_image_file_set(o, img->path, img->group);

     }
   return o;
}

static void
_entrance_gui_user_sel_cb(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          void *event_info)
{
   Entrance_Login *eu;
   eu = elm_object_item_data_get(event_info);
   entrance_login_login_set(data, eu->login);
}


static char *
_entrance_gui_user_text_get(void *data,
                            Evas_Object *obj EINA_UNUSED,
                            const char *part)
{
   Entrance_Login *eu;
   eu = data;
   if ((part) && (!strcmp(part, "elm.text.sub")))
     if (eu->lsess)
       return strdup(eu->lsess);
     else
       return NULL;
   else
     return strdup(eu->login);
}

static Evas_Object *
_entrance_gui_user_content_get(void *data EINA_UNUSED,
                               Evas_Object *obj,
                               const char *part)
{
   Evas_Object *ic = NULL;
   Entrance_Login *eu;

   eu = data;
   if (eu && !strcmp(part, "elm.swallow.icon"))
     {
        Evas_Object *o;

        ic = entrance_gui_theme_get(obj, ENTRANCE_EDJE_GROUP_USER);
        if ((!eu->image.path) && (!eu->image.group))
          {
             o = _entrance_gui_user_icon_random_get(obj, eu->login);
             if(eu->image.path || eu->image.group)
               {
                 elm_image_file_get(o,&(eu->image.path),&(eu->image.group));
                 eu->tmp_icon = EINA_TRUE;
               }
          }
        else if(eu->image.path && (!eu->image.group))
          {
             o = elm_icon_add(obj);
             elm_image_file_set(o, eu->image.path, NULL);
          }
        else
          {
             o = entrance_gui_theme_get(obj, eu->image.group);
          }

        evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_show(o);
        elm_object_part_content_set(ic ,"entrance.icon", o);
     }
   return ic;
}

static Eina_Bool
_entrance_gui_user_state_get(void *data EINA_UNUSED,
                             Evas_Object *obj EINA_UNUSED,
                             const char *part EINA_UNUSED)
{
   return EINA_FALSE;
}

///////////////////////////////////////////////////
///////////////// ACTION //////////////////////////
///////////////////////////////////////////////////

static char *
_entrance_gui_action_text_get(void *data,
                              Evas_Object *obj EINA_UNUSED,
                              const char *part EINA_UNUSED)
{
   Entrance_Action *ea;
   ea = data;
   if ((part) && (!strcmp(part, "icon")))
     return strdup(ea->icon);
   return strdup(ea->label);
}

static void
_entrance_gui_action_clicked_cb(void *data EINA_UNUSED,
                                Evas_Object *obj EINA_UNUSED,
                                void *event_info)
{
   Entrance_Action *ea;
   ea = elm_object_item_data_get(event_info);
   if (ea) entrance_connect_action_send(ea->id);
}

static void
_entrance_gui_actions_populate()
{
   Evas_Object *o;
   Eina_List *l;
   Entrance_Screen *screen;
   const char *style;

   screen = eina_list_data_get(_gui->screens);

   style = edje_object_data_get(elm_layout_edje_get(screen->edj), "item_style_actions");

   if (!style)
     style = "default"; //theme has not setted a style

   EINA_LIST_FOREACH(_gui->screens, l, screen)
     {
        Entrance_Fill *ef;
        ef = entrance_fill_new(style, _entrance_gui_action_text_get, NULL, NULL);
        o = ENTRANCE_GUI_GET(screen->edj, ENTRANCE_EDJE_PART_ACTIONS);
        entrance_fill(o, ef, _gui->actions, NULL,
                      _entrance_gui_action_clicked_cb, screen);
        edje_object_signal_emit(elm_layout_edje_get(screen->edj),
                                ENTRANCE_EDJE_SIGNAL_ACTION_ENABLED, "");
     }
}
