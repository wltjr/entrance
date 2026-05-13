#include <sys/wait.h>
#include <unistd.h>

#include "entrance.h"

typedef struct Entrance_Xserver_
{
    int vt;
    long id;
    const char *display;
    Entrance_X_Cb start;
    Ecore_Event_Handler *handler_start;
} Entrance_Xserver;

static int _xserver_start(const Entrance_Xserver *_xserver);
static Eina_Bool _xserver_started(void *data, int type EINA_UNUSED, void *event EINA_UNUSED);

static int _xserver_count = 0;
static Entrance_Xserver **_xservers;


static int
_xserver_start(const Entrance_Xserver *_xserver)
{
   int num_token = 0;
   char *abuf = NULL;
   char *buf = NULL;
   char **args = NULL;
   char *saveptr = NULL;
   char *token;
   char display[64] = {0};
   char seat[64] = {0};
   char vt[64] = {0};
   char xinit_path[PATH_MAX] = {0};
   pid_t pid;

   PT("Launching X server %ld", _xserver->id);
   pid = fork();
   if (pid)
     {
        PT("Parent process: X server %ld child pid %d", _xserver->id, pid);
        return pid;  /* Parent returns child's PID */
     }

   /* Child process (pid == 0): will exec X server */
   PT("Child process: executing X server %ld", _xserver->id);
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

   // prevent cast from 'const char *' to 'char *' drops const qualifier
   snprintf(display, sizeof(display), "%s", _xserver->display);
   snprintf(xinit_path, sizeof(xinit_path), "%s", entrance_config->command.xinit_path);

   if (num_token)
     {
        num_token += 6;

        if (!(abuf = strdup(entrance_config->command.xinit_args)))
          goto xserver_error;
        if (!(args = calloc(num_token, sizeof(char *))))
          {
             free(abuf);
             goto xserver_error;
          }
        args[0] = xinit_path;
        args[1] = display;
        args[2] = "-seat";
        snprintf(seat, sizeof(seat), "seat%ld", _xserver->id);
        args[3] = seat;
        snprintf(vt, sizeof(vt), "vt%d", _xserver->vt);
        args[4] = vt;
        token = strtok_r(abuf, " ", &saveptr);
        for(int i = 5; i < num_token; ++i)
          {
             if (token)
               args[i] = token;
             token = strtok_r(NULL, " ", &saveptr);
          }
        args[num_token - 1] = NULL;
     }
   else
     {
        if (!(args = calloc(2, sizeof(char*))))
          goto xserver_error;
        args[0] = xinit_path;
        args[1] = NULL;
     }
   PT("Executing: %s %s -seat seat%ld vt%d %s",
      entrance_config->command.xinit_path,
      _xserver->display,
      _xserver->id,
      _xserver->vt,
      entrance_config->command.xinit_args);
   // ideally close on success, otherwise proceeding PT is never outputted
   entrance_close_log();
   execv(args[0], args);
   if (abuf) free(abuf);
   free(args);
   PT("Failed to launch X server %ld ...", _xserver->id);
   return pid;

xserver_error:
   PT("Failed to launch X server %ld ...", _xserver->id);
   _exit(EXIT_FAILURE);
}

static Eina_Bool
_xserver_started(void *data, int type EINA_UNUSED, void *event EINA_UNUSED)
{
    const long id = *(int *)data;
    PT("X server %ld started on %s seat%ld vt%d", id, _xservers[id]->display, id, _xservers[id]->vt);
    setenv("DISPLAY", _xservers[id]->display, 1);
    if(!entrance_auto_login_enabled())
        _xservers[id]->start((int)id, _xservers[id]->display);
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
entrance_xserver_start(int id, Entrance_X_Cb start, const char *display, int vt)
{
   int pid;
   sigset_t newset;
   sigemptyset(&newset);

   _xservers[id] = calloc(1, sizeof(Entrance_Xserver));
   _xservers[id]->id = id;
   _xservers[id]->display = eina_stringshare_add(display);
   _xservers[id]->start = start;
   _xservers[id]->vt = vt;
   pid = _xserver_start(_xservers[id]);  /* Returns child X server PID */
   _xservers[id]->handler_start = ecore_event_handler_add(ECORE_EVENT_SIGNAL_USER,
                                                          _xserver_started,
                                                           &(_xservers[id]->id));
   PT("X server %d process started with pid %d", id, pid);
   return pid;
}

void
entrance_xserver_shutdown(int id)
{
   eina_stringshare_del(_xservers[id]->display);
   ecore_event_handler_del(_xservers[id]->handler_start);
   free(_xservers[id]);
}

void
entrance_xservers_shutdown()
{
   if(_xservers)
      free(_xservers);
   _xserver_count = 0;
}
