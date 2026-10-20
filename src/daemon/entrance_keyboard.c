#include <xcb/xkb.h>

#include "entrance.h"

static char * _entrance_keyboard_atom_name_get(xcb_get_atom_name_cookie_t cookie);
static void _entrance_keyboard_connect();
static void _entrance_keyboard_disconnect();
static void _entrance_keyboard_indicator_masks_set();
static void _entrance_keyboard_update();
static uint8_t _entrance_keyboard_indicator_mask_get(uint8_t i);

uint8_t _mask_capslock = 0;
uint8_t _mask_numlock = 0;
xcb_connection_t *_xcb_connection = NULL;

static char *
_entrance_keyboard_atom_name_get(xcb_get_atom_name_cookie_t cookie)
{
    int len;
    char *name = NULL;
    xcb_get_atom_name_reply_t *reply = NULL;
    xcb_generic_error_t *error = NULL;

    reply = xcb_get_atom_name_reply(_xcb_connection, cookie, &error);

    if (reply)
    {
        len = xcb_get_atom_name_name_length(reply) + 1;
        name = (char *)calloc(len + 1, sizeof(char));
        snprintf(name, len, "%s", xcb_get_atom_name_name(reply));

        free(reply);
    }
    else
        PT("failed to get atom name, error %d", error->error_code);

    return name;
}

static void
_entrance_keyboard_connect()
{
    xcb_xkb_use_extension_cookie_t cookie;
    xcb_generic_error_t *error = NULL;

    _xcb_connection = xcb_connect(NULL, NULL);

    if (!_xcb_connection)
    {
        PT("xcb_connect failed");
        return;
    }

    cookie = xcb_xkb_use_extension(_xcb_connection,
                                   XCB_XKB_MAJOR_VERSION,
                                   XCB_XKB_MINOR_VERSION);
    xcb_xkb_use_extension_reply(_xcb_connection, cookie, &error);

    if (error)
    {
        PT("xcb_connect failed, error %d", error->error_code);
        _xcb_connection = NULL;
        return;
    }
}

static void
_entrance_keyboard_disconnect()
{
    xcb_disconnect(_xcb_connection);
    _xcb_connection = NULL;
}

static uint8_t
_entrance_keyboard_indicator_mask_get(uint8_t i)
{
    xcb_xkb_get_indicator_map_cookie_t cookie;
    xcb_xkb_get_indicator_map_reply_t *reply = NULL;
    xcb_generic_error_t *error = NULL;
    uint8_t mask = 0;

    cookie = xcb_xkb_get_indicator_map(_xcb_connection, XCB_XKB_ID_USE_CORE_KBD, 1 << i);
    reply = xcb_xkb_get_indicator_map_reply(_xcb_connection, cookie, &error);

    if (reply)
    {
        xcb_xkb_indicator_map_t *map;

        map = xcb_xkb_get_indicator_map_maps(reply);
        mask = map->mods;

        free(reply);
    }
    else
        PT("xcb_connect failed, error %d", error->error_code);

    return mask;
}

static void 
_entrance_keyboard_indicator_masks_set()
{
    int indicators;
    const void *buffer;
    xcb_xkb_get_names_cookie_t cookie;
    xcb_xkb_get_names_reply_t *reply = NULL;
    xcb_xkb_get_names_value_list_t list;
    xcb_generic_error_t *error = NULL;

    cookie = xcb_xkb_get_names(_xcb_connection,
                               XCB_XKB_ID_USE_CORE_KBD,
                               XCB_XKB_NAME_DETAIL_INDICATOR_NAMES);
    reply = xcb_xkb_get_names_reply(_xcb_connection, cookie, &error);

    if (error)
    {
        PT("keyboard led map failed, error %d", error->error_code);
        return;
    }

    buffer = xcb_xkb_get_names_value_list(reply);
    xcb_xkb_get_names_value_list_unpack(buffer, reply->nTypes, reply->indicators,
                                        reply->virtualMods, reply->groupNames,
                                        reply->nKeys, reply->nKeyAliases,
                                        reply->nRadioGroups, reply->which, &list);

    indicators = xcb_xkb_get_names_value_list_indicator_names_length(reply, &list);
    for (int i = 0; i < indicators; i++)
    {
        char *name;

        name = _entrance_keyboard_atom_name_get(
                    xcb_get_atom_name(_xcb_connection, list.indicatorNames[i]));

        if (strcmp(name, "Num Lock") == 0)
            _mask_numlock = _entrance_keyboard_indicator_mask_get(i);
        else if (strcmp(name, "Caps Lock") == 0)
            _mask_capslock = _entrance_keyboard_indicator_mask_get(i);

        free(name);
    }

    free(reply);
}

static void
_entrance_keyboard_update()
{
    uint8_t mask_cur;
    uint8_t mask_full;
    xcb_void_cookie_t cookie;
    xcb_generic_error_t *error = NULL;

    mask_full = _mask_numlock | _mask_capslock;
    mask_cur  = (entrance_config->numlock  ? _mask_numlock  : 0) |
                (entrance_config->capslock ? _mask_capslock : 0);

    cookie = xcb_xkb_latch_lock_state(_xcb_connection,
                                      XCB_XKB_ID_USE_CORE_KBD,
                                      mask_full,
                                      mask_cur,
                                      1, 0, 0, 0, 0);

    error = xcb_request_check(_xcb_connection, cookie);
    if (error)
        PT("failed to update state, error %d", error->error_code);
}

void entrance_keyboard_init()
{
    _entrance_keyboard_connect();

    if(!_xcb_connection)
        return;

    // get and set masks
    _entrance_keyboard_indicator_masks_set();

    // enable/disable capslock/numlock
    _entrance_keyboard_update();

    _entrance_keyboard_disconnect();
}
