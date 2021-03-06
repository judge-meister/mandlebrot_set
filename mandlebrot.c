/* ----------------------------------------------------------------------------
 *  C library with python module interface to calculate the Mandlebrot set for
 *  the area provided by a set of coordinates
 */

/* ----------------------------------------------------------------------------
 * Improvements
 *
 * - make MAXITER an input parameter so we can decide from within python what the
 * escape value should be.  Will also allow us to create the "MandleBhuda".
 * - remove the PyList.Append() calls from the main loop so we can Releasing the
 * GIL and start to look at threading.
 */

/* ----------------------------------------------------------------------------
 * from wikipedia (https://en.wikipedia.org/wiki/Mandelbrot_set)
 * pseudo code to generate the mandlebrot set
 *
 * for each pixel (Px, Py) on the screen do
 *     x0 := scaled x coordinate of pixel (scaled to lie in the Mandelbrot X scale (-2.00, 0.47))
 *     y0 := scaled y coordinate of pixel (scaled to lie in the Mandelbrot Y scale (-1.12, 1.12))
 *     x := 0.0
 *     y := 0.0
 *     iteration := 0
 *     max_iteration := 1000
 *     while (x*x + y*y ≤ 2*2 AND iteration < max_iteration) do
 *         xtemp := x*x - y*y + x0
 *         y := 2*x*y + y0
 *         x := xtemp
 *         iteration := iteration + 1
 *
 *     color := palette[iteration]
 *     plot(Px, Py, color)
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <gmp.h>
#include <mpfr.h>

#include "mandlebrot.h"

/* DEFINES */

//#define TRACE 1
// TRACE DEBUG macro
#ifdef TRACE
#define TRACE_DEBUGV(formatstring, ...) \
    { printf("-D- " formatstring, __VA_ARGS__); }
#define TRACE_DEBUG(formatstring) \
	{ printf("-D- " formatstring); }
#else
#define TRACE_DEBUGV(formatstring, ...)
#define TRACE_DEBUG(formatstring)
#endif


#define PRECISION 128
/* if PRECISION is increased then NUMLEN needs to increase */
#define NUMLEN 50

#define MAXITER 1000

/* TYPE DEFS */
struct Color { int r, g, b; };

/* STATIC VARIABLES */
static mpfr_t Xe, Xs, Ye, Ys; /* algorithm values */
static int zoom_level;

/* ----------------------------------------------------------------------------
 * grayscale - return a gray scale rgb value representing the iteration count
 *             value obtained by calculate_point
 *
 * Params
 * iteration (int) - interation value calculated at a point
 * Return
 * color (struct of 3 int vals) - RGB color value
 */
static struct Color grayscale(int it, int maxiter)
{
    struct Color c; /*  = { 0, 0, 0 } */
    c.r = 0;
    c.g = 0;
    c.b = 0;

    if (it < maxiter)
    {
        /*int idx = (int)ceil( sqrt( sqrt( (double)it / (double)MAXITER ) ) * 255.0 );*/
        int idx = (int)ceil( sqrt( (double)it / (double)maxiter ) * 255.0 );
        c.r = idx;
        c.g = idx;
        c.b = idx;
    }
    return c;
}

/* ----------------------------------------------------------------------------
 * sqrt_gradient - first attempt at a procedural color gradient
 *
 * Params
 * it (int) - the iteration value calculated at a point
 * Returns
 * color (struct if 3 ints) - RGB color value
 */
static struct Color sqrt_gradient(int it, int maxiter)
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
            TRACE_DEBUGV("col(%5.6f,%5.6f,%5.6f) | ",sin(0.30 * m * 20.0),sin(0.45 * m * 20.0),sin(0.65 * m * 20.0));
    }
    else
    {
        c.r = 0; c.g = 0; c.b = 0;
    }
    return c;
}

/* ----------------------------------------------------------------------------
 * scaled - return a value in the range Xe to Xs given a location in a larger
 *          integer range
 * Params
 * x1 (int) - position in large integer range
 * sz (int) - size of integer range
 * Xs (double) - start of smaller range
 * Xe (double) - end of smaller range
 * Returns
 * (double) - the calculated scaled value
 */
/*static double scaled(int x1, int sz, double Xs, double Xe)
{
    return ( ((double)x1 / (double)sz) * (Xe-Xs) ) + Xs;
}*/

