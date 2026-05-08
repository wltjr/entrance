#ifndef ENTRANCE_LOGIND_H_
#define ENTRANCE_LOGIND_H_

#ifdef HAVE_LOGIND

#include <systemd/sd-login.h>
#include <Eina.h>

typedef struct _Entrance_Logind_Seat
{
   char *name;                  /* Seat name (e.g., "seat0") */
   Eina_Bool can_graphical;     /* Can run graphical sessions */
   Eina_Bool can_tty;           /* Can use TTY */
   char *active_session;        /* Currently active session ID */
} Entrance_Logind_Seat;

/* Get seat information */
Entrance_Logind_Seat *entrance_logind_seat_get(const char *seat_name);

/* Free seat structure */
void entrance_logind_seat_free(Entrance_Logind_Seat *seat);

/* Get available seats */
char **entrance_logind_seats_list(int *count);

/* Monitor session/seat changes */
typedef void (*Entrance_Logind_Cb)(void *data);
Eina_Bool entrance_logind_monitor_start(Entrance_Logind_Cb callback, void *data);
void entrance_logind_monitor_stop(void);

#endif /* HAVE_LOGIND */

#endif /* ENTRANCE_LOGIND_H_ */
