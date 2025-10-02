// Constants
#define TERMINAL "foot"
#define BROWSER "firefox"
#define MUSICPLAYER "feishin"
#define EMAILCLIENT "thunderbird"

/* Taken from https://github.com/djpohly/dwl/issues/466 */
#define COLOR(hex) {((hex >> 24) & 0xFF) / 255.0f, ((hex >> 16) & 0xFF) / 255.0f, ((hex >> 8) & 0xFF) / 255.0f, (hex & 0xFF) / 255.0f}
/* appearance */
static const int sloppyfocus = 1;               /* focus follows mouse */
static const int bypass_surface_visibility = 0; /* 1 means idle inhibitors will disable idle tracking even if it's
                                                   surface isn't visible  */
static const int smartgaps = 0;                 /* 1 means no outer gap when there is only one window */
static int gaps = 1;                            /* 1 means gaps between windows are added */
static const unsigned int gappx = 10;           /* gap pixel between windows */
static const unsigned int borderpx = 2;         /* border pixel of windows */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int showsystray = 1;               /* 0 means no systray */
static const int showbar = 1;                   /* 0 means no bar */
static const int topbar = 1;                    /* 0 means bottom bar */
static const char *fonts[] = {"monospace:size=10"};
static const float rootcolor[] = COLOR(0x000000ff);
/* This conforms to the xdg-protocol. Set the alpha to zero to restore the old
 * behavior */
static const float fullscreen_bg[] = {0.1f, 0.1f, 0.1f, 1.0f}; /* You can also use glsl colors */
static uint32_t colors[][3] = {
    /*               fg          bg          border    */
    [SchemeNorm] = {0xbbbbbbff, 0x222222ff, 0x444444ff},
    [SchemeSel] = {0xeeeeeeff, 0x005577ff, 0x0066ccff},
    [SchemeUrg] = {0, 0, 0x770000ff},
};

