#ifndef ENTRANCE_H_
#define ENTRANCE_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#include <Eina.h>
#include <Eet.h>
#include <Ecore.h>
#include <Ecore_File.h>
#include <Efreet.h>

#include "entrance_session.h"
#ifdef HAVE_PAM
#include "entrance_pam.h"
#endif
#ifdef HAVE_LOGIND
#include "entrance_logind.h"
#endif
#include "../event/entrance_event.h"
#include "entrance_config.h"
#include "entrance_xserver.h"
#include "entrance_server.h"
#include "entrance_history.h"
#include "entrance_action.h"
#include "entrance_image.h"
#include "entrance_theme.h"

extern int _entrance_log;
extern int _entrance_client_log;

#define PT(f, x...)                          \
do                                           \
{                                            \
   EINA_LOG_DOM_INFO(_entrance_log, f, ##x); \
} while (0)

Eina_Bool entrance_auto_login_enabled();
void entrance_close_log();

/**
 * @brief Set client id sent from client received by server
 *
 * @param id the id of the client, to identify which client it is to server
 * @param pid client pid
 */
void entrance_client_pid_set(int id, pid_t pid);

#endif /* ENTRANCE_H_ */
