#ifndef ENTRANCE_SESSION_UTILS_H_
#define ENTRANCE_SESSION_UTILS_H_
#include <pwd.h>

/* Moved to stand alone file for testing only, single consumer */

/**
 * @brief Ensure valid shell, if pw_shell is empty, set it to the users shell
 *
 * @param pwd current user information in valid passwd struct
 */
void entrance_session_shell_set(struct passwd *pwd);

/**
 * @brief Set group and user id in passwd struct
 *
 * @param pwd current user information in valid passwd struct
 *
 * @return int 0 on successful, 1 if it fails
 */
int entrance_session_userid_set(const struct passwd *pwd);

#endif /* ENTRANCE_SESSION_UTILS_H_ */
