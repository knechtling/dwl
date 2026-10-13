#include "item.h"

#include "helpers.h"
#include "icon.h"
#include "watcher.h"

#include <dbus/dbus.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// IWYU pragma: no_include "dbus/dbus-protocol.h"
// IWYU pragma: no_include "dbus/dbus-shared.h"

#define RULEBSIZE 256

static const char *match_string =
	"type='signal',"
	"sender='%s',"
	"interface='" SNI_NAME
	"',"
	"member='NewIcon'";

static Watcher *
item_get_watcher(const Item *item)
{
	if (!item)
		return NULL;

	return item->watcher;
}

static DBusConnection *
item_get_connection(const Item *item)
{
	if (!item || !item->watcher)
		return NULL;

	return item->watcher->conn;
}

static const uint8_t *
extract_image(DBusMessageIter *iter, dbus_int32_t *width, dbus_int32_t *height,
              int *size)
{
	DBusMessageIter vals, bytes;
	const uint8_t *buf;

	dbus_message_iter_recurse(iter, &vals);
	if (dbus_message_iter_get_arg_type(&vals) != DBUS_TYPE_INT32)
		goto fail;
	dbus_message_iter_get_basic(&vals, width);

	dbus_message_iter_next(&vals);
	if (dbus_message_iter_get_arg_type(&vals) != DBUS_TYPE_INT32)
		goto fail;
	dbus_message_iter_get_basic(&vals, height);

	dbus_message_iter_next(&vals);
	if (dbus_message_iter_get_arg_type(&vals) != DBUS_TYPE_ARRAY)
		goto fail;
	dbus_message_iter_recurse(&vals, &bytes);
	if (dbus_message_iter_get_arg_type(&bytes) != DBUS_TYPE_BYTE)
		goto fail;
	dbus_message_iter_get_fixed_array(&bytes, &buf, size);
	if (size == 0)
		goto fail;

	return buf;

fail:
	return NULL;
}

static int
select_image(DBusMessageIter *iter, int target_width)
{
	DBusMessageIter vals;
	dbus_int32_t cur_width;
	int i = 0;

	do {
		dbus_message_iter_recurse(iter, &vals);
		if (dbus_message_iter_get_arg_type(&vals) != DBUS_TYPE_INT32)
			return -1;
		dbus_message_iter_get_basic(&vals, &cur_width);
		if (cur_width >= target_width)
			return i;

		i++;
	} while (dbus_message_iter_next(iter));

	/* return last index if desired not found */
	return --i;
}

static void
menupath_ready_handler(DBusPendingCall *pending, void *data)
{
	Item *item = data;

	DBusError err = DBUS_ERROR_INIT;
	DBusMessage *reply = NULL;
	DBusMessageIter iter, opath;
	char *path_dup = NULL;
	const char *path;

	reply = dbus_pending_call_steal_reply(pending);
	if (!reply)
		goto fail;

	if (dbus_set_error_from_message(&err, reply)) {
		fprintf(stderr, "DBus Error: %s - %s: Couldn't get menupath\n",
		        err.name, err.message);
		goto fail;
	}

	dbus_message_iter_init(reply, &iter);
	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT)
		goto fail;
	dbus_message_iter_recurse(&iter, &opath);
	if (dbus_message_iter_get_arg_type(&opath) != DBUS_TYPE_OBJECT_PATH)
		goto fail;
	dbus_message_iter_get_basic(&opath, &path);

	path_dup = strdup(path);
	if (!path_dup)
		goto fail;

	item->menu_busobj = path_dup;

	dbus_message_unref(reply);
	dbus_pending_call_unref(pending);
	return;

fail:
	free(path_dup);
	dbus_error_free(&err);
	if (reply)
		dbus_message_unref(reply);
	if (pending)
		dbus_pending_call_unref(pending);
}

/*
 * Gets the Id dbus property, which is the name of the application,
 * most of the time...
 * The initial letter will be used as a fallback icon
 */
