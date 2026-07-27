# entrance - An EFL based display manager

[![License](http://img.shields.io/badge/license-GPLv3-blue.svg?colorB=9977bb&style=plastic)](https://github.com/wltjr/entrance/blob/master/LICENSE)
[![Build Status](https://github.com/wltjr/entrance/actions/workflows/docker_build.yml/badge.svg)](https://github.com/wltjr/entrance/actions)
[![Code Quality](https://sonarcloud.io/api/project_badges/measure?project=wltjr_entrance&metric=alert_status)](https://sonarcloud.io/dashboard?id=wltjr_entrance)

This is a fork and current development version.
It is ALIVE! IT WORKS! (for me™).

This version of entrance has worked in some form or another since 2017. Many
incorrectly assume this version is the same as past versions, when many have
never run or seen this version, please see [History](#history).

[![A video of Entrance 3.0.0 Beta 1](https://github.com/user-attachments/assets/0dbf2da2-b220-4427-b0aa-b42f4f83d3e5)](https://github.com/user-attachments/assets/0dbf2da2-b220-4427-b0aa-b42f4f83d3e5)

[![A screenshot of Entrance 3.0.0 Beta 1](https://github.com/user-attachments/assets/d3b5f451-8b01-4136-af9f-deaee28d54db)](https://github.com/user-attachments/assets/d3b5f451-8b01-4136-af9f-deaee28d54db)

![A screenshot of Entrance](https://user-images.githubusercontent.com/12835340/31921581-1c2f0d7c-b83e-11e7-8d90-1dac94ae8e5c.jpg)

## About

Entrance is a Unix Display/Login Manager written in Enlightenment Foundation
Libraries (EFL). Entrance allows a user to choose a X WM/Desktop
session to launch upon successful login. Entrance is alive and working
again for logging into X sessions and initial support for Wayland sessions!

The project has been resurrected from the dead to live on once again...

## To-do

Current todo list, this maybe revised for more accuracy as project direction
becomes more well defined.

### Primary

- [x] User settings icon/bg/session control via users.conf
- [ ] Settings UI
  - [ ] for entrance.conf
  - [ ] for user icon/bg/session users.conf
- [ ] refactor
  - [ ] Uncalled code, identify never fired code paths
  - [+] User history and pretty much all of history
  - [+] Look over user icon/background
  - [ ] Split desktop and session code in entrance_sessions
  - [ ] Name uniformity on user/login/conf functions, vars etc.
- [ ] max tries, need to uncomment code and check it works with theme/UI?
- [-] [logind integration](#logindelogind-aka-systemd)
  - [x] Multi-seat awareness (xserver/client per seat on start)
    - [ ] Test > 1 seat
    - [ ] Seat detection and management (add/remove seats)
  - [-] VT number auto-detection (dynamic config value + seat id #)
- [-] [wayland support](#wayland-display-sessions)
  - [x] Basic detection and XDG_SESSION_TYPE=wayland set
  - [x] Session type (X11/Wayland) support
  - [ ] Disable X11 specific env and software (xserver, xauth, etc)
  - [ ] Full wayland compositor (entrance client)
  - [ ] Test wayland sessions
- [ ] Theme for cursor
- [ ] Virtual keyboard support
- [-] Write tests for uncovered code (may require test binaries)

### Housekeeping

- [-] log output
  - [-] Standardize seat, #, port, etc.
  - [ ] Ensure client/seat # in client/server communication log (passed in callback data)
- [-] clean up codebase
  - [-] Refactor imports system -> libs -> local
  - [-] Comment code (add comments to headers)

## Build

Entrance presently uses meson build system.

### Build using meson and ninja

```sh
prefix=/usr/share
meson \
    --prefix "${prefix}" \
    --bindir "${prefix}/bin" \
    --sbindir "${prefix}/sbin" \
    --datadir "${prefix}/share" \
    --sysconfdir "/etc" \
    -Dpam=true \
    -Dlogind=true \
    . build
ninja -C build install
```

Entrance provides it's own pam.d files for direct use which can be modified
as necessary for operating system integration; using system pam.d files instead
of Entrance specific.

```sh
cp data/entrance.pam.d /etc/pam.d/entrance
cp data/entrance-autologin.pam.d /etc/pam.d/entrance-autologin
```

The systemd service file is installed on systemd based systems. It should be
usable and/or correct. Please, open an issue if you experience any problems,
as Entrance is not developed nor run in a systemd based development environment.

## Configuration

Most things can be configured in entrance.conf at `/etc/entrance/entrance.conf`.
Some settings may not work. Please file issues for anything that is not
configurable or does not work.

### User Configuration

Entrance user configuration allows for Entrance to be customized per user, with
icons/images and/or backgrounds set per user in `/etc/entrance/users.conf`.

### Background and User Icon Configuration

Entrance features the ability to configure both background and user icon images,
set per user, random, via system, or theme default. By default, user icons are
selected randomly, and the background is the theme default; the current default
Enlightenment desktop background.

For Background image selection, the order is as follows,

1. user specified background via users.conf (image file can reside anywhere);
2. random if enabled in configuration, overrides system if set;
3. system specified background via entrance.conf;
4. default background from theme.

For User Icon image selection, the order is as follows,

1. user specified icon via users.conf (image file can reside anywhere);
2. random from theme.

#### Random Background

Entrance might be the only display/login manager that has native support for
random background images, both user and system levels. Each user can add to the
available pool of system backgrounds, or have their own, if no system
backgrounds are available. Backgrounds are set upon user selection and are
randomized with each selection.

#### Random User Icon

Entrance supports random user icons/avatars by default with images provided via
the theme, user, and system levels. Each user can add to the available pool of
system and theme icon images, or have their own, if no system icons are
available; theme icons are always part of the pool. Icons are set upon start
and are not randomized with each selection, only once per login session.

##### System Random Background and User Icon Images

Entrance looks for system wide random backgrounds in
`PACKAGE_DATA_DIR/backgrounds/` and random icons in
`PACKAGE_DATA_DIR/icons/` where `PACKAGE_DATA_DIR` is typically
`/usr/share/entrance`. These can be a directory or a symlink but subdirectories
will not be traversed, just the topmost level. These directories are only used
for random background pools are are not required for single background set
in `entrance.conf` or `users.conf`.

##### User Random Background and User Icon Images

Entrance looks for  user specific random backgrounds in
`$HOME/.config/entrance/backgrounds/` and random icons in
`$HOME/.config/entrance/icons/` where `$HOME` is typically the
user's home directory, typically `/home/username`. These can be a directory or a
symlink but subdirectories will not be traversed, just the topmost level. These
directories are only used for random icon pools are are not required for single
icon set in `users.conf`.

## Usage

In order to start entrance, you need a system init script or systemd (untested).
This may differ based on your operating system. Entrance does not
provide an init script at this time, it may not run or work correctly if started
directly. Entrance should be invoked via init script or systemd service.
There is a provided systemd service file for entrance. It is unknown if
this works or not.

### Entrance User

Entrance presently starts as root, and then, uses setuid to run entrance_client
under the user nobody. This was done via sudo and su for a bit before
switching to setuid. Entrance also used to run under the "entrance" user but
was changed in this [past commit](https://git.enlightenment.org/misc/entrance.git/commit/?id=866fdf557acbfbf1f2404da9c3799020375c16d2).
Inherited design from the 2nd generation.

It may be changed such that entrance runs under its own user, and does
not setuid or run under root. This will require creation of a user
account and adding permissions for the user for video, etc. It may be
possible to accomplish this now, starting entrance under a user say
"entrance" by modifying the untested entrance.conf `start_user`
option:

```c
value "start_user" string: "entrance";
```

You will likely also need to create a directory for entrance, and ensure
it has proper permissions. Since entrance will not be running under root,
entrance will not be able to correct this, despite having code for such.
Expect to see some errors in log file otherwise, along with non-functional
entrance. If started as root, entrance will create directories with proper
permissions as needed.

### Multiple Displays

Entrance supports multiple displays, with the login form residing on the
primary display. In some cases this does not work correctly. If using X,
you may need to set the following settings in your monitor sections.

```c
Section "Monitor"
    Identifier  "A"
    Option      "position"  "0 0"
EndSection

Section "Monitor"
    Identifier  "B"
    Option      "LeftOf"    "A"
EndSection

Section "Monitor"
    Identifier  "C"
    Option      "RightOf"   "A"
EndSection
```

Without such, only the background will appear. The login form will not
appear along with the rest of the UI, and the cursor will be an X. This
likely due to bugs in entrance code. The work around is the previously
mentioned monitors section.

## logind/elogind aka systemd

Initial logind/elogind code has been added to entrance thanks to
[@Javed](https://github.com/fourty5), and has been further extended
to detecting seats via logind/elogind on start and launching a xserver/client
per seat from a single entrance server. There is no support for seat events,
adding/removing seats, or related actions, which requires additional coding.

This is untested code, but should work in theory. Testing requires access to a
multi-seat system, if such systems exist... Or devise a way to test multi-seat
in CI. Any testing of Entrance multi-seat code and/or functionality is welcomed!

## wayland display sessions

Once again thanks to [@Javed](https://github.com/fourty5), entrance now
has initial support for launching Wayland sessions! This is not as heavily
tested, and is not being presently tested in CI. Please report any issues with
launching Wayland sessions, preferable, Enlightenment under Wayland, when the
mesa issues are resolved.

It is not known how well this will work to launch other Desktop environments,
but testing is welcomed!

## Development Discouraged

The entire EFL community has repeatedly shunned entrance for years.
There has been talk of making Enlightenment a login manager as many under
Wayland have no choice to do, such as Gnome and GDM, soon KDE and SDDM. There
has never been any efforts, and you are reduced to starting Enlightenment with
some other DM like Light-DM, SDDM, etc, or startx. However, this is reliant on
one person as so much of EFL and Enlightenment has been for decades, and the
developer community has never grown.

Maybe someday Enlightenment can log into itself, but in the years since 2017 to
2026, and in foreseeable future in 2026, the way to log into Enlightenment
Desktop is?

Sadly, while EFL has much potential, its toxic community has shunned developers
for years. All efforts with entrance have been frowned upon and discouraged
since 2017, and development has continued, despite it all! It took almost
10 years to get [another's](https://github.com/thewaiter)
improved [efforts](https://github.com/wltjr/clipboard) into Enlightenment,
and
[re-written](https://github.com/Enlightenment/enlightenment/tree/master/src/modules/clipboard) upon addition.

If entrance development will continue remains to be seen, there is new interest
in entrance development, but like all other EFL app efforts, the future is
unknown. Hopefully, this version of entrance will not face abandonment as past
versions, and most EFL applications.

The best tech does not always win, it can rise or fall based on communities!

## History

Entrance is a long story. There have been 2 different code bases and
projects both using the name entrance.

### 3rd Generation

This project is the 3rd generation, a fork of the 2nd Generation code
base, with a lot of fixes, and removal of incomplete and/or broken code. Rather
than fix as is, we are looking to replace functionality with novel code and
solutions providing similar functionality.

This generation is currently in development and is usable. Please open an
[issue](https://github.com/wltjr/entrance/issues) for any problems encountered.

### 2nd Generation

Sometime later another came along, [Michael Bouchaud](https://github.com/eyoz)/@eyoz
who renamed his project elsa to Entrance, which is where the current code base
came from. It is not known if this was ever completed or worked, it did not
function correctly, if it was even usable to log in at all.

The broken, incomplete, unmaintained 2nd Generation Entrance resides in Enlightenment's
[entrance git repository](https://git.enlightenment.org/old/entrance).
A branch may be added to this repository for historical purposes.

### 1st Generation

There was a project long ago that worked, and went
[MIA](https://web.archive.org/web/20161112163301/http://xcomputerman.com/pages/entrance.html).
Copies of the sources for some releases are in some distro repositories. A copy
has  been obtained via a PCLinuxOS src rpm. Ideally, it would be great to get a
copy of the old entrance repo to add to this one for historical purposes. If you
have a copy of the old Entrance repository, please  open an issue and provide a
link. That would be greatly appreciated!
