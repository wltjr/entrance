#include "entrance.h"
#include <ctype.h>

typedef void (*Entrance_Action_Cb)(void *data);

static void _entrance_action_shutdown(void *data);
static void _entrance_action_reboot(void *data);
static void _entrance_action_suspend(void *data);
static Eina_Bool _entrance_action_exe_event_del_cb(void *data, int type, void *event);

static Eina_List *_entrance_actions = NULL;

typedef struct Entrance_Action_Data__
{
   unsigned char id;
   const char *label;
   const char *icon;
   Entrance_Action_Cb func;
   void *data;
} Entrance_Action_Data;

static Ecore_Exe *_action_exe = NULL;

static Entrance_Action_Data *
_entrance_action_add(const char *label,
                     Entrance_Action_Cb func,
                     void *data)
{
   Entrance_Action_Data *ead;
   ead = calloc(1, sizeof(Entrance_Action_Data));
   if (!ead)
     return NULL;
   ead->label = eina_stringshare_add(label);
   ead->func = func;
   ead->data = data;
   ead->id = (unsigned char)eina_list_count(_entrance_actions);
   char icon[128];
   snprintf(icon,128,"system-%s",label);
   /* Lowercase first char of label (after "system-" prefix) */
   if (icon[7] != '\0')
     icon[7] = (char)tolower(icon[7]);
   ead->icon = eina_stringshare_add(icon);
   ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
                           _entrance_action_exe_event_del_cb, NULL);
   return ead;
}


void
entrance_action_init(void)
{
   _entrance_actions = eina_list_append(_entrance_actions,
      _entrance_action_add("Shutdown", _entrance_action_shutdown, NULL));
   _entrance_actions = eina_list_append(_entrance_actions,
      _entrance_action_add("Reboot", _entrance_action_reboot, NULL));
   _entrance_actions = eina_list_append(_entrance_actions,
      _entrance_action_add("Suspend", _entrance_action_suspend, NULL));
}

Eina_List *
entrance_action_get(void)
{
   const Entrance_Action_Data *ead;
   Entrance_Action *ea;
   Eina_List *l;
   Eina_List *ret = NULL;

   EINA_LIST_FOREACH(_entrance_actions, l, ead)
     {
        ea = calloc(1, sizeof(Entrance_Action));
        if (!ea)
          continue;
        ea->label = eina_stringshare_add(ead->label);
        ea->id = ead->id;
        ea->icon = eina_stringshare_add(ead->icon);
        ret = eina_list_append(ret, ea);
     }
   return ret;
}

void
entrance_action_shutdown(void)
{
   Entrance_Action_Data *ead;
   EINA_LIST_FREE(_entrance_actions, ead)
     {
        eina_stringshare_del(ead->label);
        eina_stringshare_del(ead->icon);
        free(ead);
     }
}


void
entrance_action_run(int action)
{
   Entrance_Action_Data *ead;

   ead = eina_list_nth(_entrance_actions, action);
   if (ead)
     ead->func(ead->data);
}

static void
_entrance_action_suspend(void *data EINA_UNUSED)
{
   PT("Suspend");
   _action_exe = NULL;
   ecore_exe_run(entrance_config->command.suspend, NULL);
}

static void
_entrance_action_shutdown(void *data EINA_UNUSED)
{
   PT("Shutdown");
   _action_exe = ecore_exe_run(entrance_config->command.shutdown, NULL);
}

static void
_entrance_action_reboot(void *data EINA_UNUSED)
{
   PT("Reboot");
   _action_exe = ecore_exe_run(entrance_config->command.reboot, NULL);
}

static Eina_Bool
_entrance_action_exe_event_del_cb(void *data EINA_UNUSED,
                                  int type EINA_UNUSED,
                                  void *event)
{
   const Ecore_Exe_Event_Del *ev;
   Eina_Bool ret = ECORE_CALLBACK_PASS_ON;
   ev = event;
   if (!ev->exe) return ret;
   if (ev->exe == _action_exe)
     {
        PT("action quit requested by user");
        ecore_main_loop_quit();
        ret = ECORE_CALLBACK_DONE;
     }
   return ret;
}