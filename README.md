# entrance - An EFL based display manager
[![License](http://img.shields.io/badge/license-GPLv3-blue.svg?colorB=9977bb&style=plastic)](https://github.com/wltjr/entrance/blob/master/LICENSE)
![Build Status](https://github.com/wltjr/entrance/actions/workflows/docker_build.yml/badge.svg)
[![Code Quality](https://sonarcloud.io/api/project_badges/measure?project=wltjr_entrance&metric=alert_status)](https://sonarcloud.io/dashboard?id=wltjr_entrance)

This is a fork and current development version.
It is ALIVE! IT WORKS! (for me™).

This version of entrance has worked in some form or another since 2017. The
entire EFL community has repeatedly shunned entrance for years. Many incorrectly
assume this version is the same as past versions, when many have never run or
seen this version.

There has been talk of making Enlightenment a login manager as many under
Wayland have no choice to do, such as Gnome and GDM, soon KDE and SDDM. There
has never been any efforts, and you are reduced to starting Enlightment with
some other DM like Light-DM, SDDM, etc, or startx. However, this is reliant on
one person as so much of EFL and Enlightement has been for decades, and the
developer community has never grown.

Maybe someday Enlightenment can log into itself, but in the years since 2017 to
2026, and in forseeable future in 2026, the way to log into Enlightenment
Desktop is?

Sadly, while EFL has much potential, its toxic community has shunned developers
for years. All efforts with entrance have been frowned upon and discouraged
since 2017, and development has continued, despite it all! It took almost
10 years to get [another's](https://github.com/thewaiter)
improved [efforts](https://github.com/wltjr/clipboard) into Enlightement,
and
[re-written](https://github.com/Enlightenment/enlightenment/tree/master/src/modules/clipboard) upon addition.

If entrance development will continue remains to be seen, there is new interest
in entrance development, but like all other EFL app efforts, the future is
unknown. Hopefully, this version of entrance will not face abandonment as past
versions, and most EFL applications.

The best tech does not always win, it can rise or fall based on communities!

![A screenshot of Entrance](https://user-images.githubusercontent.com/12835340/31921581-1c2f0d7c-b83e-11e7-8d90-1dac94ae8e5c.jpg)

## Known Issues
- Settings UI is broken, hidden for now
[Issue #6](https://github.com/wltjr/entrance/issues/6)

## About
Entrance is a Unix Display/Login Manager written in Enlightenment Foundation 
Libraries (EFL). Entrance allows a user to choose a X WM/Desktop 
session to launch upon successful login. Entrance is alive and working 
again for logging into X sessions and eventually Wayland sessions!

The project has been resurrected from the dead to live on once again...

## History

Entrance is a long story. There have been 2 different code bases and
projects both using the name entrance.

### 3rd Generation

This project is the 3rd generation, fork of the 2nd Generation code
base, with a lot of fixes, and initial removal of incomplete and/or
broken code. Rather than fix as is, we are looking to replace functionality
with new code and finding different ways of accomplishing similar functionality.

This generation is currently in development, and should be usable. Please open
an [issue](https://github.com/wltjr/entrance/issues) for any problems encountered.

### 2nd Generation

Sometime later another came along, [Michael Bouchaud](https://github.com/eyoz)/@eyoz
who renamed his project elsa to Entrance, which is where the current code base
came from. It is not known if this was ever completed or worked, it did not
function correctly, if it is even usable to log in at all.

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

## Build
Entrance presently uses meson build system, autotools has been dropped. 

### Build using meson
```
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
ninja -C build
```

On most systems you likely need a pam file. Meson will install this file.
```
cp data/entrance.pam.d /etc/pam.d/entrance
```

The systemd service file is presently not installed. It may or may not 
be usable and/or correct. Please see the section on logind/elogind for 
further information.

## Configuration
Most things can be configured in entrance.conf at `/etc/entrance/entrance.conf`.
Some settings may not work. Please file issues for anything that is not 
configurable or does not work.

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
```
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
```
Section "Monitor"
	Identifier	"A"
	Option		"position"	"0 0"
EndSection

Section "Monitor"
	Identifier	"B"
	Option		"LeftOf"	"A"
EndSection

Section "Monitor"
	Identifier	"C"
	Option		"RightOf"	"A"
EndSection
```
Without such, only the background will appear. The login form will not 
appear along with the rest of the UI, and the cursor will be an X. This  
likely due to bugs in entrance code. The work around is the previously 
mentioned monitors section.

## logind/elogind aka systemd
Initial logind/elogind code has been added to entrance thanks to
[@OxusByte45](https://github.com/OxusByte45), and this works in some enviroments,
but we are still working out issues in CI before fully confirming that entrance
supports logind/elogind. That is the current area of development focus, for more
information please refer to
[Logind/elogind Integration Implementation](LOGIND_INTEGRATION.md).

This work should be completed soon, and there will be the first actual release
of entrance shortly thereafter.

## wayland display sessions
Once again thanks to [@OxusByte45](https://github.com/OxusByte45), entrance now
has initial support for launching Wayland sessions! This is not as heavily
tested, and is not being presented tested in CI. Please report any issues with
launching Wayland sessions, preferable, Enlightement under Wayland, when the 
mesa issues are resolved.

It is not known how well this will work to launch other Desktop enviroments,
but testing is welcomed!
