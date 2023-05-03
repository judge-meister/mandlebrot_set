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
#include <string.h>
#include <math.h>
#include <pthread.h>

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

/* TYPEDEFS */
struct Color { int r, g, b; };
typedef struct Color Color;
struct chunk_params { unsigned int x0, y0, x1, y1, maxiter, tid; 
                      mpfr_t Xe, Xs, Ye, Ys; };
typedef struct chunk_params chunk_params;

/* STATIC VARIABLES */
static mpfr_t Xe, Xs, Ye, Ys; /* algorithm values */
static int zoom_level;

/* GLOBAL DATA */
int* glb_bytearray[4]; /* 4 pointers to malloc'd bytearrays. 
                        * this cannot be fixed to 4 if we are to be able to 
                        * use any multi core CPU. */ 


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
    Color c; /*  = { 0, 0, 0 } */
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
    Color c;
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
            Color rgb;
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
#ifdef TRACE
    unsigned int w1, h1;
#endif
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

#ifdef TRACE
    w1 = mpfr_get_ui(lx, MPFR_RNDN);
    h1 = mpfr_get_ui(ly, MPFR_RNDN);
#endif
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
#ifdef TRACE
    unsigned int w1, h1;
#endif
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

#ifdef TRACE
    w1 = mpfr_get_ui(lx, MPFR_RNDN);
    h1 = mpfr_get_ui(ly, MPFR_RNDN);
#endif
    TRACE_DEBUGV("display size: %d %d\n", w1, h1);

    mpfr_clears(lx, ly, TLx, TLy, BRx, BRy, (mpfr_ptr)NULL);
}


/* ----------------------------------------------------------------------------
 * thread worker function (needs a better name than do_chunk)
 *
 * currently given a quarter of the area of the image to work on.  This causes
 * problems when stitching the resultant data back together afterwards.  Would
 * be better to use horizontal sliced.  This would make it easier to split into
 * threadable chunks of work without fixating on the exact number of threads.
 * 
 */
void do_chunk(void* arg)
{
    chunk_params *cp;
    cp = (chunk_params*)arg;
    unsigned int maxiter = cp->maxiter;
    unsigned int iteration = 0;
    unsigned int bc = 0; /* bytearray index counter */
    Color rgb;

    mpfr_t x, y, xsq, ysq, xtmp, x0, y0/*, Xe, Xs, Ye, Ys*/; /* algorithm values */
    mpfr_t a, two, four, sum_xsq_ysq;                    /* tmp vals */

    /* create all mpfr_t vars */
    mpfr_inits2(PRECISION, x, y, xsq, ysq, xtmp, x0, y0/*, Xs, Xe, Ys, Ye*/, (mpfr_ptr)NULL);
    mpfr_inits2(PRECISION, a, two, four, sum_xsq_ysq, (mpfr_ptr)NULL);

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

    for (unsigned int Dy = cp->y0; Dy < cp->y1; Dy++)
    {
        for (unsigned int Dx = cp->x0; Dx < cp->x1; Dx++)
        {
            iteration = 0;

            /* double x0 = scaled(Dx, xsize, Xs, Xe); */
            mpfr_sub(a, cp->Xe, cp->Xs, MPFR_RNDN);
            mpfr_mul_d(a, a, ((double)Dx/(double)cp->x1), MPFR_RNDN);
            mpfr_add(x0, a, cp->Xs, MPFR_RNDN);

            /* double y0 = scaled(Dy, ysize, Ys, Ye); */
            mpfr_sub(a, cp->Ye, cp->Ys, MPFR_RNDN);
            mpfr_mul_d(a, a, ((double)Dy/(double)cp->y1), MPFR_RNDN);
            mpfr_add(y0, a, cp->Ys, MPFR_RNDN);

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
            //printf("bc %d r %d\n", bc, rgb.r);
            glb_bytearray[cp->tid][bc] = rgb.r;
            bc++;
            //printf("bc %d g %d\n", bc, rgb.g);
            glb_bytearray[cp->tid][bc] = rgb.g;
            bc++;
            //printf("bc %d b %d\n", bc, rgb.b);
            glb_bytearray[cp->tid][bc] = rgb.b;
            bc++;
        }
    }
    mpfr_clears(x, y, xsq, ysq, xtmp, x0, y0/*, Xs, Xe, Ys, Ye*/, (mpfr_ptr)NULL);
    mpfr_clears(a, two, four, sum_xsq_ysq, (mpfr_ptr)NULL);

    printf("Thread %d exiting\n",cp->tid);
    pthread_exit(&glb_bytearray[cp->tid]);
}


