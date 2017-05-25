#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pgm.x"

#define PIXEL_WHITE 255
#define PIXEL_BLACK 0

struct image {
	unsigned char 	*pixels;
	unsigned cha	*scratch;
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

static inline int validate_edge_px(struct image *q, struct quirc_point p)
{
	int le = p.x > 0;
	int re = p.x < q->w;
	int te = p.y > 0;
	int be = p.y < q->h;
	int i = p.x + p.y * q->w;

	/* N E S W */
	if (te && q->pixels[i - q->w] == PIXEL_WHITE)
		return 1;
	if (re && q->pixels[i + 1]    == PIXEL_WHITE)
		return 1;
	if (be && q->pixels[i + q->w] == PIXEL_WHITE)
		return 1;
	if (le && q->pixels[i - 1]    == PIXEL_WHITE)
		return 1;

	/* NE SE SW NW */
	if (te && re && q->pixels[i - q->w + 1] == PIXEL_WHITE)
		return 1;
	if (re && be && q->pixels[i + 1 + q->w] == PIXEL_WHITE)
		return 1;
	if (be && le && q->pixels[i + q->w - 1] == PIXEL_WHITE)
		return 1;
	if (le && te && q->pixels[i - 1 - q->w] == PIXEL_WHITE)
		return 1;

	return 0;
}

int terrain_fill_seed(struct image *q, int x, int y, int bs, int id)
{

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
