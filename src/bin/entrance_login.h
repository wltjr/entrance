#ifndef ENTRANCE_LOGIN_H
#define ENTRANCE_LOGIN_H

typedef void (*Entrance_Login_Cb) (void *data, const char *user, Eina_Bool granted);

void entrance_login_init(void);
void entrance_login_shutdown(void);
Evas_Object *entrance_login_add(Evas_Object *win, void *data);
void entrance_login_xsessions_populate(Evas_Object *widget, const Eina_List *xsessions);
void entrance_login_login_set(Evas_Object *widget, const char *user);
void entrance_login_open_session_set(const Evas_Object *obj, Eina_Bool open_session);
void entrance_login_callback_set(Entrance_Login_Cb greater_cb, void *data);

#endif
