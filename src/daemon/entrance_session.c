#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <grp.h>
#ifndef HAVE_PAM
#include <shadow.h>
#include <crypt.h>
#endif

#include "entrance.h"

#define HAVE_SHADOW 1

typedef struct Entrance_Session_
{
    pid_t pid;
    int vt;
    char *login;
    char *mcookie;
    const char *display;
} Entrance_Session;

static Eina_List *_xsessions = NULL;
static int _entrance_session_sort(const Entrance_Xsession *a, const Entrance_Xsession *b);
static int _entrance_session_userid_set(const struct passwd *pwd);

static void _entrance_session_run(int id,
                                  struct passwd *pwd,
                                  const char *cmd,
                                  const char *cookie,
                                  Eina_Bool is_wayland);
static Eina_Bool _entrance_session_is_wayland(const char *session);

static void _entrance_session_desktops_scan_file(const char *path);
static void _entrance_session_desktops_scan(const char *dir);
static void _entrance_session_desktops_init(void);
static const char *_entrance_session_find_command(const char *path, const char *session);
static struct passwd *_entrance_session_session_open(int id);

static int _session_count = 0;
static Entrance_Session **_sessions;

static int
_entrance_session_cookie_add(const char *mcookie, const char *display,
                         const char *xauth_cmd, const char *auth_file)
{
    char buf[PATH_MAX];
    FILE *cmd;

    if (!xauth_cmd || !auth_file) return 1;
    snprintf(buf, sizeof(buf), "%s -f %s -q", xauth_cmd, auth_file);
    PT("write auth on display %s with file %s", display, auth_file);
    cmd = popen(buf, "w");
    if (!cmd)
      {
         PT("write auth fail !");
         return 1;
      }
    fprintf(cmd, "remove %s\n", display);
    fprintf(cmd, "add %s . %s\n", display, mcookie);
    fprintf(cmd, "exit\n");
    pclose(cmd);
    return 0;
}

static int
_entrance_session_userid_set(const struct passwd *pwd)
{
   if (!pwd)
     {
        PT("no passwd !");
        return 1;
     }
   if (initgroups(pwd->pw_name, pwd->pw_gid) != 0)
     {
        PT("can't init group");
        return 1;
     }
   if (setgid(pwd->pw_gid) != 0)
     {
        PT("can't set gid");
        return 1;
     }
   if (setuid(pwd->pw_uid) != 0)
     {
        PT("can't set uid");
        return 1;
     }
   return 0;
}

static void
_entrance_session_shell_set(struct passwd *pwd)
{
   PT("Detecting shell");
   if (pwd->pw_shell[0] == '\0')
     {
        const char *shell;
        static char default_shell[PATH_MAX];

        setusershell();
        shell = getusershell();
        if (shell)
          {
             snprintf(default_shell, sizeof(default_shell), "%s", shell);
             pwd->pw_shell = default_shell;
          }
        endusershell();
     }
}

#ifdef HAVE_PAM
static void
_entrance_session_pam_env_set(int id,
                              const struct passwd *pwd,
                              const char *cookie,
                              Eina_Bool is_wayland)
{
   PT("Setting PAM environment for session #%d", id);
   const char *term = NULL;
   char seat[64] = {0};
   char vtnr[64] = {0};

   snprintf(seat, sizeof(seat), "seat%d", id);
   term = getenv("TERM");
   snprintf(vtnr, sizeof(vtnr), "%d", id);

   if (term) entrance_pam_env_set("TERM", term);
   entrance_pam_env_set("HOME", pwd->pw_dir);
   entrance_pam_env_set("SHELL", pwd->pw_shell);
   entrance_pam_env_set("USER", pwd->pw_name);
   entrance_pam_env_set("LOGNAME", pwd->pw_name);
   entrance_pam_env_set("PATH", entrance_config->session_path);
   entrance_pam_env_set("DISPLAY", _sessions[id]->display);
   entrance_pam_env_set("MAIL=/var/mail/%s", pwd->pw_name);
   entrance_pam_env_set("XAUTHORITY", cookie);
   entrance_pam_env_set("XDG_SEAT", seat);
   entrance_pam_env_set("XDG_SESSION_CLASS", "user");
   entrance_pam_env_set("XDG_SESSION_TYPE", is_wayland ? "wayland" : "x11");
   entrance_pam_env_set("XDG_VTNR", vtnr);
}
#endif

