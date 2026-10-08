#include <xcb/xkb.h>

#include "entrance.h"

static char * _entrance_keyboard_atom_name_get(xcb_get_atom_name_cookie_t cookie);
static void _entrance_keyboard_connect();
static void _entrance_keyboard_disconnect();
static uint8_t _entrance_keyboard_indicator_mask_get(uint8_t i);

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
        name = (char *)malloc(len + 1);
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

    if (xcb_connection_has_error(_xcb_connection))
    {
        PT("xcb_connect failed, error %d", error->error_code);
        _xcb_connection = NULL;
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

void entrance_keyboard_init()
{
    _entrance_keyboard_connect();
    _entrance_keyboard_disconnect();
}
