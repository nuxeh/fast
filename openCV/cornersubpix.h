/* Various structs required for OpenCV, converted from C++ classes */

typedef struct Rect {
	int x;
	int y;
	int width;
	int height;
} Rect;

typedef struct Point {
	int x;
	int y;
} Point;

typedef struct Point2f {
	float x;
	float y;
} Point2f;

typedef struct Size {
	int width;
	int height;
} Size;

typedef unsigned char uchar;

#define MAX(a,b)  ((a) < (b) ? (b) : (a))