/* ----------------------------------------------------------------------------
 * calculate_point
 * run the z = z^2 + c algorithm for the point provided to assertain if the
 * point is inside or outside of the mandlebrot set
 *
 * Params
 * x0, y0 (double) location to calculate for
 * maxiter (int) the escape value
 *
 * Returns
 * iteration - count of how many iterations it took to head to infinity
 */
static int calculate_point(double x0, double y0, int maxiter)
{
    double x = 0.0;
    double y = 0.0;
    double xsq = 0.0;
    double ysq = 0.0;
    int iteration = 0;
    double xtemp = 0.0;

    while (((xsq + ysq) <= 2*2) && iteration < maxiter)
    {
        xsq = x*x;
        ysq = y*y;
        xtemp = xsq - ysq + x0;
        y = 2.0*x*y + y0;
        x = xtemp;
        iteration++;
    }
    return iteration;
}

/* ----------------------------------------------------------------------------
 * mandlebrot_bytearray - return the results as a list of color values to be
 *                        converted to a bytesarray by the caller
 *
 * Params (in)
 * wsize (int) - width of display screen/window
 * hsize (int) - height of display screen/window
 * maxiter (int) - escape value for mandlebrot calc
 * Xs, Xe, Ys, Ye (double) bounds of mandlebrot set to calculate
 * Params (out)
 * bytearray (array of ints) for storing color values of calculated points
 */
void mandlebrot_bytearray_c(const unsigned int wsize,   /* width of screen/display/window */
                            const unsigned int hsize,   /* height of screen/display/window */
                            const unsigned int maxiter, /* max iterations before escape */
                            const double Xs, /* string repr of mpfr_t for X top left */
                            const double Xe, /* string repr of mpfr_t for X top right */
                            const double Ys, /* string repr of mpfr_t for Y bottom left */
                            const double Ye, /* string repr of mpfr_t for Y bottom right */
                            int bytearray[]  /* reference/pointer to result list of color values */
                            )
{
    unsigned int bc = 0;
    double xstep = (Xe-Xs) / (float)wsize;
    double ystep = (Ye-Ys) / (float)hsize;

    for(unsigned int Dy = 0; Dy < hsize; Dy++)
    {
        for(unsigned int Dx = 0; Dx < wsize; Dx++)
        {
            double x0 = Dx*xstep + Xs;
            double y0 = Dy*ystep + Ys;
            struct Color rgb;
            int iter;

            iter = calculate_point(x0, y0, maxiter);

            /*rgb = grayscale(iter, maxiter);*/
            rgb = sqrt_gradient(iter, maxiter);

            /* just add the rgb values to the list */
            bytearray[bc] = rgb.r;
            bc++;

            bytearray[bc] = rgb.g;
            bc++;

            bytearray[bc] = rgb.b;
            bc++;
        }
    }
}

/* ----------------------------------------------------------------------------
 * setup_c - create the corner variables ar mpfr types
 */
void setup_c()
{
    /* create all mpfr_t vars */
    mpfr_inits2(PRECISION, Xs, Xe, Ys, Ye, (mpfr_ptr)NULL);
    TRACE_DEBUG("called setup_c()\n");
}

/* ----------------------------------------------------------------------------
 * initialise_c - set initial values for the image corners
 */
void initialize_c(
       const char* Xs_str, /* string repr of mpfr_t for X top left */
       const char* Xe_str, /* string repr of mpfr_t for X top right */
       const char* Ys_str, /* string repr of mpfr_t for Y bottom left */
       const char* Ye_str  /* string repr of mpfr_t for Y bottom right */
     )
{
    TRACE_DEBUG("called init_c()\n");
    mpfr_set_str(Xs, Xs_str, 10, MPFR_RNDN);
    mpfr_set_str(Xe, Xe_str, 10, MPFR_RNDN);
    mpfr_set_str(Ys, Ys_str, 10, MPFR_RNDN);
    mpfr_set_str(Ye, Ye_str, 10, MPFR_RNDN);
    zoom_level = 0;

}
/* ----------------------------------------------------------------------------
 * free_mpfr_mem_c - free the memory created in setup_c
 */
void free_mpfr_mem_c()
{
    mpfr_clears(Xs, Xe, Ys, Ye, (mpfr_ptr)NULL);
    mpfr_free_cache();
}

