#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pgm.x"

#define PIXEL_WHITE 255
#define PIXEL_BLACK 0

struct image {
	unsigned char 	*pixels;
	int		w;
	int		h;
};

struct quirc_point {
	int x;
	int y;
};

struct octant {
	int r;
	int n;
	struct quirc_point *ds[];
};

static inline void set_point(struct quirc_point *p, int m, int i, int x, int y)
{
	if (i >= m)
		return;
	p[i].x = x;
	p[i].y = y;
}

/* Bressenham circle */
void circle(unsigned char *img, int xc, int yc, int r, int *al)
{
	int d, x, y, p, i, l;

	int ii = 0;
	struct quirc_point *c_pixels;

	d = 3 - 2 * r;
	x = 0;
	y = r;
	while(x <= y)
	{
		if(d <= 0) {
			d = d + 4 * x + 6;
		} else {
			d = d + 4 * x - 4 * y + 10;
			y = y - 1;
		}
		x++;
	}

	p = x;
	l = (p-1) * 8;
	c_pixels = malloc(l * sizeof(struct quirc_point));
	memset(c_pixels, 0, l * sizeof(struct quirc_point));

	printf("x: %d\n", p);

	d = 3 - 2 * r;
	x = 0;
	y = r;
	while (x <= y)
	{
		set_point(c_pixels, l, x,	  +x, -y);
		set_point(c_pixels, l, (2*p)-x-2, +y, -x);
		set_point(c_pixels, l, (2*p)+x-2, +y, +x);
		set_point(c_pixels, l, (4*p)-x-4, +x, +y);
		set_point(c_pixels, l, (4*p)+x-4, -x, +y);
		set_point(c_pixels, l, (6*p)-x-6, -y, +x);
		set_point(c_pixels, l, (6*p)+x-6, -y, -x);
		set_point(c_pixels, l, (8*p)-x-8, -x, -y);

		if(d <= 0) {
			d = d + 4 * x + 6;
		} else {
			d = d + 4 * x - 4 * y + 10;
			y = y - 1;
		}
		x++;
	}

	*al = l;
	return c_pixels;
}

static inline int val_edge_px(struct image *q, struct quirc_point p)
{
	int i = p.x + p.y * q->w;

	if (p.x > 0 && q->pixels[i - 1] == PIXEL_WHITE)
		return 0;
	if (p.x < q->w && q->pixels[i + 1] == PIXEL_WHITE)
		return 0;
	if (p.y > 0 && q->pixels[i - q->w] == PIXEL_WHITE)
		return 0;
	if (p.y < q->h && q->pixels[i + q->w] == PIXEL_WHITE)
		return 0;

	return -1;
}

int main(void)
{
	unsigned char *imgc = malloc(1000 * 1000);
	memset(imgc, 255, 1000 * 1000);

	circle(imgc, 50, 50, 15);

	pgm_write("/tmp/bcirc.pgm", 100, 100, imgc);


	unsigned char *img;
	int w, h;

	pgm_write("/tmp/1.pgm", w, h, img);

	free(img);
}