#define TAGCOUNT (9)
/* tagging - TAGCOUNT must be no greater than 31 */
static char *tags[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9"};

/* logging */
static int log_level = WLR_ERROR;

/* Max amount of dynamically added rules */
#define RULES_MAX 100

static const Menu menus[] = {
    /* command                            feed function        action function
     */
    {"wmenu -i -l 10 -p Windows", menuwinfeed, menuwinaction},
    {"wmenu -i -p Layouts", menulayoutfeed, menulayoutaction},
    {"wmenu -i -l 10 -p Rules", menurulefeed, menuruleaction},
};

/* NOTE: ALWAYS keep a rule declared even if you don't use rules (e.g leave at
 * least one example) */
static const Rule rules[] = {
    /* app_id             title         tags mask     isfloating   monitor
       scratchkey */
    {"Gimp_EXAMPLE", NULL, 0, 1, -1, 0}, /* Start on currently visible tags floating, not tiled */
    {NULL, "floating", 0, 1, -1, 0},     /* Start on ONLY tag "9" */
    {NULL, "scratchpad", 0, 1, -1, 's'}, {"bitwarden", NULL, 0, 1, -1, 'p'}, {NULL, "scratchnet", 0, 1, -1, 'n'},
};

/* layout(s) */
static const Layout layouts[] = {
    /* symbol     arrange function */
    {"[]=", tile},
    {"><>", NULL}, /* no layout function means floating behavior */
    {"[M]", monocle},
};

/* monitors */
/* (x=-1, y=-1) is reserved as an "autoconfigure" monitor position indicator
 * WARNING: negative values other than (-1, -1) cause problems with Xwayland
 * clients https://gitlab.freedesktop.org/xorg/xserver/-/issues/899
 */
/* NOTE: ALWAYS add a fallback rule, even if you are completely sure it won't be
 * used */
static const MonitorRule monrules[] = {
    /* name       mfact  nmaster scale layout       rotate/reflect x    y */
    /* example of a HiDPI laptop monitor:
    { "eDP-1",    0.5f,  1,      2,    &layouts[0], WL_OUTPUT_TRANSFORM_NORMAL,
    -1,  -1 },
    */
    /* defaults */
    {NULL, 0.55f, 1, 1, &layouts[0], WL_OUTPUT_TRANSFORM_NORMAL, -1, -1},
};

/* keyboard */
static const struct xkb_rule_names xkb_rules = {
    /* can specify fields: rules, model, layout, variant, options */
    .rules = NULL,           // use default
    .model = "pc105",        // common keyboard model
    .layout = "de",          // German layout
    .variant = NULL,         // no variant
    .options = "caps:escape" // make capslock additional escape
};

/* numlock and capslock */
static const int numlock = 1;
static const int capslock = 0;

static const int repeat_rate = 25;
static const int repeat_delay = 600;

/* Trackpad */
static const int tap_to_click = 1;
static const int tap_and_drag = 1;
static const int drag_lock = 1;
static const int natural_scrolling = 0;
static const int disable_while_typing = 1;
static const int left_handed = 0;
static const int middle_button_emulation = 0;
/* You can choose between:
LIBINPUT_CONFIG_SCROLL_NO_SCROLL
LIBINPUT_CONFIG_SCROLL_2FG
LIBINPUT_CONFIG_SCROLL_EDGE
LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN
*/
static const enum libinput_config_scroll_method scroll_method = LIBINPUT_CONFIG_SCROLL_2FG;

/* You can choose between:
LIBINPUT_CONFIG_CLICK_METHOD_NONE
LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS
LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER
*/
static const enum libinput_config_click_method click_method = LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS;

/* You can choose between:
LIBINPUT_CONFIG_SEND_EVENTS_ENABLED
LIBINPUT_CONFIG_SEND_EVENTS_DISABLED
LIBINPUT_CONFIG_SEND_EVENTS_DISABLED_ON_EXTERNAL_MOUSE
*/
static const uint32_t send_events_mode = LIBINPUT_CONFIG_SEND_EVENTS_ENABLED;

/* You can choose between:
LIBINPUT_CONFIG_ACCEL_PROFILE_FLAT
LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE
*/
static const enum libinput_config_accel_profile accel_profile = LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE;
static const double accel_speed = 0.0;

/* You can choose between:
LIBINPUT_CONFIG_TAP_MAP_LRM -- 1/2/3 finger tap maps to left/right/middle
LIBINPUT_CONFIG_TAP_MAP_LMR -- 1/2/3 finger tap maps to left/middle/right
*/
static const enum libinput_config_tap_button_map button_map = LIBINPUT_CONFIG_TAP_MAP_LRM;

/* If you want to use the windows key for MODKEY, use WLR_MODIFIER_LOGO */
#define MODKEY WLR_MODIFIER_LOGO

#define TAGKEYS(KEY, SKEY, TAG)                                                                                                                      \
  {MODKEY, KEY, view, {.ui = 1 << TAG}}, {MODKEY | WLR_MODIFIER_CTRL, KEY, toggleview, {.ui = 1 << TAG}},                                            \
      {MODKEY | WLR_MODIFIER_SHIFT, SKEY, tag, {.ui = 1 << TAG}}, {                                                                                  \
    MODKEY | WLR_MODIFIER_CTRL | WLR_MODIFIER_SHIFT, SKEY, toggletag, { .ui = 1 << TAG }                                                             \
  }

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd)                                                                                                                                   \
  {                                                                                                                                                  \
    .v = (const char *[]) { "/bin/sh", "-c", cmd, NULL }                                                                                             \
  }

/* commands */
static const char *termcmd[] = {TERMINAL, NULL};
static const char *menucmd[] = {"wmenu-run", NULL};
static const char *dmenucmd[] = {"wmenu", NULL};

/* named scratchpads - First arg only serves to match against key in rules*/
static const char *scratchpadcmd[] = {"s", "foot", "-T", "scratchpad", NULL};
static const char *scratchpasscmd[] = {"p", "bitwarden-desktop", NULL, NULL, NULL};
static const char *scratchnetcmd[] = {"n", "st", "-T", "scratchnet", "nmtui"};

