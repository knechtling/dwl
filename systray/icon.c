#include "icon.h"

#include <cairo.h>
#include <fcft/fcft.h>
#include <librsvg/rsvg.h>
#include <pixman.h>

#include <ctype.h>
#include <glob.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PREMUL_ALPHA(chan, alpha) (chan * alpha + 127) / 255

/*
 * Converts pixels from uint8_t[4] to uint32_t and
 * straight alpha to premultiplied alpha.
 */
static uint32_t *
to_pixman(const uint8_t *src, int n_pixels, size_t *pix_size)
{
	uint32_t *dest = NULL;

	*pix_size = n_pixels * sizeof(uint32_t);
	dest = malloc(*pix_size);
	if (!dest)
		return NULL;

	for (int i = 0; i < n_pixels; i++) {
		uint8_t a = src[i * 4 + 0];
		uint8_t r = src[i * 4 + 1];
		uint8_t g = src[i * 4 + 2];
		uint8_t b = src[i * 4 + 3];

		/*
		 * Skip premultiplying fully opaque and fully transparent
		 * pixels.
		 */
		if (a == 0) {
			dest[i] = 0;

		} else if (a == 255) {
			dest[i] = ((uint32_t)a << 24) | ((uint32_t)r << 16) |
			          ((uint32_t)g << 8) | ((uint32_t)b);

		} else {
			dest[i] = ((uint32_t)a << 24) |
			          ((uint32_t)PREMUL_ALPHA(r, a) << 16) |
			          ((uint32_t)PREMUL_ALPHA(g, a) << 8) |
			          ((uint32_t)PREMUL_ALPHA(b, a));
		}
	}

	return dest;
}

Icon *
createicon(const uint8_t *buf, int width, int height, int size)
{
	Icon *icon = NULL;

	int n_pixels;
	pixman_image_t *img = NULL;
	size_t pixbuf_size;
	uint32_t *buf_pixman = NULL;
	uint8_t *buf_orig = NULL;

	n_pixels = size / 4;

	icon = calloc(1, sizeof(Icon));
	buf_orig = malloc(size);
	buf_pixman = to_pixman(buf, n_pixels, &pixbuf_size);
	if (!icon || !buf_orig || !buf_pixman)
		goto fail;

	img = pixman_image_create_bits(PIXMAN_a8r8g8b8, width, height,
	                               buf_pixman, width * 4);
	if (!img)
		goto fail;

	memcpy(buf_orig, buf, size);

	icon->buf_orig = buf_orig;
	icon->buf_pixman = buf_pixman;
	icon->img = img;
	icon->size_orig = size;
	icon->size_pixman = pixbuf_size;

	return icon;

fail:
	free(buf_orig);
	if (img)
		pixman_image_unref(img);
	free(buf_pixman);
	free(icon);
	return NULL;
}

void
destroyicon(Icon *icon)
{
	if (icon->img)
		pixman_image_unref(icon->img);
	free(icon->buf_orig);
	free(icon->buf_pixman);
	free(icon);
}

FallbackIcon *
createfallbackicon(const char *appname, int fgcolor, struct fcft_font *font)
{
	const struct fcft_glyph *glyph;
	char initial;

	if ((unsigned char)appname[0] > 127) {
		/* first character is not ascii */
		initial = '?';
	} else {
		initial = toupper(*appname);
	}

	glyph = fcft_rasterize_char_utf32(font, initial, FCFT_SUBPIXEL_DEFAULT);
	if (!glyph)
		return NULL;

	return glyph;
}

int
resize_image(pixman_image_t *image, int new_width, int new_height)
{
	int src_width = pixman_image_get_width(image);
	int src_height = pixman_image_get_height(image);
	pixman_transform_t transform;
	pixman_fixed_t scale_x, scale_y;

	if (src_width == new_width && src_height == new_height)
		return 0;

	scale_x = pixman_double_to_fixed((double)src_width / new_width);
	scale_y = pixman_double_to_fixed((double)src_height / new_height);

	pixman_transform_init_scale(&transform, scale_x, scale_y);
	if (!pixman_image_set_filter(image, PIXMAN_FILTER_BEST, NULL, 0) ||
	    !pixman_image_set_transform(image, &transform)) {
		return -1;
	}

	return 0;
}

