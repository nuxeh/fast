/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Copyright (C) 2013, OpenCV Foundation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

// Ported to C and made to work with only 1 channel 8-bit unsigned char images

#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <float.h>

#include <opencv/cv.h> 

#include "cornersubpix.h"

#include "pgm.x"

#if 0
static inline int cvFloor(double value)
{
    int i = (int)value;
    return i - (i > value);
}

static inline int cvRound(double value)
{
    return (int)(value + (value >= 0 ? 0.5 : -0.5));
}
#endif

enum { SUBPIX_SHIFT=16 };

static inline int scale_op(float a)
{
	return cvRound(a*(1 << SUBPIX_SHIFT));
}

static inline int cast_op(int a)
{
	return (unsigned char)((a + (1 << (SUBPIX_SHIFT-1))) >> SUBPIX_SHIFT);
}

static const unsigned char* adjustRect(
	const unsigned char *src, size_t src_step, int pix_size,
	Size src_size, Size win_size,
	Point ip, Rect* pRect )
{
    Rect rect;

    if( ip.x >= 0 )
    {
        src += ip.x*pix_size;
        rect.x = 0;
    }
    else
    {
        rect.x = -ip.x;
        if( rect.x > win_size.width )
            rect.x = win_size.width;
    }

    if( ip.x < src_size.width - win_size.width )
        rect.width = win_size.width;
    else
    {
        rect.width = src_size.width - ip.x - 1;
        if( rect.width < 0 )
        {
            src += rect.width*pix_size;
            rect.width = 0;
        }
        assert( rect.width <= win_size.width );
    }

    if( ip.y >= 0 )
    {
        src += ip.y * src_step;
        rect.y = 0;
    }
    else
        rect.y = -ip.y;

    if( ip.y < src_size.height - win_size.height )
        rect.height = win_size.height;
    else
    {
        rect.height = src_size.height - ip.y - 1;
        if( rect.height < 0 )
        {
            src += rect.height*src_step;
            rect.height = 0;
        }
    }

    *pRect = rect;
    return src - rect.x*pix_size;
}

static void getRectSubPix_8u32f
( const uchar* src, size_t src_step, Size src_size,
 float* dst, size_t dst_step, Size win_size, Point2f center0, int cn )
{
    Point2f center = center0;
    Point ip;

    center.x -= (win_size.width-1)*0.5f;
    center.y -= (win_size.height-1)*0.5f;

    ip.x = cvFloor( center.x );
    ip.y = cvFloor( center.y );

    if( cn == 1 &&
       0 <= ip.x && ip.x + win_size.width < src_size.width &&
       0 <= ip.y && ip.y + win_size.height < src_size.height &&
       win_size.width > 0 && win_size.height > 0 )
    {
        float a = center.x - ip.x;
        float b = center.y - ip.y;
        a = MAX(a,0.0001f);
        float a12 = a*(1.f-b);
        float a22 = a*b;
        float b1 = 1.f - b;
        float b2 = b;
        double s = (1. - a)/a;

        src_step /= sizeof(src[0]);
        dst_step /= sizeof(dst[0]);

        // extracted rectangle is totally inside the image
        src += ip.y * src_step + ip.x;

        for( ; win_size.height--; src += src_step, dst += dst_step )
        {
            float prev = (1 - a)*(b1*src[0] + b2*src[src_step]);
            for( int j = 0; j < win_size.width; j++ )
            {
                float t = a12*src[j+1] + a22*src[j+1+src_step];
                dst[j] = prev + t;
                prev = (float)(t*s);
            }
        }
    }
    else
    {
        //getRectSubPix_Cn_<uchar, float, float, nop<float>, nop<float> >
        //(src, src_step, src_size, dst, dst_step, win_size, center0, cn );
	printf("error\n");
    }
}

typedef int _WTp;
typedef unsigned char _Tp;