/* ----------------------------------------------------------------------------
 * mandlebrot set using mpfr library
 *
 * Params
 * xsize, ysize   -
 * maxiter        -
 *
 * (out)bytearray -
 *
 */
void mandlebrot_mpfr_c(  const unsigned int xsize,   /* width of screen/display/window */
                         const unsigned int ysize,   /* height of screen/display/window */
                         const unsigned int maxiter, /* max iterations before escape */
                         int bytearray[] /* reference/pointer to result list of color values*/
                         )
{
	mandlebrot_mpfr_slice_c(xsize, ysize, 1, 0, maxiter, bytearray);
}

/* ----------------------------------------------------------------------------
 * mandlebrot set slices using mpfr library
 *
 * this function allows for splitting the image up for multiprocessing support
 *
 * Params
 * xsize, ysize   -
 * nslice, slice
 * maxiter        -
 *
 * (out)bytearray -
 */
void mandlebrot_mpfr_slice_c( const unsigned int xsize,   /* width of screen/display/window */
                              const unsigned int ysize,   /* height of screen/display/window */
                              const unsigned int nslice,  /* number of slices */
                              const unsigned int slice,   /* which slice (range 0 -> nslice-1) */
                              const unsigned int maxiter, /* max iterations before escape */
                              int bytearray[] /* reference/pointer to result list of color values*/
                             )
{
    mpfr_t x, y, xsq, ysq, xtmp, x0, y0, ys1, ye1;  /* algorithm values */
    mpfr_t a, two, four, sum_xsq_ysq, yslice;       /* tmp vals */
    unsigned int iteration = 0;
    unsigned int bc = 0;
    struct Color rgb;
    unsigned int ystart = (slice)*(ysize/nslice); /* calc ystart incase we're doing slices */
    unsigned int yend = (slice+1)*(ysize/nslice); /* calc ystart incase we're doing slices */

    printf("slice - width %d height %d nslice %d slice %d ", xsize, ysize, nslice, slice);
    printf(" (for y = %d; y < %d)\n", ystart, yend);

    /* create all mpfr_t vars */
    mpfr_inits2(PRECISION, x, y, xsq, ysq, xtmp, x0, y0, ys1, ye1, (mpfr_ptr)NULL);
    mpfr_inits2(PRECISION, a, two, four, sum_xsq_ysq, yslice, (mpfr_ptr)NULL);

    /* initialise all mpfr_t vars */
    mpfr_set_d(x, 0.0, MPFR_RNDN);
    mpfr_set_d(y, 0.0, MPFR_RNDN);
    mpfr_set_d(xsq, 0.0, MPFR_RNDN);
    mpfr_set_d(ysq, 0.0, MPFR_RNDN);
    mpfr_set_d(xtmp, 0.0, MPFR_RNDN);
    mpfr_set_d(x0, 0.0, MPFR_RNDN);
    mpfr_set_d(y0, 0.0, MPFR_RNDN);
    mpfr_set_d(a, 0.0, MPFR_RNDN);
    mpfr_set_d(two, 2.0, MPFR_RNDN);
    mpfr_set_d(four, 4.0, MPFR_RNDN);
    mpfr_set_d(sum_xsq_ysq, 0.0, MPFR_RNDN);
    mpfr_set_d(yslice, 0.0, MPFR_RNDN);

    mpfr_sub(yslice, Ye, Ys, MPFR_RNDN);
    mpfr_div_ui(yslice, yslice, nslice, MPFR_RNDN); /* 4 should be replaced by number of slices */

    if(nslice == 1)
    {
        mpfr_set(ys1, Ys, MPFR_RNDN);
        mpfr_set(ye1, Ye, MPFR_RNDN);
    }
    else
    {
        /* Ys + n*yslice */
        mpfr_mul_ui(ys1, yslice, slice, MPFR_RNDN);
        mpfr_add(ys1, ys1, Ys, MPFR_RNDN);
        /* Ys + (n+1)*yslice */
        mpfr_mul_ui(ye1, yslice, slice+1, MPFR_RNDN);
        mpfr_add(ye1, ye1, Ys, MPFR_RNDN);
    }
    
    TRACE_DEBUG("mandlebrot_mpfr_slice_c (in)\n");
#ifdef TRACE
    mpfr_out_str(stdout, 10, 0, Xs, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, Xe, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, ys1, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, ye1, MPFR_RNDN); putchar('\n');
#endif
    TRACE_DEBUG("\n");


    for (unsigned int Dy = ystart; Dy < yend; Dy++)
    {
        //mpfr_out_str(stdout, 10, 0, y0, MPFR_RNDN); putchar('\n');
        for (unsigned int Dx = 0; Dx < xsize; Dx++)
        {
            iteration = 0;

            /* double x0 = scaled(Dx, xsize, Xs, Xe); */
            mpfr_sub(a, Xe, Xs, MPFR_RNDN);
            mpfr_mul_d(a, a, ((double)Dx/(double)xsize), MPFR_RNDN);
            mpfr_add(x0, a, Xs, MPFR_RNDN);

            /* double y0 = scaled(Dy, ysize, Ys, Ye);*/
            mpfr_sub(a, ye1, ys1, MPFR_RNDN);
            mpfr_mul_d(a, a, ((double)(Dy-ystart)/((double)(yend-ystart))), MPFR_RNDN);
            mpfr_add(y0, a, ys1, MPFR_RNDN);

            /* reset some vars for each pixel */
            mpfr_set_d(x, 0.0, MPFR_RNDN);
            mpfr_set_d(y, 0.0, MPFR_RNDN);
            mpfr_set_d(sum_xsq_ysq, 0.0, MPFR_RNDN);

            /* while (xsq+ysq <= 4 && iteration < maxiter */
            while ((mpfr_cmp(sum_xsq_ysq, four) <= 0) && (iteration < maxiter))
            {
                mpfr_mul(xsq, x, x, MPFR_RNDN);      /* xsq = x*x; */
                mpfr_mul(ysq, y, y, MPFR_RNDN);      /* ysq = y*y; */
                mpfr_sub(xtmp, xsq, ysq, MPFR_RNDN); /* xtemp = xsq - ysq + x0; */
                mpfr_add(xtmp, xtmp, x0, MPFR_RNDN);
                mpfr_mul(a, two, x, MPFR_RNDN);      /* y = 2.0*x*y + y0; */
                mpfr_mul(a, a, y, MPFR_RNDN);
                mpfr_add(y, a, y0, MPFR_RNDN);
                mpfr_swap(x, xtmp);                  /* x = xtemp; */

                /* calcs for while test */
                mpfr_add(sum_xsq_ysq, xsq, ysq, MPFR_RNDN);
                iteration++;
                //printf("iteration %d\n", iteration);
            }
            /* create a color value and add to result list */
            rgb = sqrt_gradient(iteration, maxiter);

            bytearray[bc] = rgb.r;
            bc++;

            bytearray[bc] = rgb.g;
            bc++;

            bytearray[bc] = rgb.b;
            bc++;
        }
    }
    mpfr_clears(x, y, xsq, ysq, xtmp, x0, y0/*, Xs, Xe, Ys, Ye*/, (mpfr_ptr)NULL);
    mpfr_clears(a, two, four, sum_xsq_ysq, (mpfr_ptr)NULL);
}