/*
 * Icon lookup by name (IconName property), for items that don't
 * provide an IconPixmap. This is a simplified version of the freedesktop
 * icon theme spec: search the theme, its parents and hicolor, preferring
 * scalable icons, then the largest bitmap.
 */

#define NAMED_ICON_SIZE 64
#define PATHBSIZE 4096

static int
file_exists(const char *path)
{
	return access(path, R_OK) == 0;
}

static void
read_icon_theme(char *theme, size_t len)
{
	char path[PATHBSIZE], line[512];
	const char *home = getenv("HOME");
	const char *key = "gtk-icon-theme-name";
	FILE *f;

	snprintf(theme, len, "hicolor");
	if (!home)
		return;
	snprintf(path, sizeof(path), "%s/.config/gtk-3.0/settings.ini", home);
	if (!(f = fopen(path, "r")))
		return;
	while (fgets(line, sizeof(line), f)) {
		char *v;
		if (strncmp(line, key, strlen(key)) != 0 ||
		    !(v = strchr(line, '=')))
			continue;
		v++;
		while (*v == ' ')
			v++;
		v[strcspn(v, " \r\n")] = '\0';
		if (*v)
			snprintf(theme, len, "%s", v);
		break;
	}
	fclose(f);
}

/* Higher is better: svg beats any bitmap, larger bitmaps beat smaller */
static int
score_path(const char *path)
{
	const char *ext = strrchr(path, '.');
	const char *p;
	int size = 0;

	if (ext && strcmp(ext, ".svg") == 0)
		return 10000;
	for (p = path; *p; p++) {
		if (p[0] == '/' && p[1] >= '0' && p[1] <= '9') {
			size = atoi(p + 1);
			break;
		}
	}
	return size > NAMED_ICON_SIZE ? NAMED_ICON_SIZE : size;
}

/* Searches dir/<size>/<context>/name.ext and dir/name.ext */
static int
find_in_dir(const char *dir, const char *name, char *out, size_t outlen)
{
	static const char *const patterns[] = {
		"%s/*/*/%s.svg", "%s/*/*/%s.png", "%s/%s.svg", "%s/%s.png",
	};
	char pattern[PATHBSIZE];
	int best = -1;

	for (size_t i = 0; i < sizeof(patterns) / sizeof(patterns[0]); i++) {
		glob_t g;
		snprintf(pattern, sizeof(pattern), patterns[i], dir, name);
		if (glob(pattern, 0, NULL, &g) != 0)
			continue;
		for (size_t j = 0; j < g.gl_pathc; j++) {
			int s = score_path(g.gl_pathv[j]);
			if (s > best) {
				best = s;
				snprintf(out, outlen, "%s", g.gl_pathv[j]);
			}
		}
		globfree(&g);
	}
	return best >= 0;
}

static int
find_in_theme(const char *theme, const char *name, char *out, size_t outlen,
              int depth)
{
	char bases[8][PATHBSIZE], dir[PATHBSIZE], line[512];
	const char *home = getenv("HOME");
	const char *datahome = getenv("XDG_DATA_HOME");
	const char *datadirs = getenv("XDG_DATA_DIRS");
	char dirs_copy[PATHBSIZE], *tok, *save;
	int nbases = 0;
	FILE *f;

	if (datahome && *datahome)
		snprintf(bases[nbases++], PATHBSIZE, "%s/icons", datahome);
	else if (home)
		snprintf(bases[nbases++], PATHBSIZE, "%s/.local/share/icons",
		         home);
	if (home)
		snprintf(bases[nbases++], PATHBSIZE, "%s/.icons", home);
	snprintf(dirs_copy, sizeof(dirs_copy), "%s",
	         datadirs && *datadirs ? datadirs : "/usr/local/share:/usr/share");
	for (tok = strtok_r(dirs_copy, ":", &save); tok && nbases < 8;
	     tok = strtok_r(NULL, ":", &save))
		snprintf(bases[nbases++], PATHBSIZE, "%s/icons", tok);

	for (int i = 0; i < nbases; i++) {
		snprintf(dir, sizeof(dir), "%s/%s", bases[i], theme);
		if (find_in_dir(dir, name, out, outlen))
			return 1;
	}

	if (depth > 4)
		return 0;

	/* Recurse into parent themes listed in index.theme */
	for (int i = 0; i < nbases; i++) {
		snprintf(dir, sizeof(dir), "%s/%s/index.theme", bases[i], theme);
		if (!(f = fopen(dir, "r")))
			continue;
		while (fgets(line, sizeof(line), f)) {
			if (strncmp(line, "Inherits=", 9) != 0)
				continue;
			line[strcspn(line, "\r\n")] = '\0';
			for (tok = strtok_r(line + 9, ",", &save); tok;
			     tok = strtok_r(NULL, ",", &save)) {
				if (strcmp(tok, "hicolor") != 0 &&
				    find_in_theme(tok, name, out, outlen,
				                  depth + 1)) {
					fclose(f);
					return 1;
				}
			}
			break;
		}
		fclose(f);
		break;
	}
	return 0;
}

