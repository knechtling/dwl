# dwl - dwm for Wayland

Join us on our IRC channel: [#dwl on Libera Chat]  
Or on our [Discord server].

dwl is a compact, hackable compositor for [Wayland] based on [wlroots]. It is
intended to fill the same space in the Wayland world that dwm does in X11,
primarily in terms of functionality, and secondarily in terms of
philosophy. Like dwm, dwl is:

- Easy to understand, hack on, and extend with patches
- One C source file (or a very small number) configurable via `config.h`
- Tied to as few external dependencies as possible

## Getting Started

### **dwl branch 0.7 and releases based upon 0.7 build against [wlroots] 0.18**

### Latest semi-stable [release]

This is probably where you want to start. This builds against the dependent
packages' versions currently shipping in major distributions. If your
distribution's wlroots version is older, use an earlier dwl [release] or [0.x
branch].

### Development branch [main]

Active development progresses on the `main` branch. The `main` branch is built
against a late (and often changing) git commit of wlroots. While the adventurous
are welcome to use `main`, it is a rocky road. Using `main` requires that the
user be willing to chase git commits of wlroots. Testing development pull
requests may involve merging unmerged pull requests in [wlroots]' git repository
and/or git commits of wayland.

### Building dwl

dwl has the following dependencies:

- libinput
- wayland
- wlroots (compiled with the libinput backend)
- xkbcommon
- wayland-protocols (compile-time only)
- pkg-config (compile-time only)

dwl has the following additional dependencies if XWayland support is enabled:

- libxcb
- libxcb-wm
- wlroots (compiled with X11 support)
- Xwayland (runtime only)

Install these (and their `-devel` versions if your distro has separate
development packages) and run `make`. If you wish to build against a released
version of wlroots (_you probably do_), use a [release] or a [0.x branch]. If
you want to use the unstable development `main` branch, you need to use the git
version of [wlroots].

To enable XWayland, you should uncomment its flags in `config.mk`.

## Configuration

All configuration is done by editing `config.h` and recompiling, in the same
manner as dwm. There is no way to separately restart the window manager in
Wayland without restarting the entire display server, so any changes will take
effect the next time dwl is executed.

As in the dwm community, we encourage users to share patches they have
created. Check out the [dwl-patches] repository!

## Running dwl

dwl can be run on any of the backends supported by wlroots. This means you can
run it as a separate window inside either an X11 or Wayland session, as well as
directly from a VT console. Depending on your distro's setup, you may need to
add your user to the `video` and `input` groups before you can run dwl on a
VT. If you are using `elogind` or `systemd-logind` you need to install polkit;
otherwise you need to add yourself in the `seat` group and enable/start the
seatd daemon.

When dwl is run with no arguments, it will launch the server and begin handling
any shortcuts configured in `config.h`. There is no status bar or other
decoration initially; these are instead clients that can be run within the
Wayland session. Do note that the default background color is black. This can be
modified in `config.h`.

