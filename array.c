#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void pgm_write(const char *filename,
	       int width, int height, unsigned char *buf)
{
	FILE *f = fopen(filename,"wb");
	if(f) {
		fprintf(f,"P5\n%d %d\n255\n",width,height);
		fwrite(buf,width,height,f);
		fclose(f);
	}
}

void putpixel(unsigned char *img, int x, int y)
{
	static int count = 0;

	img[x + y * 100] ^= 255;

	count++;
	//printf("count: %d\n", count);
}

struct xy {
	int x;
	int y;
};

struct octant {
	int r;
	int n;
	struct xy *ds[];
};

static inline void set_point(struct xy *p, int x, int y)
{
	p->x = x;
	p->y = y;
}

/* Bressenham circle */
void circle(unsigned char *img, int xc, int yc, int r)
{
	int d, x, y, p, i;

	int ii = 0;
	struct xy *c_pixels;

	d = 3 - 2 * r;
	x = 0;
	y = r;
	while(x <= y)
	{
		#if 1
		putpixel(img, xc + x, yc + y);
		putpixel(img, xc - y, yc - x);
		putpixel(img, xc + y, yc - x);
		putpixel(img, xc - y, yc + x);
		putpixel(img, xc + y, yc + x);
		putpixel(img, xc - x, yc - y);
		putpixel(img, xc + x, yc - y);
		putpixel(img, xc - x, yc + y);
		#endif
		#if 0
		char out_str[20];
		sprintf(out_str, "/tmp/bcirc2__%03d.pgm\0", ii++);
		printf("%s\n", out_str);
		pgm_write(out_str, 100, 100, img);
		#endif

		printf("x: %d y: %d\n", x, y);

		if(d <= 0) {
			d = d + 4 * x + 6;
		} else {
			d = d + 4 * x - 4 * y + 10;
			y = y - 1;
		}
		x++;
	}

	p = x;
	c_pixels = malloc(p * 8 * sizeof(struct xy));
	memset(c_pixels, 0, p * 8 * sizeof(struct xy));

	printf("x: %d\n", p);

	d = 3 - 2 * r;
	x = 0;
	y = r;
	while (x <= y)
	{
		set_point(&c_pixels[x],       +x, -y);
		set_point(&c_pixels[(2*p)-x-1], +y, -x);
		set_point(&c_pixels[(2*p)+x-1],   +y, +x);
		set_point(&c_pixels[(4*p)-x-2], +x, +y);
		set_point(&c_pixels[(4*p)+x-2],   -x, +y);
		set_point(&c_pixels[(6*p)-x-3], -y, +x);
		set_point(&c_pixels[(6*p)+x-3],   -y, -x);
		set_point(&c_pixels[(8*p)-x-4], -x, -y);

		printf("x: %d y: %d\n", x, y);

		if(d <= 0) {
			d = d + 4 * x + 6;
		} else {
			d = d + 4 * x - 4 * y + 10;
			y = y - 1;
		}
		x++;


		for (i = 0; i < p * 8; i++) {
			if (c_pixels[i].x == 0 && c_pixels[i].y == 0)
				printf(" ");
			else
				printf("#");
		}
		printf("\n");
	}
	#if 1
	//unsigned char *img2 = malloc(100 * 100);
	//memset(img2, 255, 1000 * 1000);
	for (i = 0; i < p * 8; i++) {
		putpixel(img, xc + c_pixels[i].x + 10, yc + c_pixels[i].y);
		#if 1
		char out_str[20];
		sprintf(out_str, "/tmp/bcirc2_%03d.pgm\0", ii++);
		printf("%s\n", out_str);
		pgm_write(out_str, 100, 100, img);
		#endif
	}
	//free(img2);
	#endif
}

int main(void)
{
	unsigned char *img = malloc(1000 * 1000);
	memset(img, 255, 1000 * 1000);

//	circle(img, 500, 500, 100);
//	circle(img, 500, 500, 20);
//	circle(img, 500, 500, 10);
//	circle(img, 500, 500, 5);
//	circle(img, 500, 500, 2);
//	circle(img, 500, 500, 3);
	circle(img, 25, 75, 11);

	pgm_write("/tmp/bcirc.pgm", 100, 100, img);
}
