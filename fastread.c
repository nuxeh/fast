#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "pgm.x"

#define PIXEL_WHITE 255
#define PIXEL_BLACK 0

struct image {
	unsigned char 	*pixels;
	unsigned char	*scratch;
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

struct circle {
	struct quirc_point	*array;
	int 			r;
	int			l;
};

/* Bressenham circle */
void get_circle(struct circle *c, int r)
{
	int d, x, y, p, i, l;

	int ii = 0;

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
	c->array = malloc(l * sizeof(struct quirc_point));
	memset(c->array, 0, l * sizeof(struct quirc_point));

	d = 3 - 2 * r;
	x = 0;
	y = r;
	while (x <= y)
	{
		set_point(c->array, l, x,	  +x, -y);
		set_point(c->array, l, (2*p)-x-2, +y, -x);
		set_point(c->array, l, (2*p)+x-2, +y, +x);
		set_point(c->array, l, (4*p)-x-4, +x, +y);
		set_point(c->array, l, (4*p)+x-4, -x, +y);
		set_point(c->array, l, (6*p)-x-6, -y, +x);
		set_point(c->array, l, (6*p)+x-6, -y, -x);
		set_point(c->array, l, (8*p)-x-8, -x, -y);

		if(d <= 0) {
			d = d + 4 * x + 6;
		} else {
			d = d + 4 * x - 4 * y + 10;
			y = y - 1;
		}
		x++;
	}

	c->r = r;
	c->l = l;
}

#define DEBUG_FILL_STACK

struct fill_stack {
	int		*stack;
	unsigned int	pointer;
	unsigned int	size;
#ifdef DEBUG_FILL_STACK
	unsigned int	max_depth;
#endif
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
#ifdef DEBUG_FILL_STACK
		if (s->pointer > s->max_depth) s->max_depth = s->pointer;
#endif
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

	if (q->scratch[i] != 0)
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

	struct circle cc;
	struct circle cf;

	s.pointer = 0;
	s.size = 10000;
	s.stack = malloc(10000 * sizeof(int));
#ifdef DEBUG_FILL_STACK
	s.max_depth = 0;
#endif
	fill_stack_push(&s, w, xs, ys);

	get_circle(&cc, bs);
	get_circle(&cf, (bs * 15) / 10);
	printf("%d\n", cc.l);
	printf("%d\n", cf.l);

	int e;
	float k = (float) cc.l / 16;
	for (e = 0; e < cc.l; e++)
		printf("#");
	printf("\n");
	printf("%f\n", k);

	int qu[16] = {0};

	for (e = 0; e < 16; e++) {
		int q = roundf(k * e);
		printf("%d\n", q);
		qu[e] = q;
	}

	for (e = 0; e < cc.l; e++) {
		int f;
		int found = 0;
		for (f = 0; f < 16; f++) {
			if (e == qu[f]) {
				printf("#");
				found = 1;
				break;
			}
		}
		if (!found)
			printf("_");
	}
	printf("\n");

	while (fill_stack_pop(&s, w, &x, &y))
	{
		i = x + y * q->w;

		if (q->scratch[i] == id)
			continue;

		q->scratch[i] = id;

		le = x > 0;
		re = x < q->w;
		te = y > 0;
		be = y < q->h;

		/* N E S W */
		if (te && validate_edge_px(q, nx = x, ny = y-1))
			fill_stack_push(&s, w, nx, ny);
		if (re && validate_edge_px(q, nx = x+1, ny = y))
			fill_stack_push(&s, w, nx, ny);
		if (be && validate_edge_px(q, nx = x, ny = y+1))
			fill_stack_push(&s, w, nx, ny);
		if (le && validate_edge_px(q, nx = x-1, ny = y))
			fill_stack_push(&s, w, nx, ny);

		/* NE SE SW NW */
		if (te && le && validate_edge_px(q, nx = x+1, ny = y-1))
			fill_stack_push(&s, w, nx, ny);
		if (re && be && validate_edge_px(q, nx = x+1, ny = y+1))
			fill_stack_push(&s, w, nx, ny);
		if (be && le && validate_edge_px(q, nx = x-1, ny = y+1))
			fill_stack_push(&s, w, nx, ny);
		if (le && te && validate_edge_px(q, nx = x-1, ny = y-1))
			fill_stack_push(&s, w, nx, ny);
	}

#ifdef DEBUG_FILL_STACK
	printf("max stack depth: %d\n", s.max_depth);
#endif
	free(s.stack);
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

	q.pixels = pgm_read(argv[1], &q.w, &q.h);
	q.scratch = malloc(q.w * q.h * sizeof(q.scratch[0]));
	memset(q.scratch, 0, q.w * q.h * sizeof(q.scratch[0]));

	terrain_fill_seed(&q, 81, 81, 5, 128);

	pgm_write("/tmp/ff.pgm", q.w, q.h, q.scratch);

	free(q.pixels);
	free(q.scratch);
}
