#ifndef ENTRANCE_SESSION_H_
#define ENTRANCE_SESSION_H_
#include <pwd.h>

/**
 * @brief Allocate memory for Entrance_Session structs, one per X server/seat
 *
 * @param count the number of sessions to allocate memory
 */
void entrance_sessions_init(int count);

/**
 * @brief Start an entrance session, for a X server/seat
 *
 * @param id the id of the X server, array index for now, could be seat id later
 * @param display the display number with colon
 * @param vt the virtual terminal number
 */
void entrance_session_start(int id, const char *display, int vt);

void entrance_session_display_set(const char *dname);
void entrance_session_cookie(void);
void entrance_session_shutdown(void);

/**
 * @brief Shutdown all sessions ( free array pointer, may have other usage later )
 */
void entrance_sessions_shutdown();

Eina_Bool entrance_session_authenticate(const char *login, const char *pwd);
void entrance_session_close(Eina_Bool opened);
Eina_Bool entrance_session_login(const char *command, Eina_Bool push);
void entrance_session_pid_set(pid_t pid);
pid_t entrance_session_pid_get(void);
char *entrance_session_login_get(void);
Eina_List *entrance_session_list_get(void);

#endif /* ENTRANCE_SESSION_H_ */
