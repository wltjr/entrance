#include "entrance.h"
#include <sys/file.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <Eina.h>
#include "Ecore_Getopt.h"
#include <xcb/xcb.h>

#define ENTRANCE_CONFIG_HOME_PATH PACKAGE_CACHE"/client"

static Eina_Bool _entrance_autologin_lock_get(void);
static Eina_Bool _entrance_client_error(void *data, int type, void *event);
static Eina_Bool _entrance_client_data(void *data, int type, void *event);
static Eina_Bool _entrance_client_del(void *data, int type, void *event);
static Eina_Bool _open_log();
static void _entrance_autologin_lock_set(void);
static void _entrance_client_handlers_del();
static void _entrance_kill_and_wait(const char *desc, pid_t pid);
static void _entrance_session_wait();
static void _entrance_start_client(const char *entrance_display);
static void _entrance_uid_gid_init();
static void _remove_lock();
static void _signal_cb(int sig);
static void _signal_log(int sig);

static Eina_Bool _entrance_auto_login = EINA_FALSE;
static Eina_Bool _xephyr = 0;
static Ecore_Exe *_entrance_client = NULL;
static Eina_List *_entrance_client_handlers = NULL;

static char *entrance_display = NULL;
static char *entrance_home_path = NULL;
static const char *entrance_user = NULL;
static int entrance_signal = 0;
static pid_t entrance_client_pid = 0;
static gid_t entrance_gid = 0;
static uid_t entrance_uid = 0;
static pid_t entrance_xserver_pid = 0;

int _entrance_log;
int _entrance_client_log;

static const Ecore_Getopt options =
{
  PACKAGE,
  "%prog [options]",
  VERSION,
  "(C) 2025 William L Thomson Jr, see AUTHORS.",
  "GPL, see COPYING",
  "Entrance is a login manager, written using core efl libraries",
  EINA_TRUE,
  {
    ECORE_GETOPT_STORE_TRUE('x', "xephyr", "run under Xephyr."),
    ECORE_GETOPT_HELP ('h', "help"),
    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_COPYRIGHT('R', "copyright"),
    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_SENTINEL
  }
};


static Eina_Bool
_entrance_autologin_lock_get(void)
{
   FILE *f;
   double sleep_time;
   double uptime;
   struct stat st_login;

   f = fopen("/proc/uptime", "r");
   if (f)
     {
        char buf[4096];

        fgets(buf,  sizeof(buf), f);
        if(fscanf(f, "%lf %lf", &uptime, &sleep_time) <= 0)
          PT("Could not read uptime input stream");
        fclose(f);
        if (stat("/var/run/entrance/login", &st_login) > 0)
           return EINA_FALSE;
        else
           return EINA_TRUE;
     }
   return EINA_FALSE;
}

static void
_entrance_autologin_lock_set(void)
{
   system("touch "PACKAGE_CACHE"/login");
}

