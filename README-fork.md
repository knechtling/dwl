# dwl Ignite

This fork keeps dwl light but adds a few power-user affordances: hot-reloadable code, a built-in bar with systray/menus, smart borders/gaps, sticky windows, scratchpads, and safer UX (notify when commands are missing).

## Why this fork stands out
- **Hot reload:** `make dwl.so` and trigger the reload keybinding to swap the compositor binary without restarting your session. Paths are resolved defensively to avoid crashes when `dwl.so` is missing.
- **Bar + systray + menus:** Built-in status bar with tray icons, right-click menus, window/layout menus, and clickable tray items.
- **Gaps/smart borders:** Tiled layouts respect configurable gaps, with smart gaps when a single client is present.
- **Quality-of-life:** Sticky clients, scratchpads, attachtop, smart border drawing, cursor auto-hide, uncluttered focus behavior, and winview (jump to focused client’s tags).
- **Safety:** Launcher failures use `notify-send` so missing tools don’t silently fail; hot reload failures also notify.

## Patches/features included
- Hot reloadable module (`HOT` build) with listener rebinding.
- Bar implementation with systray (tray icons, menus, activation).
- Layout/menu helpers (window/layout menus, winview).
- Smart borders + gaps + attachtop + sticky clients.
- Scratchpads with per-key scratch windows.
- Cursor auto-hide/unclutter tweaks.
- Global passkey handling, focus restore on reattach, urgent hint handling.
- Xwayland support kept in sync with dwl mainline.

## Building
```sh
make            # builds dwl (hot-reload capable)
make dwl.so     # builds the hot-reloadable module
```
Ensure `notify-send` (libnotify) is installed to see failure alerts.

## Keybindings (MOD = Super)

Launchers & system:
- `MOD+d` menu (wmenu-run); `MOD+Return` terminal; `MOD+Shift+Return` scratchpad; `MOD+Shift+L/W` password/net scratchpads.
- `MOD+b` toggle bar; `Control_R` wlr-which-key; `MOD+c` cliphist picker.
- `MOD+Backspace` sysact; `MOD+Shift+s` startup script; `MOD+F3` displayselect; `MOD+Alt+u/m/w/p/r` custom helpers; `MOD+F4` terminal pulsemixer; `MOD+F7` dmenuunicode; `MOD+i` pkg-install; `MOD+Shift+R` pkg-remove; `MOD+Shift+C` whatsapp-web; `MOD+F1` j4-dmenu-desktop.
- Volume: `MOD+Shift+*` loud down/up (15%); `MOD++` +5%; media keys for sink/mic volume, play/pause/next/prev/stop, brightness up/down.
- Apps: `MOD+w` browser; `MOD+z` music; `MOD+e` mail; `MOD+r` lfub; `MOD+Shift+H` htop; `MOD+Shift+Y` ytfzf; `MOD+n` nvim Wiki; `MOD+Shift+M` mute.
- Screenshots: `MOD+F8` full; `MOD+Shift+F8` region; `MOD+Ctrl+F8` region+copy.

Window management:
- Focus/stack: `MOD+j/k` cycle; `MOD+Shift+I/D` inc/dec master; `MOD+h/l` resize master; `MOD+space` zoom; `MOD+Tab` view toggle.
- Layouts: `MOD+t` tile; `MOD+f` float; `MOD+m` monocle; `MOD+g` toggle gaps; `MOD+y` fullscreen; `MOD+Shift+F` toggle floating.
- Menus: `MOD+o` window menu; `MOD+Alt+o` layout menu.

Tags/monitors:
- `MOD+0` view all; `MOD+v` winview (jump to focused client tags); `MOD+Shift+=` tag all.
- Focus/tag monitor: `MOD+,/.,</>` for left/right focus or move; `MOD+Shift+</>` move focused client.
- `MOD+[1-9]` view tag; `MOD+Ctrl+[1-9]` toggle view; `MOD+Shift+[1-9]` move to tag; `MOD+Ctrl+Shift+[1-9]` toggle tag.
- Quit: `MOD+Shift+Q` or `Ctrl+Alt+Backspace`; VTs: `Ctrl+Alt+F1..F12`.

Mouse:
- Layout symbol left/right click: set tile/monocle.
- Client: `MOD+LMB` move, `MOD+RMB` resize, `MOD+MMB` toggle floating.
- Tags: LMB view, RMB toggle view, `MOD+LMB` move to tag, `MOD+RMB` toggle tag.
- Tray: LMB activate item, RMB open menu.

## Hot reload workflow
1. Build: `make dwl.so`.
2. Trigger your reload keybinding: `MOD+Ctrl+F5`.
3. On success you’ll see a notification; on failure, a critical `notify-send` with the reason. Ensure `dwl.so` is readable in one of: current dir, `$HOME/.local/lib`, `/usr/local/lib`, `/usr/share/lib`, or alongside the running binary.