/* ----------------------------------------------------------------------------
 */
void mpfr_zoom_in(       const unsigned int pX, /* x mouse pos in display */
                         const unsigned int pY, /* y mouse pos in display */
                         const unsigned int w,  /* width of display */
                         const unsigned int h,  /* height of display */
                         const unsigned int factor /* scaling factor */
                        )
{
    unsigned int w1, h1;
    mpfr_t lx, ly, TLx, TLy, BRx, BRy;

    mpfr_inits2(PRECISION, lx, ly, TLx, TLy, BRx, BRy, (mpfr_ptr)NULL);

    TRACE_DEBUGV("mpfr_zoom_in (in) disp %d x %d factor %d at %d x %d\n",w, h, factor, pX, pY);
#ifdef TRACE
    mpfr_out_str(stdout, 10, 0, Xs, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, Xe, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, Ys, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, Ye, MPFR_RNDN); putchar('\n');
#endif
    TRACE_DEBUG("\n");

    /* scaled ( ((double)x1 / (double)sz) * (Xe-Xs) ) + Xs; */
    /* double X0 = scaled(pX, xsz, Xs, Xe); */
    mpfr_sub(lx, Xe, Xs, MPFR_RNDN);
    mpfr_mul_d(lx, lx, ((double)pX/(double)w), MPFR_RNDN);
    mpfr_add(lx, lx, Xs, MPFR_RNDN);

    /* double y0 = scaled(pY, ysz, Ys, Ye); */
    mpfr_sub(ly, Ye, Ys, MPFR_RNDN);
    mpfr_mul_d(ly, ly, ((double)pY/(double)h), MPFR_RNDN);
    mpfr_add(ly, ly, Ys, MPFR_RNDN);

    TRACE_DEBUG("scaled loc (mpfr)\n");
#ifdef TRACE
    mpfr_out_str(stdout, 10, 0, lx, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, ly, MPFR_RNDN); putchar('\n');
#endif

    /* TLx = loc_x - abs((xe-xs)/3) */
    mpfr_sub(TLx, Xe, Xs, MPFR_RNDN);
    mpfr_div_ui(TLx, TLx, factor, MPFR_RNDN);
    mpfr_abs(TLx, TLx, MPFR_RNDN);
    mpfr_sub(TLx, lx, TLx, MPFR_RNDN);

    /* TLy = loc_y - abs((ye-ys)/3) */
    mpfr_sub(TLy, Ye, Ys, MPFR_RNDN);
    mpfr_div_ui(TLy, TLy, factor, MPFR_RNDN);
    mpfr_abs(TLy, TLy, MPFR_RNDN);
    mpfr_sub(TLy, ly, TLy, MPFR_RNDN);

    /* BRx = loc_x + abs((xe-xs)/3) */
    mpfr_sub(BRx, Xe, Xs, MPFR_RNDN);
    mpfr_div_ui(BRx, BRx, factor, MPFR_RNDN);
    mpfr_abs(BRx, BRx, MPFR_RNDN);
    mpfr_add(BRx, lx, BRx, MPFR_RNDN);

    /* BRy = loc_y + abs((ye-ys)/3) */
    mpfr_sub(BRy, Ye, Ys, MPFR_RNDN);
    mpfr_div_ui(BRy, BRy, factor, MPFR_RNDN);
    mpfr_abs(BRy, BRy, MPFR_RNDN);
    mpfr_add(BRy, ly, BRy, MPFR_RNDN);

    mpfr_set(Xs, TLx, MPFR_RNDN);
    mpfr_set(Xe, BRx, MPFR_RNDN);
    mpfr_set(Ys, TLy, MPFR_RNDN);
    mpfr_set(Ye, BRy, MPFR_RNDN);

    TRACE_DEBUG("mpfr_zoom_in (out) \n");
#ifdef TRACE
    mpfr_out_str(stdout, 10, 0, Xs, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, Xe, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, Ys, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, Ye, MPFR_RNDN); putchar('\n');
#endif
    TRACE_DEBUG("\n");

    zoom_level = zoom_level + 1;

    mpfr_sub(lx, Xe, Xs, MPFR_RNDN);
    mpfr_mul_ui(lx, lx, zoom_level*10*w, MPFR_RNDN);

    mpfr_sub(ly, Ye, Ys, MPFR_RNDN);
    mpfr_mul_ui(ly, ly, zoom_level*10*h, MPFR_RNDN);

    w1 = mpfr_get_ui(lx, MPFR_RNDN);
    h1 = mpfr_get_ui(ly, MPFR_RNDN);
    TRACE_DEBUGV("display size: %d %d\n", w1, h1);

    mpfr_clears(lx, ly, TLx, TLy, BRx, BRy, (mpfr_ptr)NULL);
}

