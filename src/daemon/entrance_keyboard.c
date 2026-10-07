#include <xcb/xkb.h>

#include "entrance.h"

static void _entrance_keyboard_connect();

xcb_connection_t *_xcb_connection = NULL;

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

void entrance_keyboard_init()
{
    _entrance_keyboard_connect();
}
