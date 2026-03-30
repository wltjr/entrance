#include "entrance.h"
#include <sys/wait.h>
#include <unistd.h>

typedef struct Entrance_Xserver_
{
    long id;
    const char *dname;
    Entrance_X_Cb start;
    Ecore_Event_Handler *handler_start;
} Entrance_Xserver;

int _xserver_count = 0;
Entrance_Xserver **_xservers;

/*
 * man Xserver
 * SIGUSR1 This  signal  is  used  quite  differently  from  either of the
 * above.  When the server starts, it checks to see if it has inherite
 * SIGUSR1 as SIG_IGN instead of the usual SIG_DFL.  In this case, the server
 * sends a SIGUSR1 to its parent process after it has set up the various
 * connection schemes.  Xdm uses this feature to recognize when connecting to
 * the server is possible.
 * */
static int
_xserver_start(char *display)
{
   char *abuf = NULL;
   char *buf = NULL;
   char **args = NULL;
   char *saveptr = NULL;
   char vt[128] = {0};
   pid_t pid;

   PT("Launching xserver");
   pid = fork();
   if (pid)
     {
        PT("Parent process: X server child pid %d", pid);
        return pid;  /* Parent returns child's PID */
     }

   /* Child process (pid == 0): will exec X server */
   PT("Child process: executing X server");
   char *token;
   int num_token = 0;
   signal(SIGTTIN, SIG_IGN);
   signal(SIGTTOU, SIG_IGN);
   signal(SIGUSR1, SIG_IGN);

   if (!(buf = strdup(entrance_config->command.xinit_args)))
     goto xserver_error;
   token = strtok_r(buf, " ", &saveptr);
   while(token)
     {
       ++num_token;
       token = strtok_r(NULL, " ", &saveptr);
     }
   free(buf);
   if (num_token)
     {
        if (!(abuf = strdup(entrance_config->command.xinit_args)))
          goto xserver_error;
        if (!(args = calloc(num_token + 4, sizeof(char *))))
          {
             free(abuf);
             goto xserver_error;
          }
        args[0] = (char *)entrance_config->command.xinit_path;
        token = strtok_r(abuf, " ", &saveptr);
        ++num_token;
        for(int i = 1; i < num_token; ++i)
          {
             if (token)
               args[i] = token;
             token = strtok_r(NULL, " ", &saveptr);
          }
        snprintf(vt, sizeof(vt), "vt%d", entrance_config->command.vtnr);
        args[num_token] = vt;
        num_token++;
        args[num_token] = display;
        num_token++;
        args[num_token] = NULL;
     }
   else
     {
        if (!(args = calloc(2, sizeof(char*))))
          goto xserver_error;
        args[0] = (char *)entrance_config->command.xinit_path;
        args[1] = NULL;
     }
   PT("Executing: %s %s vt%d %s",
      entrance_config->command.xinit_path,
      entrance_config->command.xinit_args,
      entrance_config->command.vtnr,
      display);
   // ideally close on success, otherwise proceeding PT is never outputted
   entrance_close_log();
   execv(args[0], args);
   if (abuf) free(abuf);
   free(args);
   PT("Failed to launch Xserver ...");
   return pid;

xserver_error:
   PT("Failed to launch Xserver ...");
   _exit(EXIT_FAILURE);
}

static Eina_Bool
_xserver_started(void *data, int type EINA_UNUSED, void *event EINA_UNUSED)
{
    const long id = *(int *)data;
    PT("xserver %ld started on %s", id, _xservers[id]->dname);
    setenv("DISPLAY", _xservers[id]->dname, 1);
    if(!entrance_auto_login_enabled())
        _xservers[id]->start(_xservers[id]->dname);
    return ECORE_CALLBACK_PASS_ON;
}

void
entrance_xservers_init(int count)
{
    _xservers = (Entrance_Xserver**) calloc(count, sizeof(Entrance_Xserver*));
    _xserver_count = count;
    PT("Allocated memory for %d X servers", _xserver_count);
}

int
entrance_xserver_start(int id, Entrance_X_Cb start, char *display)
{
   int pid;
   sigset_t newset;
   sigemptyset(&newset);

   /* ensure we have at least 1 X server allocated, legacy support*/
    if(!_xserver_count)
    {
        entrance_xservers_init(1);
        id = 0;
    }
   _xservers[id] = calloc(1, sizeof(Entrance_Xserver));
   _xservers[id]->id = id;
   _xservers[id]->dname = eina_stringshare_add(display);
   _xservers[id]->start = start;
   pid = _xserver_start(display);  /* Returns child X server PID */
   PT("X server process started with pid %d", pid);
   PT("xserver adding signal user handler");
   _xservers[id]->handler_start = ecore_event_handler_add(ECORE_EVENT_SIGNAL_USER,
                                                          _xserver_started,
                                                           &(_xservers[id]->id));
   return pid;
}

void
entrance_xserver_shutdown(int id)
{
   eina_stringshare_del(_xservers[id]->dname);
   ecore_event_handler_del(_xservers[id]->handler_start);
   free(_xservers[id]);
}

void
entrance_xservers_shutdown()
{
   free(_xservers);
}