If you would like to run a script or command automatically at startup, you can
specify the command using the `-s` option. This command will be executed as a
shell command using `/bin/sh -c`. It serves a similar function to `.xinitrc`,
but differs in that the display server will not shut down when this process
terminates. Instead, dwl will send this process a SIGTERM at shutdown and wait
for it to terminate (if it hasn't already). This makes it ideal for execing into
a user service manager like [s6], [anopa], [runit], [dinit], or [`systemd
--user`].

Note: The `-s` command is run as a _child process_ of dwl, which means that it
does not have the ability to affect the environment of dwl or of any processes
that it spawns. If you need to set environment variables that affect the entire
dwl session, these must be set prior to running dwl. For example, Wayland
requires a valid `XDG_RUNTIME_DIR`, which is usually set up by a session manager
such as `elogind` or `systemd-logind`. If your system doesn't do this
automatically, you will need to configure it prior to launching `dwl`, e.g.:

    export XDG_RUNTIME_DIR=/tmp/xdg-runtime-$(id -u)
    mkdir -p $XDG_RUNTIME_DIR
    dwl

### Status information

Information about selected layouts, current window title, app-id, and
selected/occupied/urgent tags is written to the stdin of the `-s` command (see
the `printstatus()` function for details). This information can be used to
populate an external status bar with a script that parses the
information. Failing to read this information will cause dwl to block, so if you
do want to run a startup command that does not consume the status information,
you can close standard input with the `<&-` shell redirection, for example:

    dwl -s 'foot --server <&-'

If your startup command is a shell script, you can achieve the same inside the
script with the line

    exec <&-

To get a list of status bars that work with dwl consult our [wiki].

## Replacements for X applications

You can find a [list of useful resources on our wiki].

## Background

dwl is not meant to provide every feature under the sun. Instead, like dwm, it
sticks to features which are necessary, simple, and straightforward to implement
given the base on which it is built. Implemented default features are:

- Any features provided by dwm/Xlib: simple window borders, tags, keybindings,
  client rules, mouse move/resize. Providing a built-in status bar is an
  exception to this goal, to avoid dependencies on font rendering and/or drawing
  libraries when an external bar could work well.
- Configurable multi-monitor layout support, including position and rotation
- Configurable HiDPI/multi-DPI support
- Idle-inhibit protocol which lets applications such as mpv disable idle
  monitoring
- Provide information to external status bars via stdout/stdin
- Urgency hints via xdg-activate protocol
- Support screen lockers via ext-session-lock-v1 protocol
- Various Wayland protocols
- XWayland support as provided by wlroots (can be enabled in `config.mk`)
- Zero flickering - Wayland users naturally expect that "every frame is perfect"
- Layer shell popups (used by Waybar)
- Damage tracking provided by scenegraph API

Given the Wayland architecture, dwl has to implement features from dwm **and**
the xorg-server. Because of this, it is impossible to maintain the original
project goal of 2000 SLOC and have a reasonably complete compositor with
features comparable to dwm. However, this does not mean that the code will grow
indiscriminately. We will try to keep the code as small as possible.

Features under consideration (possibly as patches) are:

- Protocols made trivial by wlroots
- Implement the text-input and input-method protocols to support IME once ibus
  implements input-method v2 (see <https://github.com/ibus/ibus/pull/2256> and
  <https://codeberg.org/dwl/dwl/pulls/235>)

Feature _non-goals_ for the main codebase include:

- Client-side decoration (any more than is necessary to tell the clients not to)
- Client-initiated window management, such as move, resize, and close, which can
  be done through the compositor
- Animations and visual effects

## Acknowledgements

dwl began by extending the TinyWL example provided (CC0) by the sway/wlroots
developers. This was made possible in many cases by looking at how sway
accomplished something, then trying to do the same in as suckless a way as
possible.

Many thanks to suckless.org and the dwm developers and community for the
inspiration, and to the various contributors to the project, including:

- **Devin J. Pohly for creating and nurturing the fledgling project**
- Alexander Courtis for the XWayland implementation
- Guido Cella for the layer-shell protocol implementation, patch maintenance,
  and for helping to keep the project running
- Stivvo for output management and fullscreen support, and patch maintenance

## Keybindings & Functionality

This repository ships a custom `config.h`. `MODKEY` is set to the Super/Logo key
(`WLR_MODIFIER_LOGO`). Helper applications default to `foot` (terminal),
`firefox` (browser), `feishin` (music), and `thunderbird` (mail). Scratchpads are
retagged to the currently focused monitor whenever they are toggled, so the
floating terminal that pops up always follows your active output.

### Launchers & Scratchpads

| Keys                 | Action                                                                        |
| -------------------- | ----------------------------------------------------------------------------- |
| `Super+d`            | Application launcher (`wmenu-run`)                                            |
| `Super+Return`       | Open the default terminal (`foot`)                                            |
| `Super+Shift+Return` | Toggle the general scratchpad terminal (`foot -T scratchpad`)                 |
| `Super+Shift+s`      | Re-run `dwl-startup.sh` (mako, wallpaper/clipboard helpers, foot server)      |
| `Super+Alt+u`        | Run `dmenuhandler` to process the clipboard/link with mpv, queue, setbg, etc. |
| `Super+Alt+m`        | Run `dmenumountcifs` to mount a nearby CIFS share via Avahi                   |
| `Super+Alt+w`        | Launch the `weblaunch` desktop menu                                           |
| `Super+Alt+p`        | Take Wayland screenshots via `maimpick-wl`                                    |
| `Super+Alt+r`        | Open the `dmenurecord` wf-recorder/ffmpeg helper                              |
| `Super+Shift+p`      | Toggle the Bitwarden scratchpad (`bitwarden-desktop`)                         |
| `Super+Shift+w`      | Toggle the network scratchpad (`foot -T scratchnet -e nmtui`)                 |
| `Super+b`            | Show or hide the dwl bar                                                      |
| `Right Ctrl`         | Run `wlr-which-key` to display the keybinding cheat sheet                     |
| `Super+c`            | Pick from clipboard history (`cliphist → wmenu → wl-copy`)                    |
| `Super+Backspace`    | Launch `sysact` (system action helper)                                        |
| `Super+F3`           | Run `displayselect` to pick an output layout                                  |
| `Super+F4`           | Open `pulsemixer` inside the terminal                                         |
| `Super+F7`           | Launch `dmenuunicode`                                                         |
| `Super+i`            | Run `pkg-install` (terminal)                                                  |
| `Super+Shift+r`      | Run `pkg-remove` (terminal)                                                   |
| `Super+Shift+h`      | Open `htop` inside the terminal                                               |
| `Super+Shift+c`      | `gtk-launch whatsapp-web`                                                     |
| `Super+Shift+y`      | Start `ytfzf -t` inside the terminal                                          |
| `Super+F1`           | `j4-dmenu-desktop` (desktop entry launcher)                                   |
| `Super+r`            | Launch `lfub` file manager inside the terminal                                |
| `Super+n`            | Open `nvim` directly in the Wiki index (`nvim -c WikiIndex`)                  |
| `Super+w`            | Launch the browser (`firefox`)                                                |
| `Super+z`            | Launch the music player (`feishin`)                                           |
| `Super+e`            | Launch the e-mail client (`thunderbird`)                                      |
| `Super+Shift+m`      | Toggle sink mute via `wpctl set-mute @DEFAULT_AUDIO_SINK@ toggle`             |
| `F8`                 | Screenshot all monitors to `~/Pictures/screenshots`                           |
| `Shift+F8`           | Selection screenshot saved to `~/Pictures/screenshots`                        |
| `Ctrl+F8`            | Selection screenshot copied to clipboard                                      |

### Audio, Media & Hardware

| Keys                                   | Action                              |
| -------------------------------------- | ----------------------------------- |
| `Super+Shift+*` (Shift on the `+` key) | Lower sink volume by 15 % (`wpctl`) |
| `Super++`                              | Raise sink volume by 5 % (`wpctl`)  |
| `XF86AudioMute`                        | Toggle sink mute                    |
| `XF86AudioMicMute`                     | Toggle microphone mute              |
| `XF86AudioRaiseVolume`                 | Raise sink volume by 3 %            |
| `XF86AudioLowerVolume`                 | Lower sink volume by 3 %            |
| `XF86AudioPrev`                        | `mpc prev`                          |
| `XF86AudioNext`                        | `mpc next`                          |
| `XF86AudioPause`                       | `mpc pause`                         |
| `XF86AudioPlay`                        | `mpc play`                          |
| `XF86AudioStop`                        | `mpc stop`                          |
| `XF86MonBrightnessUp`                  | `brightnessctl set +15%`            |
| `XF86MonBrightnessDown`                | `brightnessctl set 15%-`            |

### Window & Layout Management

| Keys                              | Action                                         |
| --------------------------------- | ---------------------------------------------- |
| `Super+j` / `Super+k`             | Focus the next/previous tiled client           |
| `Super+Shift+I` / `Super+Shift+D` | Increase/decrease the number of master clients |
| `Super+h` / `Super+l`             | Shrink/grow the master area by 5 %             |
| `Super+Space`                     | Promote the focused client to master (`zoom`)  |
| `Super+Tab`                       | Jump back to the previous tag selection        |
| `Super+g`                         | Toggle gaps                                    |
| `Super+q`                         | Close the focused client                       |
| `Super+y`                         | Toggle fullscreen                              |
| `Super+Shift+f`                   | Toggle floating mode                           |
| `Super+t` / `Super+f` / `Super+m` | Select tile, floating, or monocle layout       |
| `Super+o`                         | Menu of open windows (`wmenu -p Windows`)      |
| `Super+Alt+o`                     | Layout picker menu (`wmenu -p Layouts`)        |
| `Super+Alt+r`                     | Dynamic rules menu (`wmenu -p Rules`)          |

### Tags & Monitors

| Keys                              | Action                                                        |
| --------------------------------- | ------------------------------------------------------------- |
| `Super+0`                         | View all tags                                                 |
| `Super+v`                         | Temporarily view the tag(s) of the focused client (`winview`) |
| `Super+Shift+=`                   | Tag the focused client with all tags                          |
| `Super+,` / `Super+.`             | Focus the monitor to the left/right                           |
| `Super+<` / `Super+>`             | Focus the monitor left/right while keeping current tags       |
| `Super+Shift+<` / `Super+Shift+>` | Send the focused client to the monitor left/right             |

For tags **1–9** on this German-layout configuration:

- `Super+<number>` – view that tag.
- `Super+Ctrl+<number>` – toggle that tag in the current view.
- `Super+Shift+<symbol>` – move the focused client to that tag (use the shifted symbol above the digit, e.g. `!` for `1`).
- `Super+Ctrl+Shift+<symbol>` – toggle-tag the focused client.

### Session, VTs & Scratchpads

| Keys                 | Action                                       |
| -------------------- | -------------------------------------------- |
| `Super+Shift+q`      | Quit dwl cleanly                             |
| `Ctrl+Alt+Backspace` | Force dwl to quit                            |
| `Ctrl+Alt+F1…F12`    | Switch to the corresponding virtual terminal |

### Mouse Bindings

| Button                           | Action                               |
| -------------------------------- | ------------------------------------ |
| `Super + Left drag` (client)     | Move the focused client              |
| `Super + Right drag` (client)    | Resize the focused client            |
| `Super + Middle click` (client)  | Toggle floating                      |
| `Left click` on layout symbol    | Select tiling layout                 |
| `Right click` on layout symbol   | Select monocle layout                |
| `Left click` on tag              | View that tag                        |
| `Right click` on tag             | Toggle that tag                      |
| `Super + Left click` on tag      | Tag the focused client with that tag |
| `Super + Right click` on tag     | Toggle-tag the focused client        |
| `Left click` inside the systray  | Activate the clicked tray item       |
| `Right click` inside the systray | Open the tray context menu           |

[`systemd --user`]: https://wiki.archlinux.org/title/Systemd/User
[#dwl on Libera Chat]: https://web.libera.chat/?channels=#dwl
[0.x branch]: https://codeberg.org/dwl/dwl/branches
[anopa]: https://jjacky.com/anopa/
[dinit]: https://davmac.org/projects/dinit/
[dwl-patches]: https://codeberg.org/dwl/dwl-patches
[list of useful resources on our wiki]: https://codeberg.org/dwl/dwl/wiki/Home#migrating-from-x
[main]: https://codeberg.org/dwl/dwl/src/branch/main
[release]: https://codeberg.org/dwl/dwl/releases
[runit]: http://smarden.org/runit/faq.html#userservices
[s6]: https://skarnet.org/software/s6/
[wlroots]: https://gitlab.freedesktop.org/wlroots/wlroots/
[wiki]: https://codeberg.org/dwl/dwl/wiki/Home#compatible-status-bars
[Discord server]: https://discord.gg/jJxZnrGPWN
[Wayland]: https://wayland.freedesktop.org/
