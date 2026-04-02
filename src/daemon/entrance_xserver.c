#include "entrance.h"
#include <sys/wait.h>
#include <unistd.h>

typedef struct Entrance_Xserver_
{
    int vt;
    long id;
    const char *display;
    Entrance_X_Cb start;
    Ecore_Event_Handler *handler_start;
} Entrance_Xserver;

static int _xserver_start(Entrance_Xserver *_xserver);
static Eina_Bool _xserver_started(void *data, int type EINA_UNUSED, void *event EINA_UNUSED);

int _xserver_count = 0;
Entrance_Xserver **_xservers;


static int
_xserver_start(Entrance_Xserver *_xserver)
{
   char *abuf = NULL;
   char *buf = NULL;
   char **args = NULL;
   char *saveptr = NULL;
   char vt[128] = {0};
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
        snprintf(vt, sizeof(vt), "vt%d", _xserver->vt);
        args[num_token] = vt;
        num_token++;
        args[num_token] = (char *)_xserver->display;
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
      _xserver->vt,
      _xserver->display);
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
    PT("X server %ld started on vt%d %s", id, _xservers[id]->vt, _xservers[id]->display);
    setenv("DISPLAY", _xservers[id]->display, 1);
    if(!entrance_auto_login_enabled())
        _xservers[id]->start(_xservers[id]->display);
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
entrance_xserver_start(int id, Entrance_X_Cb start, char *display, int vt)
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
   _xservers[id]->display = eina_stringshare_add(display);
   _xservers[id]->start = start;
   _xservers[id]->vt = vt;
   pid = _xserver_start(_xservers[id]);  /* Returns child X server PID */
   PT("X server %d process started with pid %d", id, pid);
   PT("X server %d adding signal user handler", id);
   _xservers[id]->handler_start = ecore_event_handler_add(ECORE_EVENT_SIGNAL_USER,
                                                          _xserver_started,
                                                           &(_xservers[id]->id));
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
   free(_xservers);
}
