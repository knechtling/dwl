#ifndef MENU_H
#define MENU_H

#include <dbus/dbus.h>
#include <sys/types.h>
#include <wayland-server-core.h>

/* The menu is built on demand and not kept around */
void menu_show (DBusConnection *conn, struct wl_event_loop *loop,
                const char *busname, const char *busobj, const char **menucmd);
pid_t menu_get_pid (void);
void menu_close (void);

#endif /* MENU_H */
