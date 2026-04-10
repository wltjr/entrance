#ifndef ENTRANCE_CONNECT_
#define ENTRANCE_CONNECT_

typedef void (*Entrance_Connect_Auth_Cb)(void *data, const char *login, Eina_Bool granted);

/**
 * @brief Establish an entrance client connection to the entrance server
 * 
 * @param id the id of the client, to identify which client it is to server
 * @param port server port number, socket number
 *
 * @return Eina_Bool 
 */
Eina_Bool entrance_connect(int id, int port);

void entrance_connect_auth_send(const char *login, const char *password, const char *session, Eina_Bool open_session);
void entrance_connect_action_send(unsigned char id);
void entrance_connect_conf_gui_send(const Entrance_Conf_Gui_Event *conf);
void entrance_connect_conf_user_send(const Entrance_Login *el);
void *entrance_connect_auth_cb_add(Entrance_Connect_Auth_Cb func, void *data);
void entrance_connect_auth_cb_del(void *list);
void entrance_connect_shutdown(void);
#endif /* ENTRANCE_CONNECT_ */