static void
_entrance_session_run(int id,
                      struct passwd *pwd,
                      const char *cmd,
                      const char *cookie,
                      Eina_Bool is_wayland)
{
   char **env;
   pid_t pid;
   pid = fork();
   if (pid == 0)
     {
        char buf[PATH_MAX];

        PT("Session #%d Run (%s)", id, is_wayland ? "wayland" : "x11");
        
        /* Create new session - MUST be done before PAM  */
        if (setsid() < 0)
          {
             PT("Failed to create new session #%d", id);
             exit(1);
          }
        
#ifdef HAVE_PAM
        /* Open PAM session in child process */
        if (entrance_pam_open_session())
          {
             PT("Failed to open PAM session in child");
             exit(1);
          }

        _entrance_session_pam_env_set(id, pwd, cookie, is_wayland);

        /* Retrieve final PAM environment with our vars */
        env = entrance_pam_env_list_get();
#else
        int n = 0;
        char *term = getenv("TERM");
        env = (char **)malloc(15 * sizeof(char *));
        if (!env)
          {
            PT("Failed to allocate environment array");
            return;
          }
        if(term)
          {
            char *t = NULL;
            t = strndup(term,128);
            if(t)
              {
                 snprintf(buf, sizeof(buf), "TERM=%s", t);
                 env[n++]=strdup(buf);
                 free(t);
              }
          }
        snprintf(buf, sizeof(buf), "HOME=%s", pwd->pw_dir);
        env[n++]=strdup(buf);
        snprintf(buf, sizeof(buf), "SHELL=%s", pwd->pw_shell);
        env[n++]=strdup(buf);
        snprintf(buf, sizeof(buf), "USER=%s", pwd->pw_name);
        env[n++]=strdup(buf);
        snprintf(buf, sizeof(buf), "LOGNAME=%s", pwd->pw_name);
        env[n++]=strdup(buf);
        snprintf(buf, sizeof(buf), "PATH=%s", entrance_config->session_path);
        env[n++]=strdup(buf);
        snprintf(buf, sizeof(buf), "DISPLAY=%s", _sessions[id]->display);
        env[n++]=strdup(buf);
        snprintf(buf, sizeof(buf), "MAIL=/var/mail/%s", pwd->pw_name);
        env[n++]=strdup(buf);
        snprintf(buf, sizeof(buf), "XAUTHORITY=%s", cookie);
        env[n++]=strdup(buf);
        snprintf(buf, sizeof(buf), "XDG_SEAT=seat%d", id);
        env[n++]=strdup(buf);
        snprintf(buf, sizeof(buf), "XDG_SESSION_CLASS=user");
        env[n++]=strdup(buf);
        snprintf(buf, sizeof(buf), "XDG_VTNR=%d", _sessions[id]->vt);
        env[n++]=strdup(buf);
        snprintf(buf, sizeof(buf), "XDG_SESSION_TYPE=%s", is_wayland ? "wayland" : "x11");
        env[n++]=strdup(buf);
        snprintf(buf, sizeof(buf), "XDG_RUNTIME_DIR=/run/user/%d", pwd->pw_uid);
        env[n++]=strdup(buf);
        env[n++]=0;
#endif
        
        snprintf(buf, sizeof(buf),
                 "%s %s %s",
                 entrance_config->command.session_start,
                 _sessions[id]->display,
                 pwd->pw_name);
        if (-1 == system(buf))
          PT("Error on session start command");
        if(_entrance_session_userid_set(pwd)) return;
        _entrance_session_cookie_add(_sessions[id]->mcookie, _sessions[id]->display,
                                 entrance_config->command.xauth_path, cookie);
        if (chdir(pwd->pw_dir))
          {
             PT("change directory for user fail");
             return;
          }
        
        /* Clean up PAM in child before exec */
#ifdef HAVE_PAM
        entrance_pam_end();
#endif
        
        snprintf(buf, sizeof(buf), "%s/.entrance_session.log", pwd->pw_dir);
        if (-1 == remove(buf))
          PT("Error could not remove session log file");

        snprintf(buf, sizeof(buf), "%s %s > \"%s/.entrance_session.log\" 2>&1",
                 entrance_config->command.session_login, cmd, pwd->pw_dir);

        PT("Executing: %s --login -c %s ", pwd->pw_shell, buf);
        execle(pwd->pw_shell, pwd->pw_shell, "--login", "-c", buf, NULL, env);
        PT("The Xsessions are not launched :(");
     }
   else if (pid > 0)
     {
        PT("%s: session %d pid %d", PACKAGE, 0, pid);
        _sessions[id]->pid = pid;
     }
   else
     {
        PT("Failed to start session");
     }
}

