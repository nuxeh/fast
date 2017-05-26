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
	struct quirc_point	*segments;
	int 			r;
	int			l;
	int			ns;
};

/* Bressenham circle */
struct circle * get_circle(int r)
{
	int d, x, y, p, i, l;
	struct circle *c = malloc(sizeof(struct circle));
	memset(c, 0, sizeof(struct circle));

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

	return c;
}

int get_circle_segments(struct circle *c, int n)
{
	int e, i;
	float k = (float) c->l / n;

	c->segments = malloc(n * sizeof(struct quirc_point));

	for (e = 0; e < n; e++) {
		i = roundf(k * e);
		c->segments[e] = c->array[i];
	}

	c->ns = n;
}

void destroy_circle(struct circle *c)
{
	free(c->array);
	free(c->segments);
	free(c);
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

static int fill_stack_pop_i(struct fill_stack *s, int *i)
{
	if(s->pointer > 0)
	{
		*i = s->stack[s->pointer];
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

static int fill_stack_push_i(struct fill_stack *s, int i)
{
	if(s->pointer < s->size - 1)
	{
		s->pointer++;
		s->stack[s->pointer] = i;
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

struct fill_stack * get_stack(int max_size)
{
	struct fill_stack *s = malloc(sizeof(struct fill_stack));
	memset(s, 0, sizeof(struct fill_stack));

	s->size = max_size;
	s->stack = malloc(max_size * sizeof(int));

	return s;
}

void destroy_stack(struct fill_stack *s)
{
	free(s->stack);
	free(s);
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

/*
 * Use a FAST-like method for sampling potential corners
 *
 *  q   - image structure
 *  c   - circle used for sampling
 *  w   - image width
 *  i   - index of centre pixel
 *  min - minimum number of segments needed for corner
 *
 */
#define CORNER_PLACEHOLDER 255
int fast_sample(struct image *q, struct fill_stack *s,
		struct circle *c, int i, int thresh)
{
	int j;
	int count = 0;
	struct quirc_point *o = c->segments;

	for (j = 0; j < c->ns; j++)
	{
		if (q->pixels[i + o[j].x + q->w * o[j].y] == PIXEL_WHITE)
			count++;
	}

	if (count > thresh || c->ns - count > thresh + 1)
	{
		q->scratch[i] = CORNER_PLACEHOLDER;
		fill_stack_push_i(s, i);
	}
}

int fast_detect()
{

}

int corner_area_scan(struct image *q)
{
	for(int y=-radius; y<=radius; y++)
	    for(int x=-radius; x<=radius; x++)
		if(x*x+y*y <= radius*radius)
		    setpixel(origin.x+x, origin.y+y);
}

int process_corners(struct image *q, struct fill_stack *s)
{
	int i;

	while (fill_stack_pop_i(s, &i))
	{
		q->scratch[i] = 30;
	}

}

int terrain_fill_seed(struct image *q, int xs, int ys, int bs, int id)
{
	int x, y, i;
	int le, re, te, be;
	int nx, ny;
	int w = q->w;

	struct fill_stack *s = get_stack(10000);
	struct fill_stack *cs = get_stack(10000);

	struct circle *cc = get_circle(bs/2);
	struct circle *cf = get_circle((int) roundf((float) bs * 1.5));

	fill_stack_push(s, w, xs, ys);

	get_circle_segments(cc, 16);
	int c_thresh = (cc->ns / 2) + 2;

	int e;
	for (e = 0; e<8; e++)
		printf("%d,%d\n", cc->segments[e].x, cc->segments[e].y);

	while (fill_stack_pop(s, w, &x, &y))
	{
		i = x + y * q->w;

		if (q->scratch[i] == id)
			continue;

		if (q->scratch[i] == CORNER_PLACEHOLDER)
			continue;

		q->scratch[i] = id;

		fast_sample(q, cs, cc, i, c_thresh);

		le = x > 0;
		re = x < q->w;
		te = y > 0;
		be = y < q->h;

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

#ifdef DEBUG_FILL_STACK
	printf("max stack depth: %d\n", s->max_depth);
#endif
	destroy_stack(s);

	process_corners(q, cs);

#ifdef DEBUG_FILL_STACK
	printf("max stack depth: %d\n", cs->max_depth);
#endif
	destroy_stack(cs);

	destroy_circle(cc);
	destroy_circle(cf);
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

	terrain_fill_seed(&q, 240, 200, 20, 128);
	terrain_fill_seed(&q, 81, 81, 20, 128);
	terrain_fill_seed(&q, 120, 120, 20, 128);

	pgm_write("/tmp/ff.pgm", q.w, q.h, q.scratch);

	free(q.pixels);
	free(q.scratch);
}
