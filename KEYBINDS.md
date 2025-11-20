# dwl Keybindings

**MODKEY**: Alt (can be changed to Super/Windows key by setting `MODKEY` to `WLR_MODIFIER_LOGO` in config.h)

## Applications & Launching

| Keybinding | Action |
|------------|--------|
| `Mod + p` | Launch application menu (wmenu-run) |
| `Mod + Shift + Return` | Launch terminal (foot) |

## Window Management

### Focus & Navigation
| Keybinding | Action |
|------------|--------|
| `Mod + j` | Focus next window in stack |
| `Mod + k` | Focus previous window in stack |
| `Mod + Return` | Zoom (swap focused window with master) |
| `Mod + Tab` | Toggle between current and previous tag |

### Window Actions
| Keybinding | Action |
|------------|--------|
| `Mod + Shift + c` | Kill focused window |
| `Mod + Shift + Space` | Toggle floating mode for focused window |
| `Mod + e` | Toggle fullscreen for focused window |

### Layout Control
| Keybinding | Action |
|------------|--------|
| `Mod + i` | Increase number of master windows |
| `Mod + d` | Decrease number of master windows |
| `Mod + h` | Decrease master area width (-5%) |
| `Mod + l` | Increase master area width (+5%) |
| `Mod + t` | Set tiled layout |
| `Mod + f` | Set floating layout |
| `Mod + m` | Set monocle layout |
| `Mod + Space` | Cycle through layouts |

## Tags (Workspaces)

### View Tags
| Keybinding | Action |
|------------|--------|
| `Mod + 1-9` | View tag 1-9 |
| `Mod + 0` | View all tags |
| `Mod + Ctrl + 1-9` | Toggle viewing tag 1-9 (show multiple tags) |

### Move Windows to Tags
| Keybinding | Action |
|------------|--------|
| `Mod + Shift + 1-9` | Move focused window to tag 1-9 |
| `Mod + Shift + 0` | Make focused window visible on all tags |
| `Mod + Ctrl + Shift + 1-9` | Toggle focused window on tag 1-9 |

## Multi-Monitor

| Keybinding | Action |
|------------|--------|
| `Mod + ,` | Focus monitor to the left |
| `Mod + .` | Focus monitor to the right |
| `Mod + Shift + ,` | Move focused window to monitor on the left |
| `Mod + Shift + .` | Move focused window to monitor on the right |

## System & Interface

| Keybinding | Action |
|------------|--------|
| `Mod + b` | Toggle status bar visibility |
| `Mod + Shift + r` | Hot reload configuration (dwl.so) |
| `Mod + Shift + q` | Quit dwl |
| `Ctrl + Alt + Backspace` | Quit dwl (alternative) |
| `Ctrl + Alt + F1-F12` | Switch to virtual terminal 1-12 |

## Mouse Bindings

### On Client Window
| Action | Effect |
|--------|--------|
| `Mod + Left Click` | Move window |
| `Mod + Middle Click` | Toggle floating |
| `Mod + Right Click` | Resize window |

### On Status Bar Elements
| Action | Effect |
|--------|--------|
| `Left Click on Layout Symbol` | Set tiled layout |
| `Right Click on Layout Symbol` | Set monocle layout |
| `Middle Click on Window Title` | Zoom (swap with master) |
| `Middle Click on Status Text` | Launch terminal |
| `Left Click on Tag` | View that tag |
| `Right Click on Tag` | Toggle viewing that tag |
| `Mod + Left Click on Tag` | Move focused window to that tag |
| `Mod + Right Click on Tag` | Toggle focused window on that tag |

## Special Application Pass-through Keys

Some applications need specific keys passed through:
- **OBS Studio**: Home, End, F12
- **WebCord**: `n` key

## Trackpad Settings

- **Tap to click**: Enabled
- **Tap and drag**: Enabled
- **Drag lock**: Enabled
- **Natural scrolling**: Disabled
- **Disable while typing**: Enabled
- **Scroll method**: Two-finger scroll
- **Click method**: Button areas
- **Cursor timeout**: 5 seconds (cursor hides after inactivity)

## Keyboard Settings

- **Repeat rate**: 25 keys/second
- **Repeat delay**: 600ms

## Notes

- The modifier key (Mod) is set to Alt by default. To use the Windows/Super key instead, change `MODKEY` to `WLR_MODIFIER_LOGO` in config.h
- Shift changes key codes (e.g., `2` becomes `@`, `c` becomes `C`)
- All keybindings can be customized in `config.h`
