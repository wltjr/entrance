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

/**
 * @brief Create a cookie for a specific session
 *
 * @param id of session in array, index value for now, could be seat id later
 */
void entrance_session_cookie(int id);

/**
 * @brief Shutdown a specific session
 *
 * @param id of session in array, index value for now, could be seat id later
 */
void entrance_session_shutdown(int id);

/**
 * @brief Shutdown all sessions ( free array pointer, may have other usage later )
 */
void entrance_sessions_shutdown();

/**
 * @brief Authenticate a login session
 *
 * @param id of session in array, index value for now, could be seat id later
 * @param login user login name
 * @param pwd user password
 *
 * @return Eina_Bool true on successful login, false if it fails
 */
Eina_Bool entrance_session_authenticate(int id, const char *login, const char *pwd);

/**
 * @brief Start a x11 or wayland desktop shell login session
 *
 * @param id of client/session in array, index value for now, could be seat id later
 * @param session x11 or wayland desktop session to start
 * @param history add session to history
 *
 * @return Eina_Bool true of login session start succeeds, false if it fails
 */
Eina_Bool entrance_session_login(int id, const char *session, Eina_Bool history);

/**
 * @brief Get a desktop session pid
 *
 * @param id of client/session in array, index value for now, could be seat id later
 * @return pid_t of the desktop session
 */
pid_t entrance_session_pid_get(int id);

/**
 * @brief Get session login user name
 *
 * @param id of client/session in array, index value for now, could be seat id later
 *
 * @return char* session login user name
 */
char *entrance_session_login_get(int id);

Eina_List *entrance_session_list_get(void);

#endif /* ENTRANCE_SESSION_H_ */
