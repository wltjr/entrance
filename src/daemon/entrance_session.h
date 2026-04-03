#ifndef ENTRANCE_SESSION_H_
#define ENTRANCE_SESSION_H_
#include <pwd.h>

/**
 * @brief Allocate memory for Entrance_Session structs, one per X server/seat
 *
 * @param count the number of sessions to allocate memory
 */
void entrance_sessions_init(int count);

void entrance_session_display_set(const char *dname);
void entrance_session_cookie(void);
void entrance_session_shutdown(void);
Eina_Bool entrance_session_authenticate(const char *login, const char *pwd);
void entrance_session_close(Eina_Bool opened);
Eina_Bool entrance_session_login(const char *command, Eina_Bool push);
void entrance_session_pid_set(pid_t pid);
pid_t entrance_session_pid_get(void);
char *entrance_session_login_get(void);
Eina_List *entrance_session_list_get(void);

#endif /* ENTRANCE_SESSION_H_ */
