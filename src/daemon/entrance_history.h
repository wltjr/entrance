#ifndef ENTRANCE_HISTORY_
#define ENTRANCE_HISTORY_

/**
 * @brief Check if user and session exist in history, if not add, if so update.
 *
 * @param login     user login name to add or update in history
 * @param session   user login session to add or update in history 
 */
void entrance_history_check(const char *login, const char *session);
void entrance_history_init(void);
void entrance_history_shutdown(void);
Eina_List *entrance_history_get(void);
void entrance_history_user_update(const Entrance_Login *el);

typedef struct _Entrance_History Entrance_History;

struct _Entrance_History
{
   Eina_List *history;
};


#endif /* ENTRANCE_HISTORY_ */
