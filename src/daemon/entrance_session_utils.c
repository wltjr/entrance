#include <grp.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#ifndef HAVE_PAM
#include <crypt.h>
#include <shadow.h>
#endif

#include "entrance.h"

int
entrance_session_userid_set(const struct passwd *pwd)
{
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

void
entrance_session_shell_set(struct passwd *pwd)
{
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
    PT("User shell %s", pwd->pw_shell);
}
