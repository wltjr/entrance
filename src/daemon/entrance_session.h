#ifndef ENTRANCE_SESSION_H_
#define ENTRANCE_SESSION_H_
#include <pwd.h>

void entrance_session_init(const char *dname);
void entrance_session_cookie(void);
void entrance_session_shutdown(void);
Eina_Bool entrance_session_authenticate(const char *login, const char *pwd);
void entrance_session_close(Eina_Bool opened);
Eina_Bool entrance_session_login(const char *command, Eina_Bool push);
void entrance_session_pid_set(pid_t pid);
pid_t entrance_session_pid_get(void);
long  entrance_session_seed_get(void);
char *entrance_session_login_get(void);
int entrance_session_logged_get(void);
Eina_List *entrance_session_list_get(void);

#endif /* ENTRANCE_SESSION_H_ */
