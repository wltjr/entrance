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
   int ret;
   
   PT("Initializing logind integration");
   
   /* Test systemd/elogind availability by attempting a basic call */
   ret = sd_get_seats(NULL);
   if (ret < 0)
     {
        if (ret == -ENOENT || ret == -ECONNREFUSED)
          {
             PT("WARNING: logind/systemd not available: %s", strerror(-ret));
             return EINA_FALSE;
          }
        PT("Error checking logind availability: %s", strerror(-ret));
        return EINA_FALSE;
     }
   
   PT("logind integration initialized successfully %d seats detected", ret);
   return EINA_TRUE;
}

void
entrance_logind_shutdown(void)
{
   PT("Shutting down logind integration");
   entrance_logind_monitor_stop();
   
   /* Note: Active session cleanup is handled by entrance_session_shutdown() */
}

void
entrance_logind_session_free(Entrance_Logind_Session *session)
{
   if (!session) return;
   
   /* Note: logind/elogind automatically manages session lifecycle via PAM.
    * When pam_close_session() is called (in entrance_session_close()),
    * logind is notified automatically through pam_systemd/pam_elogind.
    * We only track session info locally - actual lifecycle is managed by PAM.
    */
   if (session->id)
     PT("Freeing logind session tracking: %s", session->id);
   
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
   if (!seat->name)
     {
       free(seat);
       return NULL;
     }

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
entrance_logind_monitor_start(Entrance_Logind_Cb callback, void *data)
{
   int fd;
   int ret;

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

#endif /* HAVE_LOGIND */