static void
id_ready_handler(DBusPendingCall *pending, void *data)
{
	Item *item = data;

	DBusError err = DBUS_ERROR_INIT;
	DBusMessage *reply = NULL;
	DBusMessageIter iter, string;
	Watcher *watcher;
	char *id_dup = NULL;
	const char *id;

	watcher = item_get_watcher(item);

	reply = dbus_pending_call_steal_reply(pending);
	if (!reply)
		goto fail;

	if (dbus_set_error_from_message(&err, reply)) {
		fprintf(stderr, "DBus Error: %s - %s: Couldn't get appid\n",
		        err.name, err.message);
		goto fail;
	}

	dbus_message_iter_init(reply, &iter);
	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT)
		goto fail;
	dbus_message_iter_recurse(&iter, &string);
	if (dbus_message_iter_get_arg_type(&string) != DBUS_TYPE_STRING)
		goto fail;
	dbus_message_iter_get_basic(&string, &id);

	id_dup = strdup(id);
	if (!id_dup)
		goto fail;
	item->appid = id_dup;

	/* Don't trigger update if this item already has a real icon */
	if (!item->icon)
		watcher_update_trays(watcher);

	dbus_message_unref(reply);
	dbus_pending_call_unref(pending);
	return;

fail:
	dbus_error_free(&err);
	if (id_dup)
		free(id_dup);
	if (reply)
		dbus_message_unref(reply);
	if (pending)
		dbus_pending_call_unref(pending);
}

/* Sets the item icon, returns -1 on failure */
static int
item_set_icon(Item *item, const uint8_t *buf, int width, int height, int size)
{
	Icon *icon;

	if (item->icon && item->icon->size_orig == (size_t)size &&
	    memcmp(item->icon->buf_orig, buf, size) == 0)
		return 0; /* Icon didn't change */

	if (!(icon = createicon(buf, width, height, size)))
		return -1;
	if (item->icon)
		destroyicon(item->icon);
	item->icon = icon;
	watcher_update_trays(item_get_watcher(item));

	return 0;
}

static int
get_string_reply(DBusMessage *reply, const char **str)
{
	DBusMessageIter iter, string;

	if (!reply || dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR)
		return -1;
	dbus_message_iter_init(reply, &iter);
	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT)
		return -1;
	dbus_message_iter_recurse(&iter, &string);
	if (dbus_message_iter_get_arg_type(&string) != DBUS_TYPE_STRING)
		return -1;
	dbus_message_iter_get_basic(&string, str);

	return 0;
}

static void
themepath_ready_handler(DBusPendingCall *pending, void *data)
{
	Item *item = data;

	DBusMessage *reply;
	const char *path;

	reply = dbus_pending_call_steal_reply(pending);
	if (get_string_reply(reply, &path) == 0) {
		free(item->icon_themepath);
		item->icon_themepath = strdup(path);
	}

	if (reply)
		dbus_message_unref(reply);
	dbus_pending_call_unref(pending);
}

/*
 * Fallback for items without an IconPixmap (e.g. libayatana-appindicator):
 * look the IconName up in the icon theme.
 */
static void
iconname_ready_handler(DBusPendingCall *pending, void *data)
{
	Item *item = data;

	DBusMessage *reply;
	const char *name;
	uint8_t *buf = NULL;
	int width, height, size;

	reply = dbus_pending_call_steal_reply(pending);
	if (get_string_reply(reply, &name) == 0 &&
	    (buf = load_named_icon(name, item->icon_themepath, &width, &height,
	                           &size)))
		item_set_icon(item, buf, width, height, size);

	free(buf);
	if (reply)
		dbus_message_unref(reply);
	dbus_pending_call_unref(pending);
}

static void
request_named_icon(Item *item)
{
	DBusConnection *conn = item_get_connection(item);

	/* Requests are answered in order, so the theme path arrives first */
	request_property(conn, item->busname, item->busobj, "IconThemePath",
	                 SNI_IFACE, themepath_ready_handler, item);
	request_property(conn, item->busname, item->busobj, "IconName",
	                 SNI_IFACE, iconname_ready_handler, item);
}

