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

struct fill_stack {
	int		*stack;
	unsigned int	pointer;
	unsigned int	size;
};

// flags?

static int fill_stack_pop(struct fill_stack *s, int st, int *x, int *y)
{
	if(s->pointer > 0)
	{
		int p = s->stack[s->pointer];
		*y = p / st;
		*x = p % st;
		s->pointer--;
		return 1;
	}
	else
	{
		return 0;
	}
}

static int fill_stack_push(struct fill_stack *s, int st, int x, int y)
{
	if(s->pointer < s->size - 1)
	{
		s->pointer++;
		s->stack[s->pointer] = x + y * st;
		return 1;
	}
	else
	{
		return 0;
	}
}

void emptyStack(struct fill_stack *s)
{
	int x, y;
	while(pop(x, y));
}

/*
 * Check 8 neighbouring pixels for pixel being on an edge
 *
 * Pixel is on an edge if it has at least one white neighbour
 *
 */
static int validate_edge_px(struct image *q, int x, int y)
{
	int le = x > 0;
	int re = x < q->w;
	int te = y > 0;
	int be = y < q->h;
	int i = x + y * q->w;

	// Skip if already covered?

	if (q->pixels[i] == PIXEL_WHITE)
		return 0;

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

int terrain_fill_seed(struct image *q, int xs, int ys, int bs, int id)
{

	int x, y, i;
	int le, re, te, be;
	int nx, ny;
	int w = q->w;
	struct fill_stack s;

	s.pointer = 0;
	s.size = 1000;
	s.stack = malloc(1000 * sizeof(int));

	fill_stack_push(&s, w, xs, ys);

	while (fill_stack_pop(&s, &x, &y))
	{
		i = x + y * q->w;

		if (q->scratch[i] == id)
			continue;

		q->scratch[i] = id;

		le = p.x > 0;
		re = p.x < q->w;
		te = p.y > 0;
		be = p.y < q->h;

		/* N E S W */
		if (te && validate_edge_px(q, nx = x, ny = y-1))
			fill_stack_push(s, w, nx, ny);
		if (re && validate_edge_px(q, nx = x+1, ny = y))
			fill_stack_push(s, w, nx, ny);
		if (be && validate_edge_px(q, nx = x, ny = y+1))
			fill_stack_push(s, w, nx, ny);
		if (le && validate_edge_px(q, nx = x-1, ny = y))
			fill_stack_push(s, w, nx, ny);

		/* NE SE SW NW */
		if (te && le && validate_edge_px(q, nx = x+1, ny = y-1))
			fill_stack_push(s, w, nx, ny);
		if (re && be && validate_edge_px(q, nx = x+1, ny = y+1))
			fill_stack_push(s, w, nx, ny);
		if (be && le && validate_edge_px(q, nx = x-1, ny = y+1))
			fill_stack_push(s, w, nx, ny);
		if (le && te && validate_edge_px(q, nx = x-1, ny = y-1))
			fill_stack_push(s, w, nx, ny);
	}

	free(s);
}

int main(int argc, char **argv)
{
	#if 0
	unsigned char *imgc = malloc(1000 * 1000);
	memset(imgc, 255, 1000 * 1000);
	circle(imgc, 50, 50, 15);
	pgm_write("/tmp/bcirc.pgm", 100, 100, imgc);
	#endif

	unsigned char *img;
	struct image q;
	int w, h;
	q->pixels = pgm_read(argv[1], q->w, q->h);
	q->scratch = malloc(w * h * sizeof(q->scratch[0]));
	memset(q->scratch, 0, w * h * sizeof(q->scratch[0]));

	terrain_fill_seed(q, 10, 10, 5, 1);

	free(q->pixels);
}
