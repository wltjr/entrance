#ifndef ENTRANCE_CONNECT_
#define ENTRANCE_CONNECT_

typedef void (*Entrance_Connect_Auth_Cb)(void *data, const char *login, Eina_Bool granted);

Eina_Bool entrance_connect(int port);
void entrance_connect_auth_send(const char *login, const char *password, const char *session, Eina_Bool open_session);
void entrance_connect_action_send(unsigned char id);
void entrance_connect_conf_gui_send(const Entrance_Conf_Gui_Event *conf);
void entrance_connect_conf_user_send(const Entrance_Login *el);
void *entrance_connect_auth_cb_add(Entrance_Connect_Auth_Cb func, void *data);
void entrance_connect_auth_cb_del(void *list);
void entrance_connect_shutdown(void);
#endif /* ENTRANCE_CONNECT_ */
