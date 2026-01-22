#ifndef ENTRANCE_H_
#define ENTRANCE_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef ENABLE_NLS
# include <libintl.h>
# define _(x) gettext(x)
#else
# define _(x) (x)
#endif

#include <stdio.h>

#include <Elementary.h>

#include "../event/entrance_event.h"
#include "entrance_gui.h"
#include "entrance_fill.h"
#include "entrance_connect.h"
#include "entrance_client.h"
#include "entrance_gui.h"
#include "entrance_login.h"
#include "entrance_system.h"

#define PT(f, x...)                                                        \
do                                                                         \
{                                                                          \
   printf(__FILE__":%d "f"\n", __LINE__, ##x); \
   fflush(stdout);                               \
} while (0)

void entrance_monitor_server_pid(pid_t pid);

#endif /* ENTRANCE_H_ */
