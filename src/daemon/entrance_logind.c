#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "entrance.h"
#include "entrance_logind.h"

#ifdef HAVE_LOGIND

static sd_login_monitor *_logind_monitor = NULL;
static Ecore_Fd_Handler *_logind_handler = NULL;
static Entrance_Logind_Cb _logind_callback = NULL;
static void *_logind_callback_data = NULL;

/* Monitor callback handler */
static Eina_Bool
_entrance_logind_monitor_cb(void *data EINA_UNUSED, Ecore_Fd_Handler *fd_handler EINA_UNUSED)
{
   if (!_logind_monitor) return ECORE_CALLBACK_CANCEL;
   
   if (sd_login_monitor_flush(_logind_monitor) < 0)
     {
        PT("Failed to flush logind monitor");
        return ECORE_CALLBACK_CANCEL;
     }

   if (_logind_callback)
     _logind_callback(_logind_callback_data);

   return ECORE_CALLBACK_RENEW;
}

Eina_Bool
entrance_logind_init(void)
{
   PT("Initializing logind integration");
   return EINA_TRUE;
}

void
entrance_logind_shutdown(void)
{
   PT("Shutting down logind integration");
   entrance_logind_monitor_stop();
}

Entrance_Logind_Session *
entrance_logind_session_get(pid_t pid)
{
   Entrance_Logind_Session *session;
   char *session_id = NULL;
   char *seat = NULL;
   unsigned int vtnr = 0;
   uid_t uid;
   int ret;

   ret = sd_pid_get_session(pid, &session_id);
   if (ret < 0)
     {
        PT("Failed to get session for PID %d: %s", pid, strerror(-ret));
        return NULL;
     }

   session = calloc(1, sizeof(Entrance_Logind_Session));
   if (!session)
     {
        free(session_id);
        return NULL;
     }

   session->id = session_id;
   session->leader = pid;

   /* Get session seat */
   ret = sd_session_get_seat(session_id, &seat);
   if (ret >= 0)
     session->seat = seat;

   /* Get VT number */
   ret = sd_session_get_vt(session_id, &vtnr);
   if (ret >= 0)
     session->vtnr = vtnr;

   /* Get UID */
   ret = sd_session_get_uid(session_id, &uid);
   if (ret >= 0)
     session->uid = uid;

   /* Check if active */
   ret = sd_session_is_active(session_id);
   session->active = (ret > 0) ? EINA_TRUE : EINA_FALSE;

   PT("Got logind session: id=%s seat=%s vt=%u uid=%u active=%d",
      session->id, session->seat ? session->seat : "none",
      session->vtnr, session->uid, session->active);

   return session;
}

void
entrance_logind_session_free(Entrance_Logind_Session *session)
{
   if (!session) return;
   
   free(session->id);
   free(session->seat);
   free(session);
}

Entrance_Logind_Seat *
entrance_logind_seat_get(const char *seat_name)
{
   Entrance_Logind_Seat *seat;
   char *active_session = NULL;
   uid_t active_uid;
   int ret;

   if (!seat_name) seat_name = "seat0";

   seat = calloc(1, sizeof(Entrance_Logind_Seat));
   if (!seat) return NULL;

   seat->name = strdup(seat_name);

   /* Check capabilities */
   ret = sd_seat_can_graphical(seat_name);
   seat->can_graphical = (ret > 0) ? EINA_TRUE : EINA_FALSE;

   ret = sd_seat_can_tty(seat_name);
   seat->can_tty = (ret > 0) ? EINA_TRUE : EINA_FALSE;

   /* Get active session */
   ret = sd_seat_get_active(seat_name, &active_session, &active_uid);
   if (ret >= 0)
     seat->active_session = active_session;

   PT("Got seat info: name=%s graphical=%d tty=%d active=%s",
      seat->name, seat->can_graphical, seat->can_tty,
      seat->active_session ? seat->active_session : "none");

   return seat;
}

void
entrance_logind_seat_free(Entrance_Logind_Seat *seat)
{
   if (!seat) return;
   
   free(seat->name);
   free(seat->active_session);
   free(seat);
}