static int
lookup_icon(const char *name, const char *themepath, char *out, size_t outlen)
{
	char theme[256];

	if (name[0] == '/') {
		snprintf(out, outlen, "%s", name);
		return file_exists(out);
	}
	if (themepath && *themepath && find_in_dir(themepath, name, out, outlen))
		return 1;

	read_icon_theme(theme, sizeof(theme));
	if (find_in_theme(theme, name, out, outlen, 0) ||
	    find_in_theme("hicolor", name, out, outlen, 0))
		return 1;

	snprintf(out, outlen, "/usr/share/pixmaps/%s.png", name);
	if (file_exists(out))
		return 1;
	snprintf(out, outlen, "/usr/share/pixmaps/%s.svg", name);
	return file_exists(out);
}

static cairo_surface_t *
render_file(const char *path)
{
	const char *ext = strrchr(path, '.');
	cairo_surface_t *surf;

	if (ext && strcmp(ext, ".svg") == 0) {
		RsvgRectangle viewport = {0, 0, NAMED_ICON_SIZE, NAMED_ICON_SIZE};
		RsvgHandle *handle;
		cairo_t *cr;
		gboolean ok;

		if (!(handle = rsvg_handle_new_from_file(path, NULL)))
			return NULL;
		surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
		                                  NAMED_ICON_SIZE,
		                                  NAMED_ICON_SIZE);
		cr = cairo_create(surf);
		ok = rsvg_handle_render_document(handle, cr, &viewport, NULL);
		cairo_destroy(cr);
		g_object_unref(handle);
		if (!ok) {
			cairo_surface_destroy(surf);
			return NULL;
		}
	} else {
		surf = cairo_image_surface_create_from_png(path);
	}

	if (cairo_surface_status(surf) != CAIRO_STATUS_SUCCESS) {
		cairo_surface_destroy(surf);
		return NULL;
	}
	cairo_surface_flush(surf);
	return surf;
}

/*
 * Returns a malloc'd buffer in the IconPixmap format (ARGB bytes,
 * straight alpha) so it can go through createicon() like a pixmap.
 */
uint8_t *
load_named_icon(const char *name, const char *themepath, int *width,
                int *height, int *size)
{
	char path[PATHBSIZE];
	cairo_surface_t *surf;
	const uint8_t *data;
	uint8_t *buf;
	int w, h, stride;

	if (!name || !*name || !lookup_icon(name, themepath, path, sizeof(path)))
		return NULL;
	if (!(surf = render_file(path)))
		return NULL;

	w = cairo_image_surface_get_width(surf);
	h = cairo_image_surface_get_height(surf);
	stride = cairo_image_surface_get_stride(surf);
	data = cairo_image_surface_get_data(surf);

	if (cairo_image_surface_get_format(surf) != CAIRO_FORMAT_ARGB32 ||
	    !(buf = malloc((size_t)w * h * 4))) {
		cairo_surface_destroy(surf);
		return NULL;
	}

	for (int y = 0; y < h; y++) {
		const uint32_t *row = (const uint32_t *)(data + y * stride);
		for (int x = 0; x < w; x++) {
			uint32_t px = row[x];
			uint8_t a = px >> 24;
			uint8_t *d = buf + (y * w + x) * 4;
			d[0] = a;
			/* cairo is premultiplied, IconPixmap is not */
			d[1] = a ? ((px >> 16) & 0xff) * 255 / a : 0;
			d[2] = a ? ((px >> 8) & 0xff) * 255 / a : 0;
			d[3] = a ? (px & 0xff) * 255 / a : 0;
		}
	}

	cairo_surface_destroy(surf);
	*width = w;
	*height = h;
	*size = w * h * 4;
	return buf;
}