void
entrance_session_close(const Eina_Bool opened)
{
#ifdef HAVE_PAM
   entrance_pam_close_session(opened);
   entrance_pam_end();
   entrance_pam_shutdown();
#endif
}

void
entrance_sessions_init(int count)
{
    _sessions = (Entrance_Session**) calloc(count, sizeof(Entrance_Session*));
    _session_count = count;
    PT("Allocated memory for %d sessions", _session_count);

    /* do this once for all sessions not per session, shared xsessions list */
    _entrance_session_desktops_init();
}

pid_t
entrance_session_pid_get(int id)
{
   return _sessions[id]->pid;
}

void
entrance_session_cookie(int id)
{
    uint16_t word;
    uint8_t hi;
    uint8_t lo;
    int len;
    char *xauth_file_ptr;
    char buf[PATH_MAX];
    char xauth_file[PATH_MAX] = {0};
    static const char *dig = "0123456789abcdef";

   _sessions[id]->mcookie = calloc(33, sizeof(char));
   if (!_sessions[id]->mcookie)
     {
       PT("Failed to allocate memory for cookie");
       return;
     }
   _sessions[id]->mcookie[0] = 'a';

   long rand = 0;
   size_t read = 0;
   struct timespec time;
   FILE *fp;
   fp = fopen("/dev/urandom", "r");
   for (int i = 0; i < 32; i += 4)
     {
       read = 0;
       if (fp)
           read = fread(&rand,sizeof(rand),1,fp);
       if (!read)  /* Use clock fallback ONLY if urandom read failed */
         {
           clock_gettime(CLOCK_REALTIME, &time);
           rand = time.tv_nsec;
         }
        word = rand & 0xffff;
        lo = word & 0xff;
        hi = word >> 8;
        _sessions[id]->mcookie[i] = dig[lo & 0x0f];
        _sessions[id]->mcookie[i+1] = dig[lo >> 4];
        _sessions[id]->mcookie[i+2] = dig[hi & 0x0f];
        _sessions[id]->mcookie[i+3] = dig[hi >> 4];
     }
   if(fp)
     fclose(fp);

    /* add the session/x server/seat id to the xauth filename   */
    xauth_file_ptr = xauth_file;
    snprintf(buf, sizeof(buf), "%s", entrance_config->command.xauth_file);  // copy for guarantee null-terminated for strlen
    len = strlen(buf) - 4; // - .auth less 1, assumed
    snprintf(xauth_file, len, "%s", entrance_config->command.xauth_file);
    xauth_file_ptr += len - 1;
    len = PATH_MAX - 10; // padding for snprintf usage file_length-00.auth
    snprintf(xauth_file_ptr, len, "-%d.auth", id);

   fp = fopen(xauth_file,"a+");
   if(!fp)
     {
       PT("unable to create xauth %s",xauth_file);
       return;
     }
   fclose(fp);

   xauth_file[sizeof(buf) - 1] = '\0'; // guarantee null-terminated for strlen
   snprintf(buf, sizeof(buf) - strlen(xauth_file), "XAUTHORITY=%s", xauth_file);
   /* Note: putenv takes ownership of string, don't free */
   putenv(strdup(buf));
   _entrance_session_cookie_add(_sessions[id]->mcookie,
                                _sessions[id]->display,
                                entrance_config->command.xauth_path,
                                xauth_file);
}

void
entrance_session_start(int id, const char *display, int vt)
{
    _sessions[id] = (Entrance_Session*) calloc(1, sizeof(Entrance_Session));
    _sessions[id]->display = display;
    _sessions[id]->vt = vt;
    _sessions[id]->pid = 0;
}

void
entrance_session_shutdown(int id)
{
    if(_sessions[id])
    {
        if(_sessions[id]->mcookie)
            free(_sessions[id]->mcookie);
        free(_sessions[id]);
    }
}

