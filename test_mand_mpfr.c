/*
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <gmp.h>
#include <mpfr.h>

#define PRECISION 128

struct Color { int r, g, b; };

/* ----------------------------------------------------------------------------
 * sqrt_gradient - first attempt at a procedural color gradient
 *
 * Params
 * it (int) - the iteration value calculated at a point
 * maxiter (int) - max possible iteration value
 * Returns
 * color (struct if 3 ints) - RGB color value
 */
static struct Color sqrt_gradient(unsigned int it, unsigned int maxiter)
{
	struct Color c;
	if (it < maxiter)
	{
		double m = sqrt(sqrt( (double)it / (double)maxiter ));
		/*printf("sqrt(it/max) = %5.2f | ", m);*/
		c.r = (int)floor((( sin(0.65 * m * 85.0) *0.5)+0.5) *255);
		c.g = (int)floor((( sin(0.45 * m * 85.0) *0.5)+0.5) *255);
		c.b = (int)floor((( sin(0.25 * m * 85.0) *0.5)+0.5) *255);
		if (c.r>255 || c.g>255 || c.b>255 || c.r<0 || c.g<0 || c.b<0)
			printf("col(%5.6f,%5.6f,%5.6f) | ",sin(0.30 * m * 20.0),sin(0.45 * m * 20.0),sin(0.65
 * m * 20.0));
	}
	else
	{
		c.r = 0; c.g = 0; c.b = 0;
	}
	return c;
}

/* ----------------------------------------------------------------------------
 * mandlebrot set using mpfr library
 *
 * Params
 * xsize, ysize   - 
 * maxiter        - 
 * Xs, Xe, Ys, Ye - 
 *
 * (out)bytearray - 
 *
 * Return int
 */
int mandlebrot_set_mpfr(const unsigned int xsize,   /* width of screen/display/window */
                        const unsigned int ysize,   /* height of screen/display/window */
                        const unsigned int maxiter, /* max iterations before escape */
                        const char* Xs_str, /* string repr of mpfr_t for X top left */
                        const char* Xe_str, /* string repr of mpfr_t for X top right */
                        const char* Ys_str, /* string repr of mpfr_t for Y bottom left */
                        const char* Ye_str, /* string repr of mpfr_t for Y bottom right */
                        int bytearray[] /* reference/pointer to result list of color values*/
                        )
{
    mpfr_t x, y, xsq, ysq, xtmp, x0, y0, Xe, Xs, Ye, Ys; /* algorithm values */
    mpfr_t a, two, four, sum_xsq_ysq;                    /* tmp vals */
    unsigned int iteration = 0;
    unsigned int bc = 0;
    struct Color rgb;

    /*for (int z = 0; z<100; z++) { printf("%d %d ",z,bytearray[z]); }
    printf("");*/

    /* create all mpfr_t vars */
    mpfr_inits2(PRECISION, x, y, xsq, ysq, xtmp, x0, y0, Xs, Xe, Ys, Ye, (mpfr_ptr)NULL); 
    mpfr_inits2(PRECISION, a, two, four, sum_xsq_ysq, (mpfr_ptr)NULL);

    /* get hight precision floats from input strings */
    mpfr_strtofr(Xs, Xs_str, NULL, 10, MPFR_RNDD);
    mpfr_strtofr(Xe, Xe_str, NULL, 10, MPFR_RNDD);
    mpfr_strtofr(Ys, Ys_str, NULL, 10, MPFR_RNDD);
    mpfr_strtofr(Ye, Ye_str, NULL, 10, MPFR_RNDD);

    /* initialise all mpfr_t vars */
    mpfr_set_d(x, 0.0, MPFR_RNDD);
    mpfr_set_d(y, 0.0, MPFR_RNDD);
    mpfr_set_d(xsq, 0.0, MPFR_RNDD);
    mpfr_set_d(ysq, 0.0, MPFR_RNDD);
    mpfr_set_d(xtmp, 0.0, MPFR_RNDD);
    mpfr_set_d(x0, 0.0, MPFR_RNDD);
    mpfr_set_d(y0, 0.0, MPFR_RNDD);
    mpfr_set_d(a, 0.0, MPFR_RNDD);
    mpfr_set_d(two, 2.0, MPFR_RNDD);
    mpfr_set_d(four, 4.0, MPFR_RNDD);
    mpfr_set_d(sum_xsq_ysq, 0.0, MPFR_RNDD);

    for (unsigned int Dy = 0; Dy < ysize; Dy++)
    {
        for (unsigned int Dx = 0; Dx < xsize; Dx++)
        {
            iteration = 0;

			/* double x0 = scaled(Dx, xsize, Xs, Xe); */
            mpfr_sub(a, Xe, Xs, MPFR_RNDD);
            mpfr_mul_d(a, a, ((double)Dx/(double)xsize), MPFR_RNDD);
            mpfr_add(x0, a, Xs, MPFR_RNDD);

			/* double y0 = scaled(Dy, ysize, Ys, Ye); */
            mpfr_sub(a, Ye, Ys, MPFR_RNDD);
            mpfr_mul_d(a, a, ((double)Dy/(double)ysize), MPFR_RNDD);
            mpfr_add(y0, a, Ys, MPFR_RNDD);

            /* reset some vars for each pixel */
            mpfr_set_d(x, 0.0, MPFR_RNDD);
            mpfr_set_d(y, 0.0, MPFR_RNDD);

            /* while (xsq+ysq <= 4 && iteration < maxiter */
            while ((mpfr_cmp(sum_xsq_ysq, four) <= 0) && (iteration < maxiter))  
            {
		        mpfr_mul(xsq, x, x, MPFR_RNDD);      /* xsq = x*x; */
		        mpfr_mul(ysq, y, y, MPFR_RNDD);      /* ysq = y*y; */
		        mpfr_sub(xtmp, xsq, ysq, MPFR_RNDD); /* xtemp = xsq - ysq + x0; */
                mpfr_add(xtmp, xtmp, x0, MPFR_RNDD);
		        mpfr_mul(a, two, x, MPFR_RNDD);      /* y = 2.0*x*y + y0; */
                mpfr_mul(a, a, y, MPFR_RNDD);
                mpfr_add(y, a, y0, MPFR_RNDD);
		        mpfr_swap(x, xtmp);                  /* x = xtemp; */

                /* calcs for while test */
                mpfr_add(sum_xsq_ysq, xsq, ysq, MPFR_RNDD);
		        iteration++;
            }
            /* create a color value and add to result list */
            rgb = sqrt_gradient(iteration, maxiter);
            //printf("bc %d r %d\n", bc, rgb.r);
            bytearray[bc] = rgb.r;
            bc++;
            //printf("bc %d g %d\n", bc, rgb.g);
            bytearray[bc] = rgb.g;
            bc++;
            //printf("bc %d b %d\n", bc, rgb.b);
            bytearray[bc] = rgb.b;
            bc++;
        }
    }
}