void getRectSubPix(unsigned char *src, size_t src_step, Size src_size,
                   float *dst, size_t dst_step, Size win_size, Point2f center, int cn)
{
    Point ip;
    _WTp a11, a12, a21, a22, b1, b2;
    float a, b;
    int i, j, c;

    center.x -= (win_size.width-1)*0.5f;
    center.y -= (win_size.height-1)*0.5f;

    ip.x = cvFloor( center.x );
    ip.y = cvFloor( center.y );

    a = center.x - ip.x;
    b = center.y - ip.y;
    a11 = scale_op((1.f-a)*(1.f-b));
    a12 = scale_op(a*(1.f-b));
    a21 = scale_op((1.f-a)*b);
    a22 = scale_op(a*b);
    b1 = scale_op(1.f - b);
    b2 = scale_op(b);

    src_step /= sizeof(src[0]);
    dst_step /= sizeof(dst[0]);

    if( 0 <= ip.x && ip.x < src_size.width - win_size.width &&
       0 <= ip.y && ip.y < src_size.height - win_size.height)
    {
        // extracted rectangle is totally inside the image
        src += ip.y * src_step + ip.x*cn;
        win_size.width *= cn;

        for( i = 0; i < win_size.height; i++, src += src_step, dst += dst_step )
        {
            for( j = 0; j <= win_size.width - 2; j += 2 )
            {
                _WTp s0 = src[j]*a11 + src[j+cn]*a12 + src[j+src_step]*a21 + src[j+src_step+cn]*a22;
                _WTp s1 = src[j+1]*a11 + src[j+cn+1]*a12 + src[j+src_step+1]*a21 + src[j+src_step+cn+1]*a22;
                dst[j] = cast_op(s0);
                dst[j+1] = cast_op(s1);
            }

            for( ; j < win_size.width; j++ )
            {
                _WTp s0 = src[j]*a11 + src[j+cn]*a12 + src[j+src_step]*a21 + src[j+src_step+cn]*a22;
                dst[j] = cast_op(s0);
            }
        }
    }
    else
    {
        Rect r;
        src = (const _Tp*)adjustRect( (const uchar*)src, src_step*sizeof(*src),
                                     sizeof(*src)*cn, src_size, win_size, ip, &r);

        for( i = 0; i < win_size.height; i++, dst += dst_step )
        {
            const _Tp *src2 = src + src_step;
            _WTp s0;

            if( i < r.y || i >= r.height )
                src2 -= src_step;

            for( c = 0; c < cn; c++ )
            {
                s0 = src[r.x*cn + c]*b1 + src2[r.x*cn + c]*b2;
                for( j = 0; j < r.x; j++ )
                    dst[j*cn + c] = cast_op(s0);
                s0 = src[r.width*cn + c]*b1 + src2[r.width*cn + c]*b2;
                for( j = r.width; j < win_size.width; j++ )
                    dst[j*cn + c] = cast_op(s0);
            }

            for( j = r.x*cn; j < r.width*cn; j++ )
            {
                s0 = src[j]*a11 + src[j+cn]*a12 + src2[j]*a21 + src2[j+cn]*a22;
                dst[j] = cast_op(s0);
            }

            if( i < r.height )
                src = src2;
        }
    }
}

#define SPX_EPS 0.000001
#define SPX_ITERS 100

