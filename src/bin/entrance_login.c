#include "entrance_client.h"
#include "entrance_edje.h"
#include <Eina.h>

static void _login_check_auth(Evas_Object *widget);
static void _login_xsession_update(Evas_Object *obj);
static void _login_xsession_guess(void *data, const char *user);
static void _login_xsession_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _login_login_unfocused_cb(void *data, Evas_Object *obj, void *event);
static void _login_login_activated_cb(void *data, Evas_Object *obj, void *event);
static char *_login_xsession_text_get(void *data, Evas_Object *obj, const char *part);
static void _login_auth_cb(void *data, const char *user, Eina_Bool granted);
static void _entrance_login_session_set(Evas_Object *widget, const char *name);
static void _entrance_login_auth_check_cb(void *data, Evas_Object *obj, const char *signal, const char *source);

typedef struct _Entrance_Gui_Login
{
   Ecore_Timer *write_timer;
   Ecore_Event_Handler *handler;
   Entrance_Xsession *session;
   struct
     {
        Entrance_Login_Cb login;
        void *data;
     } func;
   void *auth;
   Eina_Bool open_session : 1;
   Eina_Bool selected : 1;
   Eina_Bool error : 1;
   Eina_Bool wait : 1;
} Entrance_Gui_Login;

static Entrance_Fill *_login_fill;
static Entrance_Gui_Login *_login;

#define ALERT_ERROR(widget,text) \
  elm_object_part_text_set(widget, ENTRANCE_EDJE_PART_LABEL, text); \
  elm_object_signal_emit(widget, ENTRANCE_EDJE_SIGNAL_AUTH_ERROR, ""); \
  _login->error = EINA_TRUE;

static void
_login_check_auth(Evas_Object *widget)
{
   Evas_Object *o;
   const char *host;
   const char *passwd;

   o = elm_object_part_content_get(widget, ENTRANCE_EDJE_PART_LOGIN);
   host = elm_entry_markup_to_utf8(elm_object_text_get(o));
   if(!host || strlen(host)<1)
     {
       ALERT_ERROR(widget, _("Please enter your user name"));
       return;
     }
   o = elm_object_part_content_get(widget, ENTRANCE_EDJE_PART_PASSWORD);
   passwd = elm_entry_markup_to_utf8(elm_object_text_get(o));
   if(entrance_gui_req_passwd_get() && (!passwd || strlen(passwd)<1))
     {
       ALERT_ERROR(widget, _("Please enter your password"));
       return;
     }
   _login->wait = EINA_TRUE;
   if (!_login->auth)
     _login->auth = entrance_connect_auth_cb_add(_login_auth_cb, widget);
   if (_login->session)
     entrance_connect_auth_send(host, passwd,
                                _login->session->name,
                                _login->open_session);
   else
     entrance_connect_auth_send(host, passwd, NULL, _login->open_session);

   elm_object_signal_emit(widget, ENTRANCE_EDJE_SIGNAL_AUTH_CHECKING, "");
}
/**
 * Login entry changed callback to clear alert/error labels, etc.
 */
static void
_login_entry_changed_cb(void *data,
                       Evas_Object *obj EINA_UNUSED,
                       void *event EINA_UNUSED)
{  
  if(_login->error) 
    {
      elm_object_signal_emit(data, ENTRANCE_EDJE_SIGNAL_AUTH_CHANGED, "");
      _login->error = EINA_FALSE;
    }
}

static void
_login_login_unfocused_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   Evas_Object *o;
   const char *hostname;

   if (_login->write_timer)
     {
        ecore_timer_del(_login->write_timer);
        _login->write_timer = NULL;
     }

   o = elm_object_part_content_get(data, ENTRANCE_EDJE_PART_LOGIN);
   hostname = elm_entry_markup_to_utf8(elm_object_text_get(o));

   _login_xsession_guess(data, hostname);
}