/* ----------------------------------------------------------------------------
 */
void mpfr_zoom_out(      const unsigned int pX, /* x mouse pos in display */
                         const unsigned int pY, /* y mouse pos in display */
                         const unsigned int w,  /* width of display */
                         const unsigned int h,  /* height of display */
                         const unsigned int factor /* scaling factor */
                        )
{
    unsigned int w1, h1;
    mpfr_t lx, ly, TLx, TLy, BRx, BRy;

    mpfr_inits2(PRECISION, lx, ly, TLx, TLy, BRx, BRy, (mpfr_ptr)NULL);

    /* #def zoom_out(xs, xe, ys, ye, pos): # pos(x,y) window_size(x,y)
    def zoom_out(self, pos): # pos(x,y) window_size(x,y)
        #print("pos ", pos)*/

    /*lx = self.scaled(pos[0], window_size, self.Xs, self.Xe)*/
    mpfr_sub(lx, Xe, Xs, MPFR_RNDN);
    mpfr_mul_d(lx, lx, ((double)pX/(double)w), MPFR_RNDN);
    mpfr_add(lx, lx, Xs, MPFR_RNDN);

    /*ly = self.scaled(pos[1], window_size, self.Ys, self.Ye)*/
    mpfr_sub(ly, Ye, Ys, MPFR_RNDN);
    mpfr_mul_d(ly, ly, ((double)pY/(double)h), MPFR_RNDN);
    mpfr_add(ly, ly, Ys, MPFR_RNDN);

    /*TLx = loc[0]-abs((self.Xe-self.Xs)*self.factor )*/
    mpfr_sub(TLx, Xe, Xs, MPFR_RNDN);
    mpfr_mul_ui(TLx, TLx, factor, MPFR_RNDN);
    mpfr_abs(TLx, TLx, MPFR_RNDN);
    mpfr_sub(TLx, lx, TLx, MPFR_RNDN);

    /*TLy = loc[1]-abs((self.Ye-self.Ys)*self.factor )*/
    mpfr_sub(TLy, Ye, Ys, MPFR_RNDN);
    mpfr_mul_ui(TLy, TLy, factor, MPFR_RNDN);
    mpfr_abs(TLy, TLy, MPFR_RNDN);
    mpfr_sub(TLy, ly, TLy, MPFR_RNDN);

    /*BRx = loc[0]+abs((self.Xe-self.Xs)*self.factor )*/
    mpfr_sub(BRx, Xe, Xs, MPFR_RNDN);
    mpfr_mul_ui(BRx, BRx, factor, MPFR_RNDN);
    mpfr_abs(BRx, BRx, MPFR_RNDN);
    mpfr_add(BRx, lx, BRx, MPFR_RNDN);

    /*BRy = loc[1]+abs((self.Ye-self.Ys)*self.factor )*/
    mpfr_sub(BRy, Ye, Ys, MPFR_RNDN);
    mpfr_mul_ui(BRy, BRy, factor, MPFR_RNDN);
    mpfr_abs(BRy, BRy, MPFR_RNDN);
    mpfr_add(BRy, ly, BRy, MPFR_RNDN);

    mpfr_set(Xs, TLx, MPFR_RNDN);
    mpfr_set(Xe, BRx, MPFR_RNDN);
    mpfr_set(Ys, TLy, MPFR_RNDN);
    mpfr_set(Ye, BRy, MPFR_RNDN);

    /*# if we start to hit the upper bounds then adjust the centre
    if TLx < X1 or BRx > X2 or TLy < Y1 or BRy > Y2:*/
    if ((mpfr_cmp_d(Xs, -2.0) < 0) && (mpfr_cmp_d(Xe, 1.0) > 0) &&
        (mpfr_cmp_d(Ys, -1.5) < 0) && (mpfr_cmp_d(Ye, 1.5) > 0))
    {
        mpfr_set_d(Xs, -2.0, MPFR_RNDN);
        mpfr_set_d(Xe,  1.0, MPFR_RNDN);
        mpfr_set_d(Ys, -1.5, MPFR_RNDN);
        mpfr_set_d(Ye,  1.5, MPFR_RNDN);
    }
    /*  #print("new coords ", TLx, BRx, TLy, BRy)
        #return TLx, BRx, TLy, BRy
        self.Xs, self.Xe, self.Ys, self.Ye = TLx, BRx, TLy, BRy
    */
    TRACE_DEBUG("mpfr_zoom_out (out) \n");
#ifdef TRACE
    mpfr_out_str(stdout, 10, 0, Xs, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, Xe, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, Ys, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, Ye, MPFR_RNDN); putchar('\n');
#endif
    TRACE_DEBUG("\n");

    zoom_level = zoom_level - 1;

    mpfr_sub(lx, Xe, Xs, MPFR_RNDN);
    mpfr_mul_ui(lx, lx, zoom_level*10*w, MPFR_RNDN);

    mpfr_sub(ly, Ye, Ys, MPFR_RNDN);
    mpfr_mul_ui(ly, ly, zoom_level*10*h, MPFR_RNDN);

    w1 = mpfr_get_ui(lx, MPFR_RNDN);
    h1 = mpfr_get_ui(ly, MPFR_RNDN);
    TRACE_DEBUGV("display size: %d %d\n", w1, h1);

    mpfr_clears(lx, ly, TLx, TLy, BRx, BRy, (mpfr_ptr)NULL);
}


/* ----------------------------------------------------------------------------
 */