void cornerSubPix(unsigned char *src, int cols, int rows,
		  Point2f *corners, int count, int ww, int wh)
{
    const int MAX_ITERS = 100;
    Size win = {ww, wh};
    int win_w = win.width * 2 + 1, win_h = win.height * 2 + 1;
    printf("win_w=%d win_h=%d\n", win_w, win_h);
    int i, j, k;
    int max_iters = SPX_ITERS;

    double eps = SPX_EPS;
    eps *= eps; // use square of error in comparsion operations

    if( count == 0 )
        return;

    if (win.width <= 0 && win.height <= 0)
	return;

    float *mask = malloc(win_h * win_w * sizeof(float));
    float *subpix_buf = malloc((win_h+2) * (win_w+2) * sizeof(float));

    #ifdef CV_COMPARE
    float *subpix_buf2 = malloc((win_h+2) * (win_w+2) * sizeof(float));
    CvMat cv_mat = cvMat(rows, cols, CV_8U, src);
    CvMat out_mat = cvMat((win_h+2), (win_w+2), CV_32F, subpix_buf2);
    printf("in mat step: %d out mat step: %d\n", cv_mat.step, out_mat.step);
    #endif

    for( i = 0; i < win_h; i++ )
    {
        float y = (float)(i - win.height)/win.height;
        float vy = exp(-y*y);
        for( j = 0; j < win_w; j++ )
        {
            float x = (float)(j - win.width)/win.width;
            mask[i * win_w + j] = (float)(vy*exp(-x*x));
        }
    }

    #if 1
    // make zero_zone (half the window width, in the centre of the window
    for( i = win.height - (win.height/2); i <= win.height + (win.width/2); i++ )
    {
        for( j = win.width - (win.height/2); j <= win.width + (win.width/2); j++ )
        {
            mask[i * win_w + j] = 0;
        }
    }
    #endif

    // do optimization loop for all the points
    for( int pt_i = 0; pt_i < count; pt_i++ )
    {
        Point2f cT = corners[pt_i], cI = cT;
        int iter = 0;
        double err = 0;

        do
        {
            Point2f cI2;
            double a = 0, b = 0, c = 0, bb1 = 0, bb2 = 0;

	    Size ds = {win_w+2, win_h+2};
	    Size is = {cols, rows};
	    /* get subpixel buffer 
	     * destination array is float, so multiply step by 4 */
            getRectSubPix_8u32f(src, is.width, is, subpix_buf, ds.width*4, ds, cI, 1);

	    #ifdef CV_COMPARE
	    CvPoint2D32f cc = cvPoint2D32f(cI.x, cI.y);
            cvGetRectSubPix(&cv_mat, &out_mat, cc);

	    printf("w: %d h: %d\n", (win_w+2), (win_h+2));
	    for (i=0; i < (win_w+2) * (win_h+2); i++)
		    printf("%d\t%f\t%f\n", i, subpix_buf[i], subpix_buf2[i]);
	    printf("%d\n",  (win_w+2) * (win_h+2));
	    #endif

	    const float* subpix = &subpix_buf[1 + ds.width];

            // process gradient
            for( i = 0, k = 0; i < win_h; i++, subpix += win_w + 2 )
            {
                double py = i - win.height;

                for( j = 0; j < win_w; j++, k++ )
                {
                    double m = mask[k];
                    double tgx = subpix[j+1] - subpix[j-1];
                    double tgy = subpix[j+win_w+2] - subpix[j-win_w-2];
                    double gxx = tgx * tgx * m;
                    double gxy = tgx * tgy * m;
                    double gyy = tgy * tgy * m;
                    double px = j - win.width;

                    a += gxx;
                    b += gxy;
                    c += gyy;

                    bb1 += gxx * px + gxy * py;
                    bb2 += gxy * px + gyy * py;
                }
            }

            double det=a*c-b*b;
            if( fabs( det ) <= DBL_EPSILON*DBL_EPSILON )
                break;

            // 2x2 matrix inversion
            double scale=1.0/det;
            cI2.x = (float)(cI.x + c*scale*bb1 - b*scale*bb2);
            cI2.y = (float)(cI.y - b*scale*bb1 + a*scale*bb2);
            err = (cI2.x - cI.x) * (cI2.x - cI.x) + (cI2.y - cI.y) * (cI2.y - cI.y);
            cI = cI2;
            if( cI.x < 0 || cI.x >= cols || cI.y < 0 || cI.y >= rows )
                break;
        }
        while( ++iter < max_iters && err > eps );

        // if new point is too far from initial, it means poor convergence.
        // leave initial point as the result
	printf("corner: x: %f y: %f\n", cI.x, cI.y);
	printf("corner: x: %f y: %f\n", cT.x, cT.y);
        if( fabs( cI.x - cT.x ) > win.width || fabs( cI.y - cT.y ) > win.height )
            cI = cT;

        corners[pt_i] = cI;
    }

    free(mask);
    free(subpix_buf);
}



int main(int argc, char **argv)
{
	unsigned char *img;
	int w, h;
	img = pgm_read(argv[1], &w, &h);
	Point2f c[] = {{3339.0, 856.0}};
	int count = 1;

	cornerSubPix(img, w, h, c, count, 27, 27);

	pgm_write("/tmp/1.pgm", w, h, img);

	printf("corner: x: %f y: %f\n", c[0].x, c[0].y);

	free(img);

}
