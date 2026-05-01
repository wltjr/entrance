#ifndef ENTRANCE_HISTORY_
#define ENTRANCE_HISTORY_

void entrance_history_init(void);
void entrance_history_shutdown(void);
void entrance_history_push(const char *login, const char *session);
Eina_List *entrance_history_get(void);
void entrance_history_user_update(const Entrance_Login *el);

typedef struct _Entrance_History Entrance_History;

struct _Entrance_History
{
   Eina_List *history;
};


#endif /* ENTRANCE_HISTORY_ */