char **
entrance_logind_seats_list(int *count)
{
   char **seats = NULL;
   int ret;

   ret = sd_get_seats(&seats);
   if (ret < 0)
     {
        PT("Failed to get seats list: %s", strerror(-ret));
        if (count) *count = 0;
        return NULL;
     }

   if (count) *count = ret;
   PT("Found %d seats", ret);
   return seats;
}

Eina_Bool
entrance_logind_session_is_active(const char *session_id)
{
   int ret;

   if (!session_id) return EINA_FALSE;

   ret = sd_session_is_active(session_id);
   if (ret < 0)
     {
        PT("Failed to check session active state: %s", strerror(-ret));
        return EINA_FALSE;
     }

   return (ret > 0) ? EINA_TRUE : EINA_FALSE;
}

unsigned int
entrance_logind_vt_get(const char *display EINA_UNUSED)
{
   Entrance_Logind_Session *session;
   unsigned int vtnr = 0;
   pid_t pid;

   /* Get current process session */
   pid = getpid();
   session = entrance_logind_session_get(pid);
   if (session)
     {
        vtnr = session->vtnr;
        entrance_logind_session_free(session);
     }

   if (vtnr == 0)
     {
        /* Fallback: try to detect from parent or use configured value */
        PT("Could not detect VT from logind, using fallback");
     }

   return vtnr;
}

char *
entrance_logind_seat_detect(void)
{
   char *seat = NULL;
   int ret;

   /* Try to get seat from our PID */
   ret = sd_pid_get_session(getpid(), NULL);
   if (ret >= 0)
     {
        Entrance_Logind_Session *session = entrance_logind_session_get(getpid());
        if (session && session->seat)
          {
             seat = strdup(session->seat);
             entrance_logind_session_free(session);
             PT("Auto-detected seat: %s", seat);
             return seat;
          }
        if (session)
          entrance_logind_session_free(session);
     }

   /* Default to seat0 */
   seat = strdup("seat0");
   PT("Using default seat: %s", seat);
   return seat;
}

Eina_Bool
entrance_logind_monitor_start(Entrance_Logind_Cb callback, void *data)
{
   int fd, ret;

   if (_logind_monitor)
     {
        PT("Logind monitor already started");
        return EINA_FALSE;
     }

   ret = sd_login_monitor_new("seat", &_logind_monitor);
   if (ret < 0)
     {
        PT("Failed to create logind monitor: %s", strerror(-ret));
        return EINA_FALSE;
     }

   fd = sd_login_monitor_get_fd(_logind_monitor);
   if (fd < 0)
     {
        PT("Failed to get monitor FD: %s", strerror(-fd));
        sd_login_monitor_unref(_logind_monitor);
        _logind_monitor = NULL;
        return EINA_FALSE;
     }

   _logind_callback = callback;
   _logind_callback_data = data;

   _logind_handler = ecore_main_fd_handler_add(fd, ECORE_FD_READ,
                                                _entrance_logind_monitor_cb,
                                                NULL, NULL, NULL);
   if (!_logind_handler)
     {
        PT("Failed to add fd handler for logind monitor");
        sd_login_monitor_unref(_logind_monitor);
        _logind_monitor = NULL;
        return EINA_FALSE;
     }

   PT("Logind monitor started");
   return EINA_TRUE;
}

void
entrance_logind_monitor_stop(void)
{
   if (_logind_handler)
     {
        ecore_main_fd_handler_del(_logind_handler);
        _logind_handler = NULL;
     }

   if (_logind_monitor)
     {
        sd_login_monitor_unref(_logind_monitor);
        _logind_monitor = NULL;
     }

   _logind_callback = NULL;
   _logind_callback_data = NULL;

   PT("Logind monitor stopped");
}

char **
entrance_logind_user_sessions_get(uid_t uid, Eina_Bool active_only, int *count)
{
   char **sessions = NULL;
   int ret;

   ret = sd_uid_get_sessions(uid, active_only ? 1 : 0, &sessions);
   if (ret < 0)
     {
        PT("Failed to get user sessions for UID %u: %s", uid, strerror(-ret));
        if (count) *count = 0;
        return NULL;
     }

   if (count) *count = ret;
   PT("Found %d sessions for UID %u (active_only=%d)", ret, uid, active_only);
   return sessions;
}

#endif /* HAVE_LOGIND */
