#include "entrance_client.h"
#include <Ecore.h>
#include <Ecore_X.h>
#include <Ecore_Getopt.h>
#include <errno.h>
#include <string.h>

#define NOBODY 65534

static Eina_Bool _entrance_server_alive_cb(void *data EINA_UNUSED);
static Ecore_Timer *check_timer;
static pid_t entrance_server_pid;

static const Ecore_Getopt options =
{
   "entrance_client",
   "%prog [options]",
   VERSION,
   "(C) 2025 William L Thomson Jr, see AUTHORS.",
   "GPL, see COPYING",
   "Launch gui client.",
   EINA_TRUE,
   {
      ECORE_GETOPT_STORE_STR('d', "display", "specify the display to use"),
      ECORE_GETOPT_STORE_INT('g', "gid", "specify the group to run under"),
      ECORE_GETOPT_STORE_USHORT('p', "port", "specify the port to use"),
      ECORE_GETOPT_STORE_STR('t', "theme", "specify the theme to use"),
      ECORE_GETOPT_STORE_INT('u', "uid", "specify the user to run under"),
      ECORE_GETOPT_HELP ('h', "help"),
      ECORE_GETOPT_VERSION('V', "version"),
      ECORE_GETOPT_COPYRIGHT('R', "copyright"),
      ECORE_GETOPT_LICENSE('L', "license"),
      ECORE_GETOPT_SENTINEL
   }
};

static Eina_Bool
_entrance_server_alive_cb(void *data EINA_UNUSED)
{
  kill(entrance_server_pid,0);
  if(errno==ESRCH)
    {
      fprintf(stderr,"server crashed stopping client\n");
      kill(getpid(),SIGTERM);
    }
  return ECORE_CALLBACK_RENEW;
}

void
entrance_monitor_server_pid(pid_t pid)
{
  PT("monitor server pid %d", pid);
  entrance_server_pid = pid;
  check_timer = ecore_timer_add(5.0, _entrance_server_alive_cb, NULL);
}

int
main(int argc, char **argv)
{
   char *display = NULL;
   char *theme = NULL;
   int args;
   int gid = 0;
   int port = 0;
   int uid = 0;
   unsigned char quit_option = 0;

   Ecore_Getopt_Value values[] =
     {
        ECORE_GETOPT_VALUE_STR(display),
        ECORE_GETOPT_VALUE_INT(gid),
        ECORE_GETOPT_VALUE_INT(port),
        ECORE_GETOPT_VALUE_STR(theme),
        ECORE_GETOPT_VALUE_INT(uid),
        ECORE_GETOPT_VALUE_BOOL(quit_option),
        ECORE_GETOPT_VALUE_BOOL(quit_option),
        ECORE_GETOPT_VALUE_BOOL(quit_option),
        ECORE_GETOPT_VALUE_BOOL(quit_option)
     };
   args = ecore_getopt_parse(&options, values, argc, argv);
   if (args < 0)
     return EXIT_FAILURE;
   if (quit_option)
     return EXIT_SUCCESS;
   if(gid>0)
     {
       if (setgid(gid) != 0)
         {
           fprintf(stderr, "Failed to set gid %d: %s\n", gid, strerror(errno));
           return EXIT_FAILURE;
         }
     }
   else
     {
       PT("root gid not allowed, defaulting to nobody");
       if (setgid(NOBODY) != 0)
         {
           fprintf(stderr, "Failed to set gid to nobody: %s\n", strerror(errno));
           return EXIT_FAILURE;
         }
     }
   if(uid>0)
     {
       if (setuid(uid) != 0)
         {
           fprintf(stderr, "Failed to set uid %d: %s\n", uid, strerror(errno));
           return EXIT_FAILURE;
         }
     }
   else
     {
       PT("root uid not allowed, defaulting to nobody");
       if (setuid(NOBODY) != 0)
         {
           fprintf(stderr, "Failed to set uid to nobody: %s\n", strerror(errno));
           return EXIT_FAILURE;
         }
     }
   eina_init();
   if (!display)
     display = ":0";
   if(!port)
     port = 42;
   if (!theme)
     theme = "default";
   ecore_init();
   ecore_x_init(display);
   elm_init(argc, argv);
   PT("login init");
   entrance_login_init();
   PT("gui init");
   if (!entrance_gui_init(theme)) return EXIT_FAILURE;
   PT("connect init");
   if(entrance_connect(port))
     elm_run();
   PT("stop server monitor");
   ecore_timer_del(check_timer);
   PT("connect shutdown");
   entrance_connect_shutdown();
   PT("gui shutdown");
   entrance_gui_shutdown();
   PT("login shutdown");
   entrance_login_shutdown();
   PT("elm shutdown");
   elm_shutdown();
   PT("ecore x shutdown");
   ecore_x_shutdown();
   PT("ecore shutdown");
   ecore_shutdown();
   PT("eina shutdown");
   eina_shutdown();
   PT("exit");
   return EXIT_SUCCESS;
}