/*
 *
 */
int main()
{
    unsigned int xsize, ysize, maxiter;
    //int *bytearray;
    char *Xs_str;
    char *Xe_str;
    char *Ys_str;
    char *Ye_str;
    mpfr_t Xs, Xe, Ys, Ye;
    mpfr_exp_t exp;


    /*mpfr_inits2(PRECISION, Xs, Xe, Ys, Ye);*/
    mpfr_init2(Xs, PRECISION);
    mpfr_init2(Xe, PRECISION);
    mpfr_init2(Ys, PRECISION);
    mpfr_init2(Ye, PRECISION);
    /* 1.768901153038437, -0.05747430607697883 */
    mpfr_set_d(Xs, -1.768901153038436, MPFR_RNDD);
    mpfr_set_d(Xe, -1.768901153038438, MPFR_RNDD);
    mpfr_set_d(Ys, -0.05747430607697882, MPFR_RNDD);
    mpfr_set_d(Ye, -0.05747430607697884, MPFR_RNDD);

    printf("ndigits %d\n", mpfr_get_str_ndigits(10, PRECISION));

    Xs_str = mpfr_get_str(NULL, &exp, 10, 0, Xs, MPFR_RNDD);
    Xe_str = mpfr_get_str(NULL, &exp, 10, 0, Xe, MPFR_RNDD);
    Ys_str = mpfr_get_str(NULL, &exp, 10, 0, Ys, MPFR_RNDD);
    Ye_str = mpfr_get_str(NULL, &exp, 10, 0, Ye, MPFR_RNDD);

    printf("str %s %s %s %s \n", Xs_str, Xe_str, Ys_str, Ye_str);

    xsize = 640;
    ysize = 640;
    maxiter = 1000;

    printf("create array size %d\n", (3*xsize*ysize));
    //bytearray = (int*)calloc((size_t)(3*xsize*ysize), sizeof(int));
    int bytearray[3*xsize*ysize];

    //printf("bytearray size %ld\n", sizeof(bytearray));
    /*int bc = 0;
    bytearray[bc] = 1; bc++;
    bytearray[bc] = 2; bc++;
    bytearray[bc] = 3; bc++;
    bytearray[bc] = 4; bc++;
    bytearray[bc] = 5; bc++;*/
    /*for (int z = 0; z<100; z++) { printf("%d %d ",z,bytearray[z]); }
    printf("");*/

    mandlebrot_set_mpfr(xsize, ysize, maxiter, Xs_str, Xe_str, Ys_str, Ye_str, bytearray);
}