void
entrance_sessions_shutdown()
{
    Entrance_Xsession *xsession;

    EINA_LIST_FREE(_xsessions, xsession)
    {
        eina_stringshare_del(xsession->name);
        eina_stringshare_del(xsession->icon);
        if (xsession->command) eina_stringshare_del(xsession->command);
        free(xsession);
    }

    if(_sessions)
        free(_sessions);
    _session_count = 0;
}

Eina_Bool
entrance_session_authenticate(int id, const char *login, const char *passwd)
{
   Eina_Bool auth;
   _sessions[id]->login = strdup(login);
   if (!_sessions[id]->login)
     return EINA_FALSE;
#ifdef HAVE_PAM
   char tty_name[16];

    snprintf(tty_name, sizeof(tty_name), "tty%u", _sessions[id]->vt);
    entrance_pam_init(PACKAGE, tty_name, login);
    auth = !!(!entrance_pam_passwd_set(passwd) &&
              !entrance_pam_authenticate());
#else
   char *enc, *v;
   struct passwd pwd_buf;
   struct passwd *pwd = NULL;
   char buf[4096];
   int result;

   result = getpwnam_r(login, &pwd_buf, buf, sizeof(buf), &pwd);
   if (result != 0 || !pwd)
     return EINA_FALSE;
#ifdef HAVE_SHADOW
   struct spwd *spd;
   spd = getspnam(pwd->pw_name);
   endspent();
   if(spd)
     v = spd->sp_pwdp;
   else
#endif
     v = pwd->pw_passwd;
   if(!v || *v == '\0')
     return EINA_TRUE;
   enc = crypt(passwd, v);
   auth = !strcmp(enc, v);
#endif
   eina_stringshare_del(passwd);
   return auth;
}

static struct passwd *
#ifdef HAVE_PAM
_entrance_session_session_open(int id EINA_UNUSED)
#else
_entrance_session_session_open(int id)
#endif
{
   static struct passwd pwd_buf;
   static char buf[4096];
   struct passwd *pwd = NULL;
   const char *user;
   int result;

#ifdef HAVE_PAM
   /* Just get user info - session will be opened in child process (spawny pattern) */
   user = entrance_pam_item_get(ENTRANCE_PAM_ITEM_USER);
   if (user)
     {
       result = getpwnam_r(user, &pwd_buf, buf, sizeof(buf), &pwd);
       if (result == 0 && pwd)
         return pwd;
     }
   return NULL;
#else
   user = entrance_session_login_get(id);
   if (user)
     {
       result = getpwnam_r(user, &pwd_buf, buf, sizeof(buf), &pwd);
       if (result == 0 && pwd)
         return pwd;
     }
   return NULL;
#endif
}

Eina_Bool
entrance_session_login(int id, const char *session, Eina_Bool history)
{
   struct passwd *pwd;
   const char *cmd;
   char buf[PATH_MAX];

   pwd = _entrance_session_session_open(id);
   endpwent();
   if (!pwd) return ECORE_CALLBACK_CANCEL;
   
   /* Detect shell in parent - env setup happens in child after session */
   _entrance_session_shell_set(pwd);
   
   snprintf(buf, sizeof(buf), "%s/.Xauthority", pwd->pw_dir);
   if (history) entrance_history_push(pwd->pw_name, session);
   cmd = _entrance_session_find_command(pwd->pw_dir, session);
   if (!cmd)
     {
        PT("Error !!! No command to launch, can't open a session :'(");
        return EINA_FALSE;
     }
   if(!_sessions[id]->login && entrance_auto_login_enabled())
     _sessions[id]->login = strdup(entrance_config->userlogin);
   PT("launching session %s for user %s", cmd, _sessions[id]->login);
   
   Eina_Bool is_wayland = _entrance_session_is_wayland(session);
   _entrance_session_run(id, pwd, cmd, buf, is_wayland);
   free(_sessions[id]->login);
   _sessions[id]->login = NULL;
   return EINA_TRUE;
}

static Eina_Bool
_entrance_session_is_wayland(const char *session)
{
   const Entrance_Xsession *xsession = NULL;
   Eina_List *l;
   
   if (!session) return EINA_FALSE;
   
   EINA_LIST_FOREACH(_xsessions, l, xsession)
     {
        if (!strcmp(xsession->name, session))
          return xsession->is_wayland;
     }
   return EINA_FALSE;
}

