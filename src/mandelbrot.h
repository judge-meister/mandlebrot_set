#ifndef MANDELBROT_MODULE_H
#define MANDELBROT_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------------------
 * Header
 */

void mandelbrot_mpfr_c( 
                const unsigned int xsize,   /* width of screen/display/window */
                const unsigned int ysize,   /* height of screen/display/window */
                const unsigned int maxiter, /* max iterations before escape */
                unsigned int **bytearray /* reference/pointer to result list of color values*/
               );

void mandelbrot_mpfr_thread_c( 
                const unsigned int xsize,   /* width of screen/display/window */
                const unsigned int ysize,   /* height of screen/display/window */
                const unsigned int maxiter, /* max iterations before escape */
                unsigned int **bytearray /* reference/pointer to result list of color values*/
               );

void mpfr_zoom_in_via_mouse( 
                const double mouse_x, /* x mouse pos in display */
                const double mouse_y, /* Y mouse pos in display */
                const unsigned int screen_width, /* display width */
                const unsigned int screen_height, /* display height */
                const unsigned int zoom_factor /* scaling factor */
               );

void mpfr_zoom_in( 
                const unsigned int screen_width, /* display width */
                const unsigned int screen_height, /* display height */
                const unsigned int zoom_factor /* scaling factor */
               );

void mpfr_zoom_out( 
                const unsigned int zoom_factor /* scaling factor */
               );

void setup_c();

void initialize_c( 
                const char* Xs_str, /* string repr of mpfr_t for X top left */
                const char* Xe_str, /* string repr of mpfr_t for X top right */
                const char* Ys_str, /* string repr of mpfr_t for Y bottom left */
                const char* Ye_str, /* string repr of mpfr_t for Y bottom right */
                const char* Cx_str, /* string repr of centre X pos for zooming in */
                const char* Cy_str  /* string repr of centre Y pos for zooming in */
               );

void free_mpfr_mem_c();

#ifdef __cplusplus
}
#endif

#endif /* MANDELBROT_MODULE_H */