/* ----------------------------------------------------------------------------
 * mandlebrot set using mpfr library, but 4 threads
 *
 * Description - the data area is cut into quarters although this is not the
 *               easiest split to manage as putting the results back together
 *               is non trivial.  would be better to use horizontal slices.
 *
 *               the setting up of the params for the threads is currently a
 *               bit long-winded and only copes with 4 threads.  Need to use
 *               a loop along with arrays of structures to provide flexibility
 *               to cope with any number of CPU cores
 * Params
 * xsize, ysize   - width and height of fractal
 * maxiter        - the maximum iterations before escaping the algorithm
 *
 * (out)bytearray - a bytearray of ints storing the color values of calculated points
 *
 * Return void (not status returned)
 */
void mandlebrot_mpfr_thread_c( const unsigned int xsize,   /* width of screen/display/window */
                               const unsigned int ysize,   /* height of screen/display/window */
                               const unsigned int maxiter, /* max iterations before escape */
                               int bytearray[] /* reference/pointer to result list of color values*/
                             )
{
    unsigned int bc = 0;

    printf("mandlebrot_mpfr_c (in) \n");
    mpfr_out_str(stdout, 10, 0, Xs, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, Xe, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, Ys, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, Ye, MPFR_RNDN); putchar('\n');
    putchar('\n');

    /* divide data into 4 quarters that can be split between 4 threads 
     *
     *  Xs Xe Ys Ye
     *  
     *  -2 -> 1
     *  -1.5 -> 1.5
     *
     *  -2->-0.5  -0.5->1
     *  -1.5->0   0->1.5
     */
    pthread_t tid[4];

    /* initialise the global bytearray*/
    for(int i=0; i<4; i++)
    {
      glb_bytearray[i] = (int*)calloc((size_t)(xsize/2 * ysize/2 * 3), sizeof(int));
    }

    chunk_params cp1;
    mpfr_t lx, ly;
    mpfr_inits2(PRECISION, lx, ly, cp1.Xe, cp1.Xs, cp1.Ye, cp1.Ys, (mpfr_ptr)NULL);
    cp1.tid = 0;
    cp1.x0 = 0;
    cp1.y0 = 0;
    cp1.x1 = xsize/2;
    cp1.y1 = ysize/2;
    cp1.maxiter = maxiter;
    
    mpfr_set(cp1.Xs, Xs, MPFR_RNDN);//cp1->Xs = Xs;               -2
    
    mpfr_sub(lx, Xe, Xs, MPFR_RNDN);//cp1->Xe = Xs + (Xe-Xs)/2;   -2 + (1 - -2)/2
    mpfr_div_ui(lx, lx, 2, MPFR_RNDN);
    mpfr_add(cp1.Xe, lx, Xs, MPFR_RNDN);
    
    mpfr_set(cp1.Ys, Ys, MPFR_RNDN);//cp1->Ys = Ys;               -1.5
    
    mpfr_sub(ly, Ye, Ys, MPFR_RNDN);//cp1->Ye = Ys + (Ye-Ys)/2;   -1.5 + (1.5 - -1.5)/2
    mpfr_div_ui(ly, ly, 2, MPFR_RNDN);
    mpfr_add(cp1.Ye, ly, Ys, MPFR_RNDN);

    printf("start thread[%d](%d,%d %d,%d)\n",cp1.tid, cp1.x0, cp1.y0, cp1.x1, cp1.y1);
    mpfr_out_str(stdout, 10, 0, cp1.Xs, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, cp1.Xe, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, cp1.Ys, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, cp1.Ye, MPFR_RNDN); putchar('\n');
    pthread_create(&tid[0], NULL, (void *)do_chunk, &cp1);

    chunk_params cp2;
    mpfr_inits2(PRECISION, cp2.Xe, cp2.Xs, cp2.Ye, cp2.Ys, (mpfr_ptr)NULL);
    cp2.tid = 1;
    cp2.x0 = 0;
    cp2.y0 = 0;
    cp2.x1 = xsize/2;
    cp2.y1 = ysize/2;
    cp2.maxiter = maxiter;
    //cp2.Xs = Xs
    mpfr_set(cp2.Xs, Xs, MPFR_RNDN);
    //cp2.Ys = Ys + (Ye-Ys)/2
    mpfr_sub(ly, Ye, Ys, MPFR_RNDN);
    mpfr_div_ui(ly, ly, 2, MPFR_RNDN);
    mpfr_add(cp2.Ys, ly, Ys, MPFR_RNDN);
    //cp2.Xe = Xs +(Xe-Xs)/2
    mpfr_sub(lx, Xe, Xs, MPFR_RNDN);
    mpfr_div_ui(lx, lx, 2, MPFR_RNDN);
    mpfr_add(cp2.Xe, lx, Xs, MPFR_RNDN);
    //cp2.Ye = Ye
    mpfr_set(cp2.Ye, Ye, MPFR_RNDN);
    
    printf("start thread[%d](%d,%d %d,%d)\n",cp2.tid, cp2.x0, cp2.y0, cp2.x1, cp2.y1);
    mpfr_out_str(stdout, 10, 0, cp2.Xs, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, cp2.Xe, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, cp2.Ys, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, cp2.Ye, MPFR_RNDN); putchar('\n');
    pthread_create(&tid[1], NULL, (void *)do_chunk, &cp2);

    chunk_params cp3;
    mpfr_inits2(PRECISION, cp3.Xe, cp3.Xs, cp3.Ye, cp3.Ys, (mpfr_ptr)NULL);
    cp3.tid = 2;
    cp3.x0 = 0;
    cp3.y0 = 0;
    cp3.x1 = xsize/2;
    cp3.y1 = ysize/2;
    cp3.maxiter = maxiter;
    //cp3.Xs = Xs + (Xe-Xs)/2
    mpfr_sub(lx, Xe, Xs, MPFR_RNDN);
    mpfr_div_ui(lx, lx, 2, MPFR_RNDN);
    mpfr_add(cp3.Xs, lx, Xs, MPFR_RNDN);
    //cp3.Ys = Ys
    mpfr_set(cp3.Ys, Ys, MPFR_RNDN);
    //cp3.Xe = Xe
    mpfr_set(cp3.Xe, Xe, MPFR_RNDN);
    //cp3.Ye = Ys + (Ye-Ys)/2
    mpfr_sub(ly, Ye, Ys, MPFR_RNDN);
    mpfr_div_ui(ly, ly, 2, MPFR_RNDN);
    mpfr_add(cp3.Ye, ly, Ys, MPFR_RNDN);
    
    printf("start thread[%d](%d,%d %d,%d)\n",cp3.tid, cp3.x0, cp3.y0, cp3.x1, cp3.y1);
    mpfr_out_str(stdout, 10, 0, cp3.Xs, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, cp3.Xe, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, cp3.Ys, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, cp3.Ye, MPFR_RNDN); putchar('\n');
    pthread_create(&tid[2], NULL, (void *)do_chunk, &cp3);

    chunk_params cp4;
    mpfr_inits2(PRECISION, cp4.Xe, cp4.Xs, cp4.Ye, cp4.Ys, (mpfr_ptr)NULL);
    cp4.tid = 3;
    cp4.x0 = 0;
    cp4.y0 = 0;
    cp4.x1 = xsize/2;
    cp4.y1 = ysize/2;
    cp4.maxiter = maxiter;
    //cp4.Xs = Xs + (Xe-Xs)/2
    mpfr_sub(lx, Xe, Xs, MPFR_RNDN);
    mpfr_div_ui(lx, lx, 2, MPFR_RNDN);
    mpfr_add(cp4.Xs, lx, Xs, MPFR_RNDN);
    //cp4.Ys = Ys + (Ye-Ys)/2
    mpfr_sub(ly, Ye, Ys, MPFR_RNDN);
    mpfr_div_ui(ly, ly, 2, MPFR_RNDN);
    mpfr_add(cp4.Ys, ly, Ys, MPFR_RNDN);
    //cp4.Xe = Xe
    mpfr_set(cp4.Xe, Xe, MPFR_RNDN);
    //cp4.Ye = Ye
    mpfr_set(cp4.Ye, Ye, MPFR_RNDN);
    
    printf("start thread[%d](%d,%d %d,%d)\n",cp4.tid, cp4.x0, cp4.y0, cp4.x1, cp4.y1);
    mpfr_out_str(stdout, 10, 0, cp4.Xs, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, cp4.Xe, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, cp4.Ys, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, cp4.Ye, MPFR_RNDN); putchar('\n');
    pthread_create(&tid[3], NULL, (void *)do_chunk, &cp4);

    /*wait for all the threads to complete */
    int* ptr1, ptr2, ptr3, ptr4; /* this should be replaced with a bytearray */
    pthread_join(tid[0], (void**)&ptr1);
    pthread_join(tid[1], (void**)&ptr2);
    pthread_join(tid[2], (void**)&ptr3);
    pthread_join(tid[3], (void**)&ptr4);

    mpfr_clears(lx, ly, (mpfr_ptr)NULL);
    mpfr_clears(cp1.Xe, cp1.Xs, cp1.Ye, cp1.Ys, (mpfr_ptr)NULL);
    mpfr_clears(cp2.Xe, cp2.Xs, cp2.Ye, cp2.Ys, (mpfr_ptr)NULL);
    mpfr_clears(cp3.Xe, cp3.Xs, cp3.Ye, cp3.Ys, (mpfr_ptr)NULL);
    mpfr_clears(cp4.Xe, cp4.Xs, cp4.Ye, cp4.Ys, (mpfr_ptr)NULL);
    
    /* populate the returned bytearray from the global one */
    /* (pseudo)
    using memcpy()
    for y in range(0,ysize):
      if y < ysize/2:
        memcpy(ba[bc],ba0[bc0], xsize/2);
        bc = bc + xsize/2;
        bc0 = bc0 + xsize/2;
        memcpy(ba[bc],ba1[bc1], xsize/2);
        bc = bc + xsize/2;
        bc1 = bc1 + xsize/2;
      else
        memcpy(ba[bc],ba2[bc2], xsize/2);
        bc = bc + xsize/2;
        bc2 = bc2 + xsize/2;
        memcpy(ba[bc],ba3[bc3], xsize/2);
        bc = bc + xsize/2;
        bc3 = bc3 + xsize/2;

    */
    printf("Combining Results\n");
    unsigned int bc0, bc1, bc2, bc3;
    bc0 = bc1 = bc2 = bc3 = 0;
    bc = 0;
    for(unsigned int y=0; y < ysize; y++)
    { 
      if(y < ysize/2)
      {
        /*memcpy((void*)(&bytearray[bc]), (void*)(&glb_bytearray[0][bc0]), xsize/2*3);*/
        for(unsigned int x=bc0; x<bc0+xsize/2*3; x++)
        { bytearray[bc] = glb_bytearray[0][x]; bc++;}
        /*bc = bc + xsize/2*3;*/
        bc0 = bc0 + xsize/2*3;
        /*memcpy((void*)(&bytearray[bc]), (void*)(&glb_bytearray[2][bc2]), xsize/2*3);*/
        for(unsigned int x=bc2; x<bc2+xsize/2*3; x++)
        { bytearray[bc] = glb_bytearray[2][x]; bc++;}
        /*bc = bc + xsize/2*3;*/
        bc2 = bc2 + xsize/2*3;
      }
      else
      {
        /*memcpy((void*)(&bytearray[bc]), (void*)(&glb_bytearray[1][bc1]), xsize/2*3);*/
        for(unsigned int x=bc1; x<bc1+xsize/2*3; x++)
        { bytearray[bc] = glb_bytearray[1][x]; bc++;}
        /*bc = bc + xsize/2*3;*/
        bc1 = bc1 + xsize/2*3;
        /*memcpy((void*)(&bytearray[bc]), (void*)(&glb_bytearray[3][bc3]), xsize/2*3);*/
        for(unsigned int x=bc3; x<bc3+xsize/2*3; x++)
        { bytearray[bc] = glb_bytearray[3][x]; bc++;}
        /*bc = bc + xsize/2*3;*/
        bc3 = bc3 + xsize/2*3;
      }
    }

    /* free the global bytearray*/
    printf("Freeing glb_bytearray\n");
    for(int i=0; i<4; i++)
    {
      free(glb_bytearray[i]); /* = (int*)calloc((size_t)(xsize/2 * ysize/2 * 3), sizeof(int));*/
    }
    printf("mandlebrot_mpfr_c (exit) \n");
}

/* ----------------------------------------------------------------------------
 */