static const Key keys[] = {
    /* Note that Shift changes certain key codes: c -> C, 2 -> at, etc. */
    /* modifier                  key                 function        argument */
    // system
    {MODKEY, XKB_KEY_d, spawn, {.v = menucmd}},
    {MODKEY, XKB_KEY_Return, spawn, {.v = termcmd}},
    {MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_Return, togglescratch, {.v = scratchpadcmd}},
    {MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_P, togglescratch, {.v = scratchpasscmd}},
    {MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_W, togglescratch, {.v = scratchnetcmd}},
    {MODKEY, XKB_KEY_b, togglebar, {0}},
    {0, XKB_KEY_Control_R, spawn, SHCMD("wlr-which-key")},
    {MODKEY, XKB_KEY_c, spawn, SHCMD("cliphist list | wmenu | cliphist decode | wl-copy")},
    // utilities
    {MODKEY, XKB_KEY_BackSpace, spawn, {.v = (const char *[]){"sysact", NULL}}},
    // {MODKEY, XKB_KEY_F1, spawn, SHCMD("zathura /home/anton/Nextcloud/dox/important/tud-indexed-secret.pdf")},
    // {MODKEY, XKB_KEY_F2, spawn, {.v = (const char *[]){"tutorialvids", NULL}}},
    {MODKEY, XKB_KEY_F3, spawn, {.v = (const char *[]){"displayselect", NULL}}},
    {MODKEY, XKB_KEY_F4, spawn, SHCMD(TERMINAL " -e pulsemixer")},
    // {MODKEY, XKB_KEY_F6, spawn, {.v = (const char *[]){"torwrap", NULL}}},
    {MODKEY, XKB_KEY_F7, spawn, {.v = (const char *[]){"dmenuunicode"}}},
    // NULL}}}, {MODKEY, XKB_KEY_F8, spawn, {.v = (const char *[]){"maimpick",
    // NULL}}}, {MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_F8, spawn, {.v = (const
    // char *[]){"ss-uni", NULL}}}, {MODKEY, XKB_KEY_F9, spawn, {.v = (const
    // char *[]){"mounter", NULL}}}, {MODKEY, XKB_KEY_F10, spawn, {.v = (const
    // char *[]){"unmounter", NULL}}},
    // {MODKEY, XKB_KEY_F11, spawn,
    // SHCMD("mpv --untimed --no-cache --no-osc --no-input-default-bindings "
    // "--profile=low-latency --input-conf=/dev/null --title=webcam $(ls "
    // "/dev/video[0,2,4,6,8] | tail -n 1)")},
    {MODKEY, XKB_KEY_i, spawn, {.v = (const char *[]){"foot", "-e", "pkg-install", NULL}}},
    {MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_R, spawn, {.v = (const char *[]){"foot", "-e", "pkg-remove", NULL}}},
    {MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_C, spawn, SHCMD("gtk-launch whatsapp-web")},
    {MODKEY, XKB_KEY_F1, spawn, {.v = (const char *[]){"j4-dmenu-desktop"}}},
    {MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_asterisk, spawn, SHCMD("wpctl set-volume @DEFAULT_AUDIO_SINK@ 15%-")},
    {MODKEY, XKB_KEY_plus, spawn, SHCMD("wpctl set-volume @DEFAULT_AUDIO_SINK@ 5%+")},
    {MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_asterisk, spawn, SHCMD("wpctl set-volume @DEFAULT_AUDIO_SINK@ 15%+")},
    {MODKEY, XKB_KEY_w, spawn, {.v = (const char *[]){BROWSER, NULL}}},
    {MODKEY, XKB_KEY_z, spawn, {.v = (const char *[]){MUSICPLAYER, NULL}}},
    {MODKEY, XKB_KEY_e, spawn, {.v = (const char *[]){EMAILCLIENT, NULL}}},
    {MODKEY, XKB_KEY_r, spawn, {.v = (const char *[]){TERMINAL, "-e", "lfub", NULL}}},
    {MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_R, spawn, {.v = (const char *[]){TERMINAL, "-e", "htop", NULL}}},
    {MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_Y, spawn, {.v = (const char *[]){TERMINAL, "-e", "ytfzf", "-t", NULL}}},
    {MODKEY, XKB_KEY_n, spawn, {.v = (const char *[]){TERMINAL, "-e", "nvim", "-c", "WikiIndex", NULL}}},
    {MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_M, spawn, SHCMD("wpctl set-mute @DEFAULT_AUDIO_SINK@ toggle")},
    // XF86 Media Keys
    {0, XKB_KEY_XF86AudioMute, spawn, SHCMD("wpctl set-mute @DEFAULT_AUDIO_SINK@ toggle")},
    {0, XKB_KEY_XF86AudioMicMute, spawn, SHCMD("pactl set-source-mute @DEFAULT_SOURCE@ toggle")},
    {0, XKB_KEY_XF86AudioRaiseVolume, spawn, SHCMD("wpctl set-volume @DEFAULT_AUDIO_SINK@ 3%+")},
    {0, XKB_KEY_XF86AudioLowerVolume, spawn, SHCMD("wpctl set-volume @DEFAULT_AUDIO_SINK@ 3%-")},
    {0, XKB_KEY_XF86AudioPrev, spawn, {.v = (const char *[]){"mpc", "prev", NULL}}},
    {0, XKB_KEY_XF86AudioNext, spawn, {.v = (const char *[]){"mpc", "next", NULL}}},
    {0, XKB_KEY_XF86AudioPause, spawn, {.v = (const char *[]){"mpc", "pause", NULL}}},
    {0, XKB_KEY_XF86AudioPlay, spawn, {.v = (const char *[]){"mpc", "play", NULL}}},
    {0, XKB_KEY_XF86AudioStop, spawn, {.v = (const char *[]){"mpc", "stop", NULL}}},
    {0, XKB_KEY_XF86MonBrightnessUp, spawn, {.v = (const char *[]){"brightnessctl", "set", "+15%", NULL}}},
    {0, XKB_KEY_XF86MonBrightnessDown, spawn, {.v = (const char *[]){"brightnessctl", "set", "15%-", NULL}}},
    // Windows
    {MODKEY, XKB_KEY_j, focusstack, {.i = +1}},
    {MODKEY, XKB_KEY_k, focusstack, {.i = -1}},
    {MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_I, incnmaster, {.i = +1}},
    {MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_D, incnmaster, {.i = -1}},
    {MODKEY, XKB_KEY_h, setmfact, {.f = -0.05f}},
    {MODKEY, XKB_KEY_l, setmfact, {.f = +0.05f}},
    {MODKEY, XKB_KEY_space, zoom, {0}},
    {MODKEY, XKB_KEY_Tab, view, {0}},
    {MODKEY, XKB_KEY_g, togglegaps, {0}},
    {MODKEY, XKB_KEY_q, killclient, {0}},
    {MODKEY, XKB_KEY_y, togglefullscreen, {0}},
    {MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_F, togglefloating, {0}},
    // layouts
    {MODKEY, XKB_KEY_t, setlayout, {.v = &layouts[0]}},
    {MODKEY, XKB_KEY_f, setlayout, {.v = &layouts[1]}},
    {MODKEY, XKB_KEY_m, setlayout, {.v = &layouts[2]}},
    // {MODKEY, XKB_KEY_space, setlayout, {0}},
    //
    // menus
    {MODKEY, XKB_KEY_o, menu, {.v = &menus[0]}},
    {MODKEY | WLR_MODIFIER_ALT, XKB_KEY_o, menu, {.v = &menus[1]}},
    {MODKEY | WLR_MODIFIER_ALT, XKB_KEY_r, menu, {.v = &menus[2]}},
    // tags
    {MODKEY, XKB_KEY_0, view, {.ui = ~0}},
    {MODKEY, XKB_KEY_v, winview, {0}},
    {MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_equal, tag, {.ui = ~0}},
    {MODKEY, XKB_KEY_comma, focusmon, {.i = WLR_DIRECTION_LEFT}},
    {MODKEY, XKB_KEY_period, focusmon, {.i = WLR_DIRECTION_RIGHT}},
    {MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_less, tagmon, {.i = WLR_DIRECTION_LEFT}},
    {MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_greater, tagmon, {.i = WLR_DIRECTION_RIGHT}},
    TAGKEYS(XKB_KEY_1, XKB_KEY_exclam, 0),
    TAGKEYS(XKB_KEY_2, XKB_KEY_quotedbl, 1),
    TAGKEYS(XKB_KEY_3, XKB_KEY_section, 2),
    TAGKEYS(XKB_KEY_4, XKB_KEY_dollar, 3),
    TAGKEYS(XKB_KEY_5, XKB_KEY_percent, 4),
    TAGKEYS(XKB_KEY_6, XKB_KEY_ampersand, 5),
    TAGKEYS(XKB_KEY_7, XKB_KEY_slash, 6),
    TAGKEYS(XKB_KEY_8, XKB_KEY_parenleft, 7),
    TAGKEYS(XKB_KEY_9, XKB_KEY_parenright, 8),
    {MODKEY | WLR_MODIFIER_SHIFT, XKB_KEY_Q, quit, {0}},

    /* Ctrl-Alt-Backspace and Ctrl-Alt-Fx used to be handled by X server */
    {WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_Terminate_Server, quit, {0}},
/* Ctrl-Alt-Fx is used to switch to another VT, if you don't know what a VT is
 * do not remove them.
 */
#define CHVT(n)                                                                                                                                      \
  {                                                                                                                                                  \
    WLR_MODIFIER_CTRL | WLR_MODIFIER_ALT, XKB_KEY_XF86Switch_VT_##n, chvt, { .ui = (n) }                                                             \
  }
    CHVT(1),
    CHVT(2),
    CHVT(3),
    CHVT(4),
    CHVT(5),
    CHVT(6),
    CHVT(7),
    CHVT(8),
    CHVT(9),
    CHVT(10),
    CHVT(11),
    CHVT(12),
};

