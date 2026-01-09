#include "entrance.h"
#include <sys/wait.h>
#include <unistd.h>

typedef struct Entrance_Xserver_
{
   const char *dname;
   Entrance_X_Cb start;
} Entrance_Xserver;

Entrance_Xserver *_xserver;
Ecore_Event_Handler *_handler_start;

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
_xserver_start(void)
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
     return pid;

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
        args[num_token] = (char *)entrance_config->command.xdisplay;
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
      entrance_config->command.xdisplay);
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
_xserver_started(void *data EINA_UNUSED,
                 int type EINA_UNUSED,
                 void *event EINA_UNUSED)
{
   PT("xserver started");
   setenv("DISPLAY",_xserver->dname,1);
   if(!entrance_auto_login_enabled())
     _xserver->start(_xserver->dname);
   return ECORE_CALLBACK_PASS_ON;
}

int
entrance_xserver_init(Entrance_X_Cb start, const char *dname)
{
   int pid;
   sigset_t newset;
   sigemptyset(&newset);

   _xserver = calloc(1, sizeof(Entrance_Xserver));
   _xserver->dname = eina_stringshare_add(dname);
   _xserver->start = start;
   pid = _xserver_start();
   PT("xserver adding signal user handler");
   _handler_start = ecore_event_handler_add(ECORE_EVENT_SIGNAL_USER,
                                            _xserver_started,
                                            NULL);
   return pid;
}

void
entrance_xserver_shutdown(void)
{
   eina_stringshare_del(_xserver->dname);
   free(_xserver);
   ecore_event_handler_del(_handler_start);
}

