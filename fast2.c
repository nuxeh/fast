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

	img[x + y * 100] = 0;

	count++;
	printf("count: %d\n", count);
}

struct xy {
	int x;
	int y;
}

struct octant {
	int r;
	struct n;
	struct xy *ds[];
}

/* Bressenham circle */
void circle(unsigned char *img, int xc, int yc, int r)
{
	int d, x, y;

	d = 3 - 2 * r;
	x = 0;
	y = r;

	int i = 0;

	while(x <= y)
	{
		//putpixel(img, xc + x, yc + y);
		//putpixel(img, xc - y, yc - x);
		putpixel(img, xc + y, yc - x);
		//putpixel(img, xc - y, yc + x);
		//putpixel(img, xc + y, yc + x);
		//putpixel(img, xc - x, yc - y);
		//putpixel(img, xc + x, yc - y);
		//putpixel(img, xc - x, yc + y);

		#if 1
		char out_str[20];
		sprintf(out_str, "/tmp/bcirc_%03d.pgm\0", i++);
		printf("%s\n", out_str);
		pgm_write(out_str, 100, 100, img);
		#endif

		printf("x: %d y: %d\n", x, y);

		if(d<=0)
		{
			d = d + 4 * x + 6;
		}
		else
		{
			d = d + 4 * x - 4 * y + 10;
			y = y - 1;
		}

		x = x + 1;
	}
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
	circle(img, 50, 50, 15);

	pgm_write("/tmp/bcirc.pgm", 100, 100, img);
}