static Eina_Bool
_login_login_timer_cb(void *data)
{
   Evas_Object *o;
   const char *hostname;

   o = elm_object_part_content_get(data, ENTRANCE_EDJE_PART_LOGIN);
   hostname = elm_entry_markup_to_utf8(elm_object_text_get(o));
   _login_xsession_guess(data, hostname);

   _login->write_timer = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_login_login_activated_cb(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          void *event EINA_UNUSED)
{
   elm_object_focus_set(data, EINA_TRUE);
}

static void
_login_login_changed_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   _login_entry_changed_cb(data, NULL, NULL);
   if (_login->write_timer)
     ecore_timer_del(_login->write_timer);
   _login->write_timer = ecore_timer_add(0.5, _login_login_timer_cb, data);
}

static char *
_login_xsession_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part)
{
   Entrance_Xsession *xsession;
   xsession = data;
   if ((part) && (!strcmp(part, "icon")))
     {
        if (xsession->icon)
          return strdup(xsession->icon);
        else
          return NULL;
     }
   return strdup(xsession->name);

}

static void
_login_xsession_update(Evas_Object *obj)
{
   Evas_Object *icon;
   Evas_Object *o;

   o = elm_object_part_content_get(obj, ENTRANCE_EDJE_PART_XSESSIONS);
   if (!_login->session) return;
   elm_object_text_set(o, _login->session->name);
   icon = elm_object_part_content_get(o, "icon");
   if (_login->session->icon &&
       strcmp(_login->session->icon,""))
     {
       Eina_Stringshare *path;
       if (!icon)
          icon = elm_icon_add(o);
        path = entrance_gui_theme_path_get();
        if(_login->session->icon[0]!='/')
          elm_icon_standard_set(icon, _login->session->icon);
        else
          elm_image_file_set(icon, _login->session->icon, path);
        eina_stringshare_del(path);
        elm_object_part_content_set(o, "icon", icon);
     }
   else
     {
        evas_object_del(icon);
        elm_object_part_content_set(o, "icon", NULL);
     }
}

static void
_login_xsession_guess(void *data, const char *user)
{
   const Eina_List *l;
   const Eina_List *users;
   Entrance_Login *eu;

   users = entrance_gui_users_get();
   EINA_LIST_FOREACH(users, l, eu)
     {
        if (!strcmp(eu->login, user))
          {
             _entrance_login_session_set(data, eu->lsess);
             break;
          }
     }

   if (!l)
     {
        _entrance_login_session_set(data, NULL);
     }
}

static void
_login_xsession_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   _login->session = elm_object_item_data_get(event_info);
   _login_xsession_update(data);
}

static void
_login_auth_cb(void *data, const char *user, Eina_Bool granted)
{
   if (_login->wait)
     {
        _login->wait = EINA_FALSE;
        entrance_connect_auth_cb_del(_login->auth);
        _login->auth = NULL;
        if (!granted)
          {
            Evas_Object *o;

            ALERT_ERROR(data, _("Login failed"));
            o = elm_object_part_content_get(data, ENTRANCE_EDJE_PART_PASSWORD);
            elm_object_text_set(o, "");
          }
        else
          elm_object_signal_emit(data, ENTRANCE_EDJE_SIGNAL_AUTH_VALID, "");
     }
}

static void
_entrance_login_session_set(Evas_Object *widget, const char *name)
{
   Entrance_Xsession *sess;
   const Eina_List *l = NULL;

   if (name)
     {
        EINA_LIST_FOREACH(entrance_gui_xsessions_get(), l, sess)
          {
             if ((sess->name) &&
                 (!strcmp(sess->name, name)))
               {
                  break;
               }
          }
     }
   if (l)
     _login->session = sess;
   else
     _login->session = eina_list_data_get(entrance_gui_xsessions_get());
   _login_xsession_update(widget);
}

static void
_entrance_login_auth_check_cb(void *data, Evas_Object *obj EINA_UNUSED, const char *signal EINA_UNUSED, const char *source EINA_UNUSED)
{
   _login_check_auth(data);
}

static void
_login_auth_check_cb(void *data,
                     Evas_Object *obj EINA_UNUSED,
                     void *event EINA_UNUSED)
{
   _login_check_auth(data);
}