static const char *
_entrance_session_find_command(const char *path, const char *session)
{
   const Entrance_Xsession *xsession = NULL;
   char buf[PATH_MAX];
   if (session)
     {
        Eina_List *l;

        EINA_LIST_FOREACH(_xsessions, l, xsession)
          {
             if (!strcmp(xsession->name, session) && xsession->command)
               return xsession->command;
          }
     }
   snprintf(buf, sizeof(buf), "%s/%s",
            path, ".xinitrc");
   if (ecore_file_can_exec(buf))
     {
        if (xsession)
          snprintf(buf, sizeof(buf), "%s/%s %s",
                   path, ".xinitrc", xsession->command);
        return eina_stringshare_add(buf);
     }
   snprintf(buf, sizeof(buf), "%s/%s",
            path, ".Xsession");
   if (ecore_file_can_exec(buf))
     {
        if (xsession)
          snprintf(buf, sizeof(buf), "%s/%s %s",
                   path, ".Xsession", xsession->command);
        return eina_stringshare_add(buf);
     }
   if (ecore_file_exists("/etc/X11/xinit/xinitrc"))
     {
        if (xsession)
          {
             snprintf(buf, sizeof(buf), "sh /etc/X11/xinit/xinitrc %s",
                      xsession->command);
             return eina_stringshare_add(buf);
          }
        return eina_stringshare_add("sh /etc/X11/xinit/xinitrc");
     }
   return NULL;
}

char *
entrance_session_login_get(int id)
{
   return _sessions[id]->login;
}

Eina_List *
entrance_session_list_get(void)
{
   return _xsessions;
}

static void
_entrance_session_desktops_init(void)
{
   efreet_desktop_type_alias(EFREET_DESKTOP_TYPE_APPLICATION, "XSession");
   _entrance_session_desktops_scan("/etc/X11/Sessions");
   _entrance_session_desktops_scan("/usr/share/xsessions");
   _entrance_session_desktops_scan("/usr/share/wayland-sessions");
}

static void
_entrance_session_desktops_scan(const char *dir)
{
   if (ecore_file_is_dir(dir))
     {
        Eina_List *files;
        char *filename;
        char path[PATH_MAX];

        files = ecore_file_ls(dir);
        EINA_LIST_FREE(files, filename)
          {
             snprintf(path, sizeof(path), "%s/%s", dir, filename);
             _entrance_session_desktops_scan_file(path);
             free(filename);
          }
     }
}

static void
_entrance_session_desktops_scan_file(const char *path)
{
   Efreet_Desktop *desktop;
   Eina_List *commands;
   Eina_List *l;
   Entrance_Xsession *xsession;
   char *command = NULL;
   Eina_Bool is_wayland = EINA_FALSE;

   desktop = efreet_desktop_get(path);
   if (!desktop) return;
   EINA_LIST_FOREACH(_xsessions, l, xsession)
     {
        if (!strcmp(xsession->name, desktop->name))
          {
             efreet_desktop_free(desktop);
             return;
          }
     }

   /* Detect if this is a Wayland session */
   if (strstr(path, "wayland-sessions"))
     is_wayland = EINA_TRUE;

   commands = efreet_desktop_command_local_get(desktop, NULL);
   if (commands)
     command = eina_list_data_get(commands);
   if (command && desktop->name)
     {
        PT("Adding %s as %s session", desktop->name, is_wayland ? "wayland" : "x11");
        xsession= calloc(1, sizeof(Entrance_Xsession));
        if (!xsession)
          return;
        xsession->command = eina_stringshare_add(command);
        xsession->name = eina_stringshare_add(desktop->name);
        xsession->is_wayland = is_wayland;
        if (desktop->icon && strcmp(desktop->icon,""))
          xsession->icon = eina_stringshare_add(desktop->icon);
        _xsessions = eina_list_sorted_insert(_xsessions,
            (Eina_Compare_Cb)_entrance_session_sort,
            xsession);
     }
   EINA_LIST_FREE(commands, command)
     free(command);
   efreet_desktop_free(desktop);
}

static int
_entrance_session_sort(const Entrance_Xsession *a, const Entrance_Xsession *b)
{
    return strcasecmp(a->name, b->name);
}