static const Button buttons[] = {
    {ClkLtSymbol, 0, BTN_LEFT, setlayout, {.v = &layouts[0]}},
    {ClkLtSymbol, 0, BTN_RIGHT, setlayout, {.v = &layouts[2]}},
    {ClkClient, MODKEY, BTN_LEFT, moveresize, {.ui = CurMove}},
    {ClkClient, MODKEY, BTN_MIDDLE, togglefloating, {0}},
    {ClkClient, MODKEY, BTN_RIGHT, moveresize, {.ui = CurResize}},
    {ClkClient, MODKEY, BTN_LEFT, moveresize, {.ui = CurMove}},
    {ClkClient, MODKEY, BTN_MIDDLE, togglefloating, {0}},
    {ClkClient, MODKEY, BTN_RIGHT, moveresize, {.ui = CurResize}},
    {ClkClient, MODKEY, BTN_LEFT, moveresize, {.ui = CurMove}},
    {ClkClient, MODKEY, BTN_MIDDLE, togglefloating, {0}},
    {ClkClient, MODKEY, BTN_RIGHT, moveresize, {.ui = CurResize}},
    {ClkTagBar, 0, BTN_LEFT, view, {0}},
    {ClkTagBar, 0, BTN_RIGHT, toggleview, {0}},
    {ClkClient, MODKEY, BTN_LEFT, moveresize, {.ui = CurMove}},
    {ClkClient, MODKEY, BTN_MIDDLE, togglefloating, {0}},
    {ClkClient, MODKEY, BTN_RIGHT, moveresize, {.ui = CurResize}},
    {ClkTagBar, 0, BTN_LEFT, view, {0}},
    {ClkTagBar, 0, BTN_RIGHT, toggleview, {0}},
    {ClkTagBar, MODKEY, BTN_LEFT, tag, {0}},
    {ClkTagBar, MODKEY, BTN_RIGHT, toggletag, {0}},
    {ClkTray, 0, BTN_LEFT, trayactivate, {0}},
    {ClkTray, 0, BTN_RIGHT, traymenu, {0}},
};