static Eina_Bool
_entrance_client_data(void *d EINA_UNUSED, int t EINA_UNUSED, void *event)
{
   char buf[4096];
   const Ecore_Exe_Event_Data *ev;
   size_t size;

   ev = event;
   size = ev->size;


   if ((unsigned int)ev->size > sizeof(buf) - 1)
     size = sizeof(buf) - 1;

   memcpy(buf, ev->data, size);
   buf[size] = '\0';  /* Ensure null termination */
   EINA_LOG_DOM_INFO(_entrance_client_log, "%s", buf);
   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_entrance_client_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   const Ecore_Exe_Event_Del *ev;

   ev = event;
   if (ev->exe != _entrance_client)
     return ECORE_CALLBACK_PASS_ON;
   PT("client terminated");
   _entrance_client = NULL;
   _entrance_session_wait();
    if(!entrance_signal && !_xephyr)
      {
         PT("stopping X server");
         entrance_xserver_shutdown();
         _entrance_kill_and_wait("xserver", entrance_xserver_pid);
         PT("closing session");
         entrance_session_close(EINA_TRUE);
         PT("session shutdown");
         entrance_session_shutdown();
         PT("session init");
         entrance_session_init(entrance_display);
         entrance_session_cookie();
         PT("restarting X server");
         entrance_xserver_pid = entrance_xserver_init(_entrance_start_client,
                                                      entrance_display);
         PT("X server restarted pid %d",entrance_xserver_pid);
      }
    else
      {
        PT("stopping server");
        PT("closing session on exit");
        entrance_session_close(EINA_TRUE);
        entrance_session_shutdown();
        ecore_main_loop_quit();
      }
   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_entrance_client_error(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   char buf[4096];
   const Ecore_Exe_Event_Data *ev;
   size_t size;

   ev = event;
   size = ev->size;

   if ((unsigned int)ev->size > sizeof(buf) - 1)
     size = sizeof(buf) - 1;

   memcpy(buf, ev->data, size);
   buf[size] = '\0';  /* Ensure null termination */
   EINA_LOG_DOM_WARN(_entrance_client_log, "%s", buf);
   return ECORE_CALLBACK_DONE;
}

static void
_entrance_client_handlers_del()
{
  Ecore_Event_Handler *h;

  EINA_LIST_FREE(_entrance_client_handlers, h)
    ecore_event_handler_del(h);
}

static void
_entrance_kill_and_wait(const char *desc, pid_t pid)
{
  PT("kill %s",desc);
  kill(pid, SIGTERM);
  while (waitpid(pid, NULL, WUNTRACED | WCONTINUED) > 0)
    {
       PT("Waiting on %s...", desc);
       sleep(1);
    }
}

static void
_entrance_session_wait()
{
  pid_t session_pid = 0;
  char proc_path[64];
  int wait_result;

  if((session_pid = entrance_session_pid_get())>0)
    {
      PT("session running pid %d, waiting...", session_pid);
      snprintf(proc_path, sizeof(proc_path), "/proc/%d", session_pid);
      
      /* Try waitpid() - may fail with ECHILD if backgrounded by init system */
      while (!entrance_signal)
        {
          struct timespec request = { 0, 500000000 };
          wait_result = waitpid(session_pid, NULL, WNOHANG);
          
          if (wait_result > 0)
            {
              /* Child terminated */
              PT("session pid %d exited", session_pid);
              break;
            }
          else if (wait_result < 0)
            {
              if (errno == ECHILD)
                {
                  /* Not a direct child (backgrounded by OpenRC), poll /proc instead */
                  PT("session not direct child, polling /proc/%d", session_pid);
                  while (!entrance_signal)
                    {
                      if (access(proc_path, F_OK) != 0)
                        {
                          PT("session pid %d terminated", session_pid);
                          break;
                        }
                      nanosleep(&request, NULL); /* Check every 500ms */
                    }
                  break;
                  break;
                }
              else
                {
                  /* Other error */
                  PT("waitpid error: %d", errno);
                }
                break;
            }
          /* wait_result == 0: child still running, sleep and check again */
          nanosleep(&request, NULL);
        }
    }
}

static void
_entrance_start_client(const char *entrance_display)
{
   char *home_path = ENTRANCE_CONFIG_HOME_PATH;
   int home_dir;
   struct stat st;
   Ecore_Event_Handler *h;

   if (_entrance_client)
     return;
   PT("starting client...");
   _entrance_client_handlers_del();
   h = ecore_event_handler_add(ECORE_EXE_EVENT_DEL, _entrance_client_del, NULL);
   _entrance_client_handlers = eina_list_append(_entrance_client_handlers, h);
   h = ecore_event_handler_add(ECORE_EXE_EVENT_ERROR, _entrance_client_error, NULL);
   _entrance_client_handlers = eina_list_append(_entrance_client_handlers, h);
   h = ecore_event_handler_add(ECORE_EXE_EVENT_DATA, _entrance_client_data, NULL);
   _entrance_client_handlers = eina_list_append(_entrance_client_handlers, h);
   if(entrance_home_path)
       home_path = entrance_home_path;
   home_dir = open(home_path, O_RDONLY);
   if(!home_dir || home_dir<0)
     {
       PT("Failed to open home directory %s", home_path);
       ecore_main_loop_quit();
       return;
     }
   if(flock(home_dir, LOCK_SH)==-1)
     {
       PT("Failed to lock home directory %s", home_path);
       close(home_dir);
       ecore_main_loop_quit();
       return;
     }
   if(fstat(home_dir, &st)!= -1)
     {
       char buf[PATH_MAX];

       if ((st.st_uid != entrance_uid)
           || (st.st_gid != entrance_gid))
         {
            PT("chown home directory %s", home_path);
            fchown(home_dir, entrance_uid, entrance_gid);
         }
       snprintf(buf, sizeof(buf),
                "export HOME='%s'; export USER='%s';"
                "export LD_LIBRARY_PATH='"PACKAGE_LIB_DIR"';%s "
                PACKAGE_BIN_DIR"/entrance_client -d '%s' -t '%s' -g %d -u %d -p %d",
                home_path, entrance_user, entrance_config->command.session_login ? entrance_config->command.session_login : "",
                entrance_display, entrance_config->theme,
                entrance_gid,entrance_uid, entrance_config->port);
       PT("Exec entrance_client: %s", buf);

       _entrance_client =
          ecore_exe_pipe_run(buf,
                             ECORE_EXE_PIPE_READ | ECORE_EXE_PIPE_ERROR,
                             NULL);
       if(_entrance_client)
         {
           entrance_client_pid = ecore_exe_pid_get(_entrance_client);
           PT("entrance_client started pid %d", entrance_client_pid);
         }
     }
   flock(home_dir, LOCK_UN);
   close(home_dir);
}

static void
_entrance_uid_gid_init()
{
   struct passwd pwd_buf;
   struct passwd *pwd = NULL;
   char buf[4096];
   int result;

   if (entrance_config->start_user
       && entrance_config->start_user[0])
     {
       result = getpwnam_r(entrance_config->start_user, &pwd_buf, buf, sizeof(buf), &pwd);
       if (result != 0 || !pwd)
         pwd = NULL;
     }
   if (!pwd)
     {
       PT("The given user %s, is not valid."
          "Falling back to nobody", entrance_config->start_user);
       result = getpwnam_r("nobody", &pwd_buf, buf, sizeof(buf), &pwd);
       if (result != 0 || !pwd)
         {
           PT("Critical: Cannot get nobody user information");
           return;
         }
       entrance_user = "nobody";
     }
   else
     entrance_user = entrance_config->start_user;
   PT("running under user : %s",entrance_user);
   entrance_gid = pwd->pw_gid;
   entrance_uid = pwd->pw_uid;
   if (!pwd->pw_dir ||
       !strcmp(pwd->pw_dir, "/") ||
       !strcmp(pwd->pw_dir, "/nonexistent"))
     {
        PT("No home directory for client");
        if (!ecore_file_exists(ENTRANCE_CONFIG_HOME_PATH))
          {
             PT("Creating new home directory for client");
             ecore_file_mkdir(ENTRANCE_CONFIG_HOME_PATH);
             chown(ENTRANCE_CONFIG_HOME_PATH, entrance_uid, entrance_gid);
          }
        else
          {
             if (!ecore_file_is_dir(ENTRANCE_CONFIG_HOME_PATH))
               {
                  PT("Replacing existing file "
                     ENTRANCE_CONFIG_HOME_PATH
                     " with a directory");
                  ecore_file_remove(ENTRANCE_CONFIG_HOME_PATH);
                  ecore_file_mkdir(ENTRANCE_CONFIG_HOME_PATH);
                  chown(ENTRANCE_CONFIG_HOME_PATH, entrance_uid, entrance_gid);
               }
          }
        entrance_home_path = strdup(ENTRANCE_CONFIG_HOME_PATH);
     }
   else
     entrance_home_path = strdup(pwd->pw_dir);
   PT("Home directory %s", entrance_home_path);
}

static Eina_Bool
_get_lock()
{
   FILE *f;
   char buf[128];
   int my_pid;

   my_pid = getpid();
   f = fopen(entrance_config->lockfile, "r");
   if (!f)
     {
        /* No lockfile, so create one */
        f = fopen(entrance_config->lockfile, "w");
        if (!f)
          {
             PT("Could n0t create lockfile!");
             return (EINA_FALSE);
          }
        snprintf(buf, sizeof(buf), "%d", my_pid);
        if (!fwrite(buf, strlen(buf), 1, f))
          {
             fclose(f);
             PT("Could not write the lockfile");
             return EINA_FALSE;
          }
        fclose(f);
     }
   else
     {
        int pid = 0;
        /* read the lockfile */
        if (fgets(buf, sizeof(buf), f))
          pid = atoi(buf);
        fclose(f);
        if (pid == my_pid)
          return EINA_TRUE;

        PT("A lock file is present, another instance may exist ?");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_open_log()
{
   FILE *elog;
   elog = fopen(entrance_config->logfile, "a");
   if (!elog)
     {
        PT("could not open logfile !");
        return EINA_FALSE;
     }
   fclose(elog);
   if (!freopen(entrance_config->logfile, "a", stdout))
     PT("Error on reopen stdout");
   setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
   if (!freopen(entrance_config->logfile, "a", stderr))
     PT("Error on reopen stderr");
   setvbuf(stderr, NULL, _IONBF, BUFSIZ);
   return EINA_TRUE;
}

static void
_remove_lock()
{
   if(remove(entrance_config->lockfile)== -1)
     PT("Could not remove lockfile");
}

static void
_signal_cb(int sig)
{
   pid_t session_pid = 0;

   PT("signal %d received", sig);
   entrance_signal = sig;
   if (_entrance_client)
     {
       PT("terminate client");
       _entrance_kill_and_wait("entrance_client", entrance_client_pid);
     }
   else
     {
       if((session_pid = entrance_session_pid_get())>0)
           _entrance_kill_and_wait("session", session_pid);
     }
   ecore_main_loop_quit();
}

static void
_signal_log(int sig EINA_UNUSED)
{
   PT("reopen the log file");
   entrance_close_log();
   _open_log();
}

Eina_Bool entrance_auto_login_enabled()
{
  return _entrance_auto_login;
}

void
entrance_client_pid_set(pid_t pid)
{
    entrance_client_pid = pid;
}

void
entrance_close_log()
{
  fclose(stderr);
  fclose(stdout);
}

int
main (int argc, char ** argv)
{
   int args;
   unsigned char quit_option = 0;

   Ecore_Getopt_Value values[] =
     {
        ECORE_GETOPT_VALUE_BOOL(_xephyr),
        ECORE_GETOPT_VALUE_BOOL(quit_option),
        ECORE_GETOPT_VALUE_BOOL(quit_option),
        ECORE_GETOPT_VALUE_BOOL(quit_option),
        ECORE_GETOPT_VALUE_BOOL(quit_option),
        ECORE_GETOPT_VALUE_NONE
     };

   if (getuid() != 0)
     {
        fprintf(stderr, "Sorry, only root can run this program!");
        return 1;
     }
   
   args = ecore_getopt_parse(&options, values, argc, argv);
   if (args < 0)
     {
        fprintf(stderr, "ERROR: could not parse options.");
        return -1;
     }

   if (quit_option)
     return 0;

   eina_init();
   eina_log_threads_enable();
   ecore_init();
   _entrance_log = eina_log_domain_register("entrance", EINA_COLOR_CYAN);
   _entrance_client_log = eina_log_domain_register("entrance_client",
                                                   EINA_COLOR_CYAN);
   eina_log_domain_level_set("entrance", 5);
   eina_log_domain_level_set("entrance_client", 5);

   eet_init();
   efreet_init();
   entrance_config_init();
   if (!entrance_config)
     {
        PT("No config loaded, sorry must quit ...");
        exit(1);
     }


   _entrance_auto_login = entrance_config->autologin;
   entrance_display = strdup(entrance_config->command.xdisplay);

   if (!_xephyr && !_get_lock())
        exit(1);

   if (!_open_log())
     {
        PT("Unable to open log file !!!!");
        entrance_config_shutdown();
        exit(1);
     }

   time_t t = time(0);
   struct tm local_tm;
   struct tm tm = *localtime_r(&t, &local_tm);
   char date[32];
   strftime(date,32,"%a %b %d %T %Y",&tm);

   PT("Starting "PACKAGE_STRING" %s",date);
   /* initialize event handler */

   signal(SIGQUIT, _signal_cb);
   signal(SIGTERM, _signal_cb);
   signal(SIGKILL, _signal_cb);
   signal(SIGINT, _signal_cb);
   signal(SIGHUP, _signal_cb);
   signal(SIGPIPE, _signal_cb);
   signal(SIGALRM, _signal_cb);
   signal(SIGUSR2, _signal_log);

   PT("session init");
   entrance_session_init(entrance_display);
   entrance_session_cookie();

   if(!_entrance_auto_login)
     _entrance_uid_gid_init();

   if (_xephyr == EINA_FALSE)
     {
        PT("xserver init");
        entrance_xserver_pid = entrance_xserver_init(_entrance_start_client,
                                                     entrance_display);
        PT("X server started pid %d",entrance_xserver_pid);
     }
   else
     _entrance_start_client(entrance_display);

   PT("history init");
   entrance_history_init();
   if (_entrance_auto_login && _entrance_autologin_lock_get())
     {
        PT("autologin init");
        xcb_connection_t *disp = NULL;
        disp = xcb_connect(entrance_display, NULL);
#ifdef HAVE_PAM
        PT("pam init");
        char tty_name[16];
#ifdef HAVE_LOGIND
        unsigned int logind_vt = entrance_logind_vt_get(entrance_display);
        if (logind_vt > 0)
          snprintf(tty_name, sizeof(tty_name), "tty%u", logind_vt);
        else
#endif
          snprintf(tty_name, sizeof(tty_name), "tty%u", entrance_config->command.vtnr);
        
        entrance_pam_init("entrance-autologin", tty_name, entrance_config->userlogin);
#endif
        PT("login user");
        entrance_session_login(entrance_config->session, EINA_FALSE);
        sleep(30);
        xcb_disconnect(disp);
        _entrance_session_wait();
 
        if(!entrance_signal)
          {
             _entrance_auto_login = EINA_FALSE;
             _entrance_uid_gid_init();
             _entrance_start_client(entrance_display);
          }
     }

   PT("action init");
   entrance_action_init();
   PT("server init");
   entrance_server_init(entrance_uid, entrance_gid);
   PT("starting main loop");
   ecore_main_loop_begin();
   PT("main loop end");
   _entrance_client_handlers_del();
   entrance_server_shutdown();
   PT("server shutdown");
   entrance_action_shutdown();
   PT("action shutdown");
   entrance_history_shutdown();
   PT("history shutdown");
   if (!_xephyr)
     {
        entrance_xserver_shutdown();
        PT("xserver shutdown");
     }
#ifdef HAVE_PAM
   entrance_pam_shutdown();
   PT("pam shutdown");
#endif
   _entrance_autologin_lock_set();
   PT("ecore shutdown");
   ecore_shutdown();
   PT("session shutdown");
   entrance_session_shutdown();
   _remove_lock();
   PT("config shutdown");
   entrance_config_shutdown();
   PT("eet shutdown");
   eet_shutdown();
   if(entrance_home_path)
     free(entrance_home_path);
   free(entrance_display);
   if (!_xephyr)
     {
        PT("ending xserver");
        _entrance_kill_and_wait("xserver", entrance_xserver_pid);
     }
   else
     PT("No session to wait, exiting");
   PT("close log and exit\n\n");
   entrance_close_log();
   efreet_shutdown();
   return 0;
}

