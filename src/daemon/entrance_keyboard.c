#include <xcb/xkb.h>

#include "entrance.h"

static char * _entrance_keyboard_atom_name_get(xcb_get_atom_name_cookie_t cookie);
static void _entrance_keyboard_connect();
static void _entrance_keyboard_disconnect();

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

void entrance_keyboard_init()
{
    _entrance_keyboard_connect();
    _entrance_keyboard_disconnect();
}