////////////////////////////////////////////////////////////////////////////////

void
entrance_login_init(void)
{
   _login_fill = entrance_fill_new(NULL, _login_xsession_text_get, NULL, NULL);
}

void
entrance_login_shutdown(void)
{
   free(_login);
   entrance_fill_del(_login_fill);
}

Evas_Object *
entrance_login_add(Evas_Object *obj, void *data)
{
   Evas_Object *h;
   Evas_Object *l;
   Evas_Object *o;
   Evas_Object *p;

   /* layout */
   _login = calloc(1, sizeof(Entrance_Gui_Login));
   if (!_login)
     return NULL;
   _login->func.data = data;
   o = entrance_gui_theme_get(obj, ENTRANCE_EDJE_GROUP_LOGIN);

   /* login label */
   elm_object_part_text_set(o, ENTRANCE_EDJE_PART_LOGIN_LABEL, _("Login"));

   /* login */
   l = elm_entry_add(o);
   elm_entry_single_line_set(l, EINA_TRUE);
   elm_entry_scrollable_set(l, EINA_TRUE);
   elm_object_part_content_set(o, ENTRANCE_EDJE_PART_LOGIN, l);
   elm_object_focus_set(l, EINA_TRUE);
   evas_object_show(l);

   /* password label */
   elm_object_part_text_set(o, ENTRANCE_EDJE_PART_PASSWORD_LABEL, _("Password"));

   /* password */
   p = elm_entry_add(o);
   elm_entry_password_set(p, EINA_TRUE);
   elm_entry_single_line_set(p, EINA_TRUE);
   elm_entry_scrollable_set(p, EINA_TRUE);
   elm_object_part_content_set(o, ENTRANCE_EDJE_PART_PASSWORD, p);
   evas_object_show(p);

   /* callbacks */
   evas_object_smart_callback_add(l, "activated", _login_login_activated_cb, p);
   evas_object_smart_callback_add(l, "unfocused", _login_login_unfocused_cb, o);
   evas_object_smart_callback_add(l, "changed,user", _login_login_changed_cb, o);
   evas_object_smart_callback_add(p, "activated", _login_auth_check_cb, o);
   evas_object_smart_callback_add(p, "changed,user", _login_entry_changed_cb, o);
   elm_object_signal_callback_add(o, ENTRANCE_EDJE_SIGNAL_AUTH_CHECK, "",
                                  _entrance_login_auth_check_cb, o);
   h = elm_hoversel_add(o);
   elm_hoversel_hover_parent_set(h, obj);
   elm_object_part_content_set(o, ENTRANCE_EDJE_PART_XSESSIONS, h);
   _login_xsession_update(o);
   return o;
}

void
entrance_login_xsessions_populate(Evas_Object *widget, Eina_List *xsessions)
{
   PT("Session set");
   Evas_Object *o;

   o = elm_object_part_content_get(widget, ENTRANCE_EDJE_PART_XSESSIONS);
   entrance_fill(o, _login_fill, xsessions, NULL,
                 _login_xsession_clicked_cb, widget);
   _login->session = eina_list_data_get(xsessions);
   _login_xsession_update(widget);
}

void
entrance_login_login_set(Evas_Object *widget, const char *user)
{
   Evas_Object *o;
   o = elm_object_part_content_get(widget, ENTRANCE_EDJE_PART_LOGIN);
   elm_object_text_set(o, user);
   o = elm_object_part_content_get(widget, ENTRANCE_EDJE_PART_PASSWORD);
   elm_object_focus_set(o, EINA_TRUE);

  _login_xsession_guess(widget, user);
}

void
entrance_login_open_session_set(Evas_Object *widget, Eina_Bool open_session)
{
   Evas_Object *o;

   open_session = !!open_session;
   _login->open_session = open_session;
   o = elm_object_part_content_get(widget, ENTRANCE_EDJE_PART_XSESSIONS);
   if (!_login->open_session)
     elm_object_disabled_set(o, EINA_TRUE);
}
