#ifndef Py_MANDLEBROTMODULE_H
#define Py_MANDLEBROTMODULE_H

#ifdef __cplusplus
extern "C" {
#endif
/* ----------------------------------------------------------------------------
 * Header
 */
void mandlebrot_bytearray_c(const unsigned int wsize,   /* width of screen/display/window */
                            const unsigned int hsize,   /* height of screen/display/window */
                            const unsigned int maxiter, /* max iterations before escape */
                            const double Xs, /* string repr of mpfr_t for X top left */
                            const double Xe, /* string repr of mpfr_t for X top right */
                            const double Ys, /* string repr of mpfr_t for Y bottom left */
                            const double Ye, /* string repr of mpfr_t for Y bottom right */
                            int bytearray[] /* reference/pointer to result list of color values*/
                           );

void mandlebrot_mpfr_c( const unsigned int xsize,   /* width of screen/display/window */
                        const unsigned int ysize,   /* height of screen/display/window */
                        const unsigned int maxiter, /* max iterations before escape */
                        int bytearray[] /* reference/pointer to result list of color values*/
                       );

void mandlebrot_mpfr_slice_c( const unsigned int xsize,   /* width of screen/display/window */
                              const unsigned int ysize,   /* height of screen/display/window */
                              const unsigned int nslice,  /* number of slices */
                              const unsigned int slice,   /* which slice (range 0 -> nslice-1) */
                              const unsigned int maxiter, /* max iterations before escape */
                              int bytearray[] /* reference/pointer to result list of color values*/
                          );

void mpfr_zoom_in( const unsigned int pX, /* */
                   const unsigned int pY, /* */
                   const unsigned int w, /* */
                   const unsigned int h, /* */
                   const unsigned int factor /* */
                  );

void mpfr_zoom_out( const unsigned int pX, /* */
                    const unsigned int pY, /* */
                    const unsigned int w, /* */
                    const unsigned int h, /* */
                    const unsigned int factor /* */
                   );

void setup_c();

void initialize_c( const char* Xs_str, /* string repr of mpfr_t for X top left */
                   const char* Xe_str, /* string repr of mpfr_t for X top right */
                   const char* Ys_str, /* string repr of mpfr_t for Y bottom left */
                   const char* Ye_str  /* string repr of mpfr_t for Y bottom right */
                  );

void free_mpfr_mem_c();

#ifdef __cplusplus
}
#endif

#endif /* PyMANDLEBROTMODULE_H */