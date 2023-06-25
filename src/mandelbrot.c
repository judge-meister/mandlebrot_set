/* ----------------------------------------------------------------------------
 *  C library with python module interface to calculate the mandelbrot set for
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
 * pseudo code to generate the mandelbrot set
 *
 * for each pixel (Px, Py) on the screen do
 *     x0 := scaled x coordinate of pixel (scaled to lie in the Mandelbrot X scale (-2.00, 0.47))
 *     y0 := scaled y coordinate of pixel (scaled to lie in the Mandelbrot Y scale (-1.12, 1.12))
 *     x := 0.0
 *     y := 0.0
 *     iteration := 0
 *     max_iteration := 1000
 *     while (x*x + y*y <= 2*2 AND iteration < max_iteration) do
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

#ifdef USES_THREADS
#include <pthread.h>
#endif

#ifdef _WIN32
#include <windows.h>
#elif MACOS
#include <sys/param.h>
#include <sys/sysctl.h>
#elif _WIN64
#pragma warning CANNOT BUILD FOR WIN64
#else
#include <unistd.h>
#endif

#ifdef _WIN32
#include <mpir.h> /* Was GMP */
#else
#include <gmp.h>
#endif

#include <mpfr.h>

#include "mandelbrot.h"

/* DEFINES */
#define TRUE 1
#define FALSE 0
#define BOOLEAN unsigned int

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


#define PRECISION 256
/* if PRECISION is increased then NUMLEN needs to increase */
//#define NUMLEN 50

#define MAXITER 1000

/* TYPEDEFS */
struct Color { int r, g, b; };
typedef struct Color Color;
struct worker_args { unsigned int x0, y0, x1, y1, maxiter, tid, cpus; 
                     mpfr_t Xe, Xs, Ye, Ys; };
typedef struct worker_args worker_args;

/* STATIC VARIABLES */
static mpfr_t Xe, Xs, Ye, Ys, Cx, Cy; /* algorithm values */
static mpfr_t MX, MY, Xe_Xs, Ye_Ys, CX, CY;
  
static int zoom_level = 0;
static int ncpus = 1; /* just to be safe */

/* GLOBAL DATA */
unsigned int** glb_bytearray; /* this will contain an array of bytearrays
                      * ncpus x (num points in array) */ 


/* ----------------------------------------------------------------------------
 * short utility function to assign the rgb values to the Color struct
 */
static struct Color setRgb(int r, int g, int b)
{
    Color c = {r, g, b};
    return c;
}

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
    Color c = { 0, 0, 0 };

    if (it < maxiter)
    {
        /*int idx = (int)ceil( sqrt( sqrt( (double)it / (double)MAXITER ) ) * 255.0 );*/
        int idx = (int)ceil( sqrt( (double)it / (double)maxiter ) * 255.0 );
        c = setRgb(idx, idx, idx);
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
        c = setRgb(0, 0, 0);
    }
    return c;
}

/* ----------------------------------------------------------------------------
 * Ultra_Fract_colors - these are the colors used by the Ultra Fractal program
 *
 * uses a simple modulo method to choose a color from a fixed list
 */
static struct Color Ultra_Fractal_colors(int it, int maxiter)
{
    if (it < maxiter && it > 0) 
    {
        int i = it % 16;
        Color mapping[16];
        mapping[0] = setRgb(66, 30, 15);
        mapping[1] = setRgb(25, 7, 26);
        mapping[2] = setRgb(9, 1, 47);
        mapping[3] = setRgb(4, 4, 73);
        mapping[4] = setRgb(0, 7, 100);
        mapping[5] = setRgb(12, 44, 138);
        mapping[6] = setRgb(24, 82, 177);
        mapping[7] = setRgb(57, 125, 209);
        mapping[8] = setRgb(134, 181, 229);
        mapping[9] = setRgb(211, 236, 248);
        mapping[10] = setRgb(241, 233, 191);
        mapping[11] = setRgb(248, 201, 95);
        mapping[12] = setRgb(255, 170, 0);
        mapping[13] = setRgb(204, 128, 0);
        mapping[14] = setRgb(153, 87, 0);
        mapping[15] = setRgb(106, 52, 3);
        return mapping[i];
    }
    else 
    {
        return setRgb(0, 0, 0);
    }
}

/* ----------------------------------------------------------------------------
 * calculate_point
 * run the z = z^2 + c algorithm for the point provided to assertain if the
 * point is inside or outside of the mandelbrot set
 *
 * Params
 * x0, y0 (double) location to calculate for
 * maxiter (int) the escape value
 *
 * Returns
 * iteration - count of how many iterations it took to head to infinity
 */
/*static int calculate_point(double x0, double y0, int maxiter)
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
}*/

/* ----------------------------------------------------------------------------
 * setup_c - create the corner variables ar mpfr types
 */
void setup_c()
{
    /* create all mpfr_t vars */
    mpfr_inits2(PRECISION, Xs, Xe, Ys, Ye, Cx, Cy, (mpfr_ptr)NULL);
    mpfr_inits2(PRECISION, MX, MY, Xe_Xs, Ye_Ys, (mpfr_ptr)NULL);
    mpfr_inits2(PRECISION, CX, CY, (mpfr_ptr)NULL);
    TRACE_DEBUG("called setup_c()\n");
    
    /* record the number of cpu cores */
#ifdef WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    ncpus = sysinfo.dwNumberOfProcessors;
#elif MACOS
    int nm[2];
    size_t len = 4;
    uint32_t count

    nm[0] = CTL_HW; nm[1] = HW_AVAILCPU;
    sysctl(nm, 2, &count, &len, NULL, 0);

    if (count < 1) {
    nm[1] = HW_NCPU;
    sysctl(nm, 2, &count, &len, NULL, 0);
    if (count < 1) { count = 1; }
    ncpus = count;
#else
    //ncpus = get_nprocs(); /* cpus available, use get_nprocs_conf() for cpus configured */
    ncpus = sysconf(_SC_NPROCESSORS_ONLN);
#endif

}

/* ----------------------------------------------------------------------------
 * initialise_c - set initial values for the image corners
 */
void initialize_c(
                const char* Xs_str, /* string repr of mpfr_t for X top left */
                const char* Xe_str, /* string repr of mpfr_t for X top right */
                const char* Ys_str, /* string repr of mpfr_t for Y bottom left */
                const char* Ye_str, /* string repr of mpfr_t for Y bottom right */
                const char* Cx_str, /* string repr of centre X pos for zooming in */
                const char* Cy_str  /* string repr of centre Y pos for zooming in */
               )
{
    TRACE_DEBUGV("called init_c(Cx = %s \nCy = %s )\n", Cx_str, Cy_str);
    mpfr_set_str(Xs, Xs_str, 10, MPFR_RNDN);
    mpfr_set_str(Xe, Xe_str, 10, MPFR_RNDN);
    mpfr_set_str(Ys, Ys_str, 10, MPFR_RNDN);
    mpfr_set_str(Ye, Ye_str, 10, MPFR_RNDN);
    
    mpfr_set_str(MX, Cx_str, 10, MPFR_RNDN);
    mpfr_set_str(MY, Cy_str, 10, MPFR_RNDN);
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
 */
static void push_sq_back_into_bounds()
{
    /* 5	push square back into bound of x(-2, 1) y(-1.5, 1.5) */
    /* if Xs < -2.0 then offset = Xs- -2.0; Xs=-2.0; Xe=Xe-offset */
    if (mpfr_cmp_d(Xs, -2.0) < 0) {
        mpfr_sub_d(CX, Xs, -2.0, MPFR_RNDN); // offset
        mpfr_set_d(Xs, -2.0, MPFR_RNDN); // adjust Xs
        mpfr_sub(Xe, Xe, CX, MPFR_RNDN); // adjust Xe
    }
    /* if Xe > 1.0  then offset = Xe-  1.0; Xe= 1.0; Xs=Xs-offset */
    if (mpfr_cmp_d(Xe, 1.0) > 0) {
        mpfr_sub_d(CX, Xe, 1.0, MPFR_RNDN); // offset
        mpfr_set_d(Xe, 1.0, MPFR_RNDN); // adjust Xe
        mpfr_sub(Xs, Xs, CX, MPFR_RNDN); // adjust Xs
  
        }
    /* if Ys < -1.5 then offset = Ys- -1.5; Ys=-1.5; Ye=Ye-offset */
    if (mpfr_cmp_d(Ys, -1.5) < 0) {
        mpfr_sub_d(CY, Ys, -1.5, MPFR_RNDN); // offset
        mpfr_set_d(Ys, -1.5, MPFR_RNDN); // adjust Ys
        mpfr_sub(Ye, Ye, CY, MPFR_RNDN); // adjust Ye
    }
    /* if Ye > 1.5  then offset = Ye-  1.5; Ye= 1.5; Ys=Ys-offset */
    if (mpfr_cmp_d(Ye, 1.5) > 0) {
        mpfr_sub_d(CY, Ye, 1.5, MPFR_RNDN); // offset
        mpfr_set_d(Ye, 1.5, MPFR_RNDN); // adjust Ye
        mpfr_sub(Ys, Ys, CY, MPFR_RNDN); // adjust Ys
    }
    /**/
}

/* ----------------------------------------------------------------------------
 * How to zoom in - (needs similar zoom_out)
 *
 * The basic idea is to find where the mouse of pointing and convert that to a
 * point in the current dataset before find new corner extents in the current
 * dataset based on that new centre.
 *
 * Then scale the new corners based on a percentage factor to bring the corners
 * in by some amount. For example a 10% factor will reduce pull in the corners 
 * by 10% producing a new square that is 90% of the original.
 *
 * Finally check whether the new corners are still within the bounds of the 
 * initial square when the program started and adjust accordingly.
 *
 * 1	scale mouse pos PX,PY to the data
 * 	MX =	((PX / SW) * (XE - XS)) + XS
 * 	MY =	((PY / SH) * (YE - YS)) + YS
 *
 * 2	find centre of existing square	
 * 	CX =	((XE - XS) / 2) + XS
 * 	CY =	((YE - YS) / 2) + YS
 *
 * 3	using mouse pos find new corners of square
 * 	XS =	XS + MX - CX
 * 	YS =	YS + MY - CY
 * 	XE =	XE + MX - CX
 * 	YE =	YE + MY - CY
 *
 * 4	scale new square - get new corners factor is a percentage reduction (eg reduce by 10%)	
 * 	XS =	XS + ((XE - XS) / (1-(FAC/100) / 2)
 * 	YS =	XE - ((XE - XS) / (1-(FAC/100) / 2)
 * 	XE =	YS + ((YE - YS) / (1-(FAC/100) / 2)
 * 	YE =	YE - ((YE - YS) / (1-(FAC/100) / 2)
 *
 * 5	push square back into bound of x(-2, 1) y(-1.5, 1.5)
 */

/* ----------------------------------------------------------------------------
 * Zoom in via mouse
 *
 * use the current mouse position as the centre point to zoon in on
 */
void mpfr_zoom_in_via_mouse(
                const double mouse_x, const double mouse_y,
                const unsigned int screen_width, const unsigned int screen_height,
                const unsigned int zoom_factor /* +ve int repr of percent */
               )
{
    printf("mpfr_zoom_in_via_mouse()\n");
    /* 1	scale mouse pos PX,PY to the data	
     * 	MX =	((mouse_x / screen_width) * (XE - XS)) + XS
     * 	MY =	((mouse_y / screen_height) * (YE - YS)) + YS
     */
    /*printf("1. mouse x,y  %f, %f [%f, %f]\n",mouse_x,mouse_y, 
          ((mouse_x/screen_width)*3.0) + -2.0, ((mouse_y/screen_height)*3.0) + -1.5);
    */
  
    mpfr_sub(Xe_Xs, Xe, Xs, MPFR_RNDN);
    mpfr_sub(Ye_Ys, Ye, Ys, MPFR_RNDN);
  
    mpfr_mul_d(MX, Xe_Xs, (mouse_x/screen_width), MPFR_RNDN);
    mpfr_add(MX, MX, Xs, MPFR_RNDN);
  
    mpfr_mul_d(MY, Ye_Ys, (mouse_y/screen_height), MPFR_RNDN);
    mpfr_add(MY, MY, Ys, MPFR_RNDN);
  
    mpfr_zoom_in( screen_width, screen_height, zoom_factor);
}

/* ----------------------------------------------------------------------------
 * zoom in on a predetermined fixed position 
 * zoom in on the mouse location if you call _via_mouse() above first
 */
void mpfr_zoom_in(
                const unsigned int screen_width, const unsigned int screen_height,
                const unsigned int zoom_factor /* +ve int repr of percent */
               )
{
    printf("mpfr_zoom_in()\n");
  
    /*printf("MX, MY ");
    mpfr_out_str(stdout, 10, 10, MX, MPFR_RNDN); printf(", ");
    mpfr_out_str(stdout, 10, 10, MY, MPFR_RNDN); putchar('\n');
    */
    /* 2	find centre of existing square	
     * 	CX =	((XE - XS) / 2) + XS
     * 	CY =	((YE - YS) / 2) + YS
     */
    mpfr_sub(Xe_Xs, Xe, Xs, MPFR_RNDN);
    mpfr_sub(Ye_Ys, Ye, Ys, MPFR_RNDN);
  
    mpfr_div_ui(CX, Xe_Xs, 2, MPFR_RNDN);
    mpfr_add(CX, CX, Xs, MPFR_RNDN);
   		
    mpfr_div_ui(CY, Ye_Ys, 2, MPFR_RNDN);
    mpfr_add(CY, CY, Ys, MPFR_RNDN);
  
    /*printf("2. Centre CX, CY ");
    mpfr_out_str(stdout, 10, 10, CX, MPFR_RNDN); printf(", ");
    mpfr_out_str(stdout, 10, 10, CY, MPFR_RNDN); putchar('\n');
    */
  
    /* 3	using mouse pos find new corners of square
     * 	XS =	XS + MX - CX
     * 	XE =	XE + MX - CX
     * 	YS =	YS + MY - CY
     * 	YE =	YE + MY - CY
     */
    mpfr_add(Xs, Xs, MX, MPFR_RNDN);
    mpfr_sub(Xs, Xs, CX, MPFR_RNDN);
  
    mpfr_add(Xe, Xe, MX, MPFR_RNDN);
    mpfr_sub(Xe, Xe, CX, MPFR_RNDN);
  
    mpfr_add(Ys, Ys, MY, MPFR_RNDN);
    mpfr_sub(Ys, Ys, CY, MPFR_RNDN);

    mpfr_add(Ye, Ye, MY, MPFR_RNDN);
    mpfr_sub(Ye, Ye, CY, MPFR_RNDN);
  
    /*printf("3. New Corners Xs, Xe, Ys, Ye ");
    mpfr_out_str(stdout, 10, 10, Xs, MPFR_RNDN); printf(", ");
    mpfr_out_str(stdout, 10, 10, Xe, MPFR_RNDN); printf(", ");
    mpfr_out_str(stdout, 10, 10, Ys, MPFR_RNDN); printf(", ");
    mpfr_out_str(stdout, 10, 10, Ye, MPFR_RNDN); putchar('\n');
    */

    /* 4	scale new square - get new corners factor is a percentage reduction (eg reduce by 10%)	
     * 	XS =	XS + ((XE - XS) / FAC )/ 2   FAC = 1-(zoom_factor/100)
     * 	XE =	XE - ((XE - XS) / FAC )/ 2
     * 	YS =	YS + ((YE - YS) / FAC )/ 2
     * 	YE =	YE - ((YE - YS) / FAC )/ 2
     */
    mpfr_sub(Xe_Xs, Xe, Xs, MPFR_RNDN);
    mpfr_sub(Ye_Ys, Ye, Ys, MPFR_RNDN);

    mpfr_mul_d(CX, Xe_Xs, (1.0-((double)zoom_factor/100.0))/2.0, MPFR_RNDN);
    mpfr_mul_d(CY, Ye_Ys, (1.0-((double)zoom_factor/100.0))/2.0, MPFR_RNDN);
    //printf("zoom in factor = %f\n", (1.0-((double)zoom_factor/100.0))/2.0 );
  
    /*printf("Xe_Xs = ");
    mpfr_out_str(stdout, 10, 10, Xe_Xs, MPFR_RNDN); printf(", Ye_Ys = ");
    mpfr_out_str(stdout, 10, 10, Ye_Ys, MPFR_RNDN); putchar('\n');
    printf("CX = ");
    mpfr_out_str(stdout, 10, 10, CX, MPFR_RNDN); printf(", CY = ");
    mpfr_out_str(stdout, 10, 10, CY, MPFR_RNDN); putchar('\n');
    */
    mpfr_add(Xs, Xs, CX, MPFR_RNDN);
    mpfr_sub(Xe, Xe, CX, MPFR_RNDN);
    mpfr_add(Ys, Ys, CY, MPFR_RNDN);
    mpfr_sub(Ye, Ye, CY, MPFR_RNDN);

    /*printf("4. scale new square Xs, Xe, Ys, Ye ");
    mpfr_out_str(stdout, 10, 10, Xs, MPFR_RNDN); printf(", ");
    mpfr_out_str(stdout, 10, 10, Xe, MPFR_RNDN); printf(", ");
    mpfr_out_str(stdout, 10, 10, Ys, MPFR_RNDN); printf(", ");
    mpfr_out_str(stdout, 10, 10, Ye, MPFR_RNDN); putchar('\n');
    */

    /* 5	push square back into bound of x(-2, 1) y(-1.5, 1.5) */
    push_sq_back_into_bounds();    
}

/* ----------------------------------------------------------------------------
 * How to zoom out
 *
 * Zooming out should be simpler that in if we disregard the mouse location.
 *
 * Using the current centre of the dataset push the corners out by the zoom
 * factor (percentage) and then check for corners being out of bound of the 
 * initial square.
 *
 * 1	scale new square - get new corners factor is a percentage increase (eg reduce by 10%)	
 *
 * XS = XS - ( ((XE-XS) / (FAC/100)) - (XE-XS) )/2 
 * XE = XE + ( ((XE-XS) / (FAC/100)) - (XE-XS) )/2 
 * YS = YS - ( ((YE-YS) / (FAC/100)) - (YE-XS) )/2 
 * YE = YE + ( ((YE-YS) / (FAC/100)) - (YE-XS) )/2 
 *
 * 2	push square back into bound of x(-2, 1) y(-1.5, 1.5)
 * 3	ensure we are still within initial dataset of x(-2, 1) y(-1.5, 1.5)
 */
void mpfr_zoom_out( const unsigned int zoom_factor )
{
    printf("mpfr_zoom_out()\n");
  
    /* 1	scale new square - get new corners factor is a percentage increase (eg reduce by 10%)	
     *
     * XS = XS - ( ((XE-XS) / (FAC/100)) - (XE-XS) )/2 
     * XE = XE + ( ((XE-XS) / (FAC/100)) - (XE-XS) )/2 
     * YS = YS - ( ((YE-YS) / (FAC/100)) - (YE-XS) )/2 
     * YE = YE + ( ((YE-YS) / (FAC/100)) - (YE-XS) )/2 
     */
    mpfr_sub(Xe_Xs, Xe, Xs, MPFR_RNDN);
    mpfr_sub(Ye_Ys, Ye, Ys, MPFR_RNDN);
  
    mpfr_div_d(CX, Xe_Xs, ((double)zoom_factor/100.0), MPFR_RNDN);
    mpfr_div_d(CY, Ye_Ys, ((double)zoom_factor/100.0), MPFR_RNDN);
    //printf("zoom out factor = %f\n", ((double)zoom_factor/100.0) );
  
    mpfr_sub(CX, CX, Xe_Xs, MPFR_RNDN);
    mpfr_sub(CY, CY, Ye_Ys, MPFR_RNDN);
  
    mpfr_div_d(CX, CX, 2.0, MPFR_RNDN);
    mpfr_div_d(CY, CY, 2.0, MPFR_RNDN);
  
    /*printf("Xe_Xs = ");
    mpfr_out_str(stdout, 10, 10, Xe_Xs, MPFR_RNDN); printf(", Ye_Ys = ");
    mpfr_out_str(stdout, 10, 10, Ye_Ys, MPFR_RNDN); putchar('\n');
    printf("CX = ");
    mpfr_out_str(stdout, 10, 10, CX, MPFR_RNDN); printf(", CY = ");
    mpfr_out_str(stdout, 10, 10, CY, MPFR_RNDN); putchar('\n');
    */
    mpfr_sub(Xs, Xs, CX, MPFR_RNDN);
    mpfr_add(Xe, Xe, CX, MPFR_RNDN);
    mpfr_sub(Ys, Ys, CY, MPFR_RNDN);
    mpfr_add(Ye, Ye, CY, MPFR_RNDN);

    /*printf("4. scale new square Xs, Xe, Ys, Ye ");
    mpfr_out_str(stdout, 10, 10, Xs, MPFR_RNDN); printf(", ");
    mpfr_out_str(stdout, 10, 10, Xe, MPFR_RNDN); printf(", ");
    mpfr_out_str(stdout, 10, 10, Ys, MPFR_RNDN); printf(", ");
    mpfr_out_str(stdout, 10, 10, Ye, MPFR_RNDN); putchar('\n');
    */

    /* 2	push square back into bound of x(-2, 1) y(-1.5, 1.5) */
    push_sq_back_into_bounds();    
  
    /* 3	now check all extents again and reduce to initial values 
     */
    if (mpfr_cmp_d(Xs, -2.0) < 0) {
        mpfr_set_d(Xs, -2.0, MPFR_RNDN); // adjust Xs
    }
    if (mpfr_cmp_d(Xe, 1.0) > 0) {
        mpfr_set_d(Xe, 1.0, MPFR_RNDN); // adjust Xe
    }
    if (mpfr_cmp_d(Ys, -1.5) < 0) {
        mpfr_set_d(Ys, -1.5, MPFR_RNDN); // adjust Ys
    }
    if (mpfr_cmp_d(Ye, 1.5) > 0) {
        mpfr_set_d(Ye, 1.5, MPFR_RNDN); // adjust Ye
    }
}

/* ----------------------------------------------------------------------------
 * worker_process_slice (used in the threads)
 *
 * given a slice of the mandelbrot set data to process.
 * returns part of the result to be combined later.
 */
static void *worker_process_slice(void* arg)
{
    worker_args *cp;
    cp = (worker_args*)arg;
    unsigned int maxiter = cp->maxiter;
    unsigned int iteration = 0;
    unsigned int bc = 0; /* bytearray index counter */
    Color rgb;

    mpfr_t x, y, xsq, ysq, xtmp, x0, y0/*, Xe, Xs, Ye, Ys*/; /* algorithm values */
    mpfr_t a, sum_xsq_ysq;                    /* tmp vals */
    
    TRACE_DEBUGV("Start Thread[%d](%d,%d) (%d,%d)\n",cp->tid, cp->x0, cp->y0, cp->x1, cp->y1);
    TRACE_DEBUGV("Start Thread %d\n",cp->tid);

    /* create all mpfr_t vars */
    mpfr_inits2(PRECISION, x, y, xsq, ysq, xtmp, x0, y0/*, Xs, Xe, Ys, Ye*/, (mpfr_ptr)NULL);
    mpfr_inits2(PRECISION, a, sum_xsq_ysq, (mpfr_ptr)NULL);

    /* initialise all mpfr_t vars */
    mpfr_set_d(x, 0.0, MPFR_RNDN);
    mpfr_set_d(y, 0.0, MPFR_RNDN);
    mpfr_set_d(xsq, 0.0, MPFR_RNDN);
    mpfr_set_d(ysq, 0.0, MPFR_RNDN);
    mpfr_set_d(xtmp, 0.0, MPFR_RNDN);
    mpfr_set_d(x0, 0.0, MPFR_RNDN);
    mpfr_set_d(y0, 0.0, MPFR_RNDN);
    mpfr_set_d(a, 0.0, MPFR_RNDN);
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
            while ((mpfr_cmp_d(sum_xsq_ysq, 4.0) <= 0) && (iteration < maxiter))
            {
                mpfr_mul(xsq, x, x, MPFR_RNDN);      /* xsq = x*x; */
                mpfr_mul(ysq, y, y, MPFR_RNDN);      /* ysq = y*y; */
                mpfr_sub(xtmp, xsq, ysq, MPFR_RNDN); /* xtemp = xsq - ysq + x0; */
                mpfr_add(xtmp, xtmp, x0, MPFR_RNDN);
                mpfr_mul_d(a, x, 2.0, MPFR_RNDN);      /* y = 2.0*x*y + y0; */
                mpfr_mul(a, a, y, MPFR_RNDN);
                mpfr_add(y, a, y0, MPFR_RNDN);
                mpfr_swap(x, xtmp);                  /* x = xtemp; */

                /* calcs for while test */
                mpfr_add(sum_xsq_ysq, xsq, ysq, MPFR_RNDN);
                iteration++;
                //printf("iteration %d\n", iteration);
            }
            /* create a color value and add to result list */
            //rgb = sqrt_gradient(iteration, maxiter);
            //rgb = grayscale(iteration, maxiter);
            rgb = Ultra_Fractal_colors(iteration, maxiter);
            
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
    mpfr_clears(a, sum_xsq_ysq, (mpfr_ptr)NULL);

#ifdef USES_THREADS
    TRACE_DEBUGV("Exit Thread %d\n",cp->tid);
    if (cp->cpus > 1)
    {
        pthread_exit(&glb_bytearray[cp->tid]);
    }
#endif
    return NULL;
}


/* ----------------------------------------------------------------------------
 * mandelbrot set using mpfr library, but 4 threads
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
static void mandelbrot_mpfr_main_c( 
                const unsigned int xsize,   /* width of screen/display/window */
                const unsigned int ysize,   /* height of screen/display/window */
                const unsigned int maxiter, /* max iterations before escape */
                const BOOLEAN use_threads,  /* use threads or not*/
                unsigned int **bytearray /* reference/pointer to result list of color values*/
               )
{
    unsigned int bc = 0;
    mpfr_t lx, ly, yslice, ys1, ye1;
    mpfr_inits2(PRECISION, lx, ly, yslice, ys1, ye1, (mpfr_ptr)NULL);

#ifdef USES_THREADS
    printf("mandelbrot_mpfr_main_c (%s) Entry\n", use_threads?"Threaded":"Single");
    TRACE_DEBUGV("mandelbrot_mpfr_main_c (%s) Entry\n", use_threads?"Threaded":"Single");
#else
    printf("mandelbrot_mpfr_main_c (Single) Entry\n");
    TRACE_DEBUG("mandelbrot_mpfr_main_c (Single) Entry\n");
#endif
#ifdef TRACE
    mpfr_out_str(stdout, 10, 0, Xs, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, Xe, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, Ys, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, Ye, MPFR_RNDN); putchar('\n');
    putchar('\n');
#endif
    
    unsigned int core_count = 1;
    unsigned int nslice = 1;
    
    if (use_threads == TRUE)
    {
        core_count = ncpus;
        nslice = core_count;
    }

    printf("core_count = %d\n", core_count);
#ifdef USES_THREADS
    pthread_t tid[core_count];
#endif
    worker_args wargs[core_count];
    
    mpfr_set_d(yslice, 0.0, MPFR_RNDN);
    mpfr_sub(yslice, Ye, Ys, MPFR_RNDN);
    mpfr_div_ui(yslice, yslice, nslice, MPFR_RNDN);

    /* initialise the global bytearray*/
    glb_bytearray = (unsigned int**)malloc(core_count*sizeof(unsigned int*));
    for(unsigned int slice=0; slice<core_count; slice++)
    {
        glb_bytearray[slice] = (unsigned int*)calloc((size_t)(xsize * ysize/core_count * 3), sizeof(unsigned int));

        wargs[slice].tid = slice;
        wargs[slice].cpus = core_count;
        wargs[slice].maxiter = maxiter;
        wargs[slice].x0 = 0;
        wargs[slice].x1 = xsize;
        wargs[slice].y0 = 0;
        wargs[slice].y1 = (ysize/nslice);
      
        mpfr_inits2(PRECISION, wargs[slice].Xe, wargs[slice].Xs, wargs[slice].Ye, wargs[slice].Ys, (mpfr_ptr)NULL);
        mpfr_set(wargs[slice].Xs, Xs, MPFR_RNDN);
        mpfr_set(wargs[slice].Xe, Xe, MPFR_RNDN);

        if (nslice > 1)
        {
            /* Ys + n*yslice */
            mpfr_mul_ui(ys1, yslice, slice, MPFR_RNDN);
            mpfr_add(wargs[slice].Ys, ys1, Ys, MPFR_RNDN);
            /* Ys + (n+1)*yslice */
            mpfr_mul_ui(ye1, yslice, slice+1, MPFR_RNDN);
            mpfr_add(wargs[slice].Ye, ye1, Ys, MPFR_RNDN);
        }
        else
        {
            mpfr_set(wargs[slice].Ys, Ys, MPFR_RNDN);
            mpfr_set(wargs[slice].Ye, Ye, MPFR_RNDN);
        }
#ifdef TRACE
    mpfr_out_str(stdout, 10, 0, Xs, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, Xe, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, Ys, MPFR_RNDN); putchar('\n');
    mpfr_out_str(stdout, 10, 0, Ye, MPFR_RNDN); putchar('\n');
    putchar('\n');
#endif
        
        if (use_threads == TRUE)
        {
#ifdef USES_THREADS
            pthread_create(&tid[slice], NULL, worker_process_slice, &(wargs[slice]));
#endif
        } 
        else 
        {
            printf("call worker directly\n");
            worker_process_slice(&(wargs[slice]));
        }
    }

    /* ------------------------------------------------------------------------- */

    /*wait for all the threads to complete */
    int** ptr;
    ptr = (int**)malloc(core_count*sizeof(int*));
    for(unsigned int slice=0; slice<core_count; slice++)
    {
#ifdef USES_THREADS
        if (use_threads == TRUE)
        {
            pthread_join(tid[slice], (void**)&ptr[slice]);
        }
#endif
    }
    
    /* populate the returned bytearray from the global one */
    TRACE_DEBUG("Combining Results\n");
    bc = 0;
    for(unsigned int slice=0; slice<core_count; slice++)
    {
        for(unsigned int gbc=0; gbc < xsize*3*(ysize/core_count); gbc++)
        {
            (*bytearray)[bc] = glb_bytearray[slice][gbc]; bc++;
        }
    }

    /* free the global bytearray*/
    TRACE_DEBUG("Freeing glb_bytearray\n");
    for(unsigned int slice=0; slice<core_count; slice++)
    {
        mpfr_clears(wargs[slice].Xe, wargs[slice].Xs, wargs[slice].Ye, wargs[slice].Ys, (mpfr_ptr)NULL);
        free(glb_bytearray[slice]); /* = (int*)calloc((size_t)(xsize/2 * ysize/2 * 3), sizeof(int));*/
    }
    free(glb_bytearray);
    free(ptr);
    
    /* clear (free) mpfr data */
    mpfr_clears(lx, ly, yslice, ys1, ye1, (mpfr_ptr)NULL);
    
#ifdef USES_THREADS
    TRACE_DEBUGV("mandelbrot_mpfr_main_c (%s) Exit\n", use_threads?"Threaded":"Single");
    printf("mandelbrot_mpfr_main_c (%s) Exit\n", use_threads?"Threaded":"Single");
#else
    printf("mandelbrot_mpfr_main_c (Single) Exit\n");
    TRACE_DEBUG("mandelbrot_mpfr_main_c (Single) Exit\n");
#endif
}

/* ----------------------------------------------------------------------------
 * call mandelbrot_mpfr_main_c above with threading enabled
 */
void mandelbrot_mpfr_thread_c( 
                const unsigned int xsize,   /* width of screen/display/window */
                const unsigned int ysize,   /* height of screen/display/window */
                const unsigned int maxiter, /* max iterations before escape */
                unsigned int **bytearray /* reference/pointer to result list of color values*/
               )
{
    mandelbrot_mpfr_main_c(xsize, ysize, maxiter, TRUE, bytearray);
}

/* ----------------------------------------------------------------------------
 * call mandelbrot_mpfr_main_c above with threading disabled
 */
void mandelbrot_mpfr_c( 
                const unsigned int xsize,   /* width of screen/display/window */
                const unsigned int ysize,   /* height of screen/display/window */
                const unsigned int maxiter, /* max iterations before escape */
                unsigned int **bytearray /* reference/pointer to result list of color values*/
               )
{
    mandelbrot_mpfr_main_c(xsize, ysize, maxiter, FALSE, bytearray);
}