static void
pixmap_ready_handler(DBusPendingCall *pending, void *data)
{
	Item *item = data;

	DBusMessage *reply = NULL;
	DBusMessageIter iter, array, select, strct;
	dbus_int32_t width, height;
	int selected_index, size;
	const uint8_t *buf;

	reply = dbus_pending_call_steal_reply(pending);
	if (!reply || dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR)
		goto fail;
	dbus_message_iter_init(reply, &iter);
	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT)
		goto fail;
	dbus_message_iter_recurse(&iter, &array);
	if (dbus_message_iter_get_arg_type(&array) != DBUS_TYPE_ARRAY)
		goto fail;
	dbus_message_iter_recurse(&array, &select);
	if (dbus_message_iter_get_arg_type(&select) != DBUS_TYPE_STRUCT)
		goto fail;
	selected_index = select_image(&select, 22); // Get the 22*22 image
	if (selected_index < 0)
		goto fail;

	dbus_message_iter_recurse(&array, &strct);
	if (dbus_message_iter_get_arg_type(&strct) != DBUS_TYPE_STRUCT)
		goto fail;
	for (int i = 0; i < selected_index; i++)
		dbus_message_iter_next(&strct);
	buf = extract_image(&strct, &width, &height, &size);
	if (!buf || item_set_icon(item, buf, width, height, size) < 0)
		goto fail;

	dbus_message_unref(reply);
	dbus_pending_call_unref(pending);
	return;

fail:
	/* No usable pixmap, try the icon theme */
	request_named_icon(item);
	if (reply)
		dbus_message_unref(reply);
	if (pending)
		dbus_pending_call_unref(pending);
}

static DBusHandlerResult
handle_newicon(Item *item, DBusConnection *conn, DBusMessage *msg)
{
	const char *sender = dbus_message_get_sender(msg);

	if (sender && strcmp(sender, item->busname) == 0) {
		request_property(conn, item->busname, item->busobj,
		                 "IconPixmap", SNI_IFACE, pixmap_ready_handler,
		                 item);

		return DBUS_HANDLER_RESULT_HANDLED;

	} else {
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}
}

static DBusHandlerResult
filter_bus(DBusConnection *conn, DBusMessage *msg, void *data)
{
	Item *item = data;

	if (dbus_message_is_signal(msg, SNI_IFACE, "NewIcon"))
		return handle_newicon(item, conn, msg);
	else
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

Item *
createitem(const char *busname, const char *busobj, Watcher *watcher)
{
	DBusConnection *conn;
	Item *item;
	char *busname_dup = NULL;
	char *busobj_dup = NULL;
	char match_rule[RULEBSIZE];

	item = calloc(1, sizeof(Item));
	busname_dup = strdup(busname);
	busobj_dup = strdup(busobj);
	if (!item || !busname_dup || !busobj_dup)
		goto fail;

	conn = watcher->conn;
	item->busname = busname_dup;
	item->busobj = busobj_dup;
	item->watcher = watcher;

	request_property(conn, busname, busobj, "IconPixmap", SNI_IFACE,
	                 pixmap_ready_handler, item);

	request_property(conn, busname, busobj, "Id", SNI_IFACE,
	                 id_ready_handler, item);

	request_property(conn, busname, busobj, "Menu", SNI_IFACE,
	                 menupath_ready_handler, item);

	if (snprintf(match_rule, sizeof(match_rule), match_string, busname) >=
	    RULEBSIZE) {
		goto fail;
	}

	if (!dbus_connection_add_filter(conn, filter_bus, item, NULL))
		goto fail;
	dbus_bus_add_match(conn, match_rule, NULL);

	return item;

fail:
	free(busname_dup);
	free(busobj_dup);
	return NULL;
}

void
destroyitem(Item *item)
{
	DBusConnection *conn;
	char match_rule[RULEBSIZE];

	conn = item_get_connection(item);

	if (snprintf(match_rule, sizeof(match_rule), match_string,
	             item->busname) < RULEBSIZE) {
		dbus_bus_remove_match(conn, match_rule, NULL);
		dbus_connection_remove_filter(conn, filter_bus, item);
	}
	if (item->icon)
		destroyicon(item->icon);
	free(item->menu_busobj);
	free(item->busname);
	free(item->busobj);
	free(item->appid);
	free(item->icon_themepath);
	free(item);
}

void
item_activate(Item *item)
{
	DBusConnection *conn;
	DBusMessage *msg = NULL;
	dbus_int32_t x = 0, y = 0;

	conn = item_get_connection(item);

	if (!(msg = dbus_message_new_method_call(item->busname, item->busobj,
	                                         SNI_IFACE, "Activate")) ||
	    !dbus_message_append_args(msg, DBUS_TYPE_INT32, &x, DBUS_TYPE_INT32,
	                              &y, DBUS_TYPE_INVALID) ||
	    !dbus_connection_send_with_reply(conn, msg, NULL, -1)) {
		goto fail;
	}

	dbus_message_unref(msg);
	return;

fail:
	if (msg)
		dbus_message_unref(msg);
}
