/* ----------------------------------------------------------------------------
 * Test program to exercise the functions in mandelbrot.c
 *
 * This is currently enough to provide over 99% branch coverage.
 * - use 'make coverage' to generate the results
 */ 

#include <stdio.h>
#include <stdlib.h>

#include "mandelbrot.h"

int main(int argc, char *argv[])
{
    unsigned int wsize, hsize, maxiter, factor;

    wsize = 32; //640;
    hsize = 32; //640;
    factor = 50;
    maxiter = 1000;

    printf("Testing setup_c\n");
    setup_c();
    printf("Testing initialize_c\n");
    initialize_c("-2.0", "1.0", "-1.5", "1.5", "9.0", "9.0");

    /* create an array of integers to store the result of the mandelbrot calculation */
    unsigned char *bytearray; /* [wsize * hsize * 3]; */
    bytearray = (unsigned char*)calloc((size_t)(wsize * hsize * 3), sizeof(unsigned char));

    /* call mandelbrot_mpfr_c */
    printf("Testing mandelbrot_mpfr_c\n");
    mandelbrot_mpfr_c(wsize, hsize, maxiter, &bytearray);

    printf("Testing mpfr_zoom_in\n");
    mpfr_zoom_in_via_mouse(4, 4, wsize, hsize, factor);
    mandelbrot_mpfr_c(wsize, hsize, maxiter, &bytearray);

    printf("Testing mpfr_zoom_out\n");
    mpfr_zoom_out(/*4, 4, wsize, hsize,*/ factor);
    mandelbrot_mpfr_c(wsize, hsize, maxiter, &bytearray);

    printf("Testing mpfr_zoom_in\n");
    mpfr_zoom_in_via_mouse(28, 28, wsize, hsize, factor);
    mandelbrot_mpfr_c(wsize, hsize, maxiter, &bytearray);

    printf("Testing mpfr_zoom_out\n");
    mpfr_zoom_out(/*4, 4, wsize, hsize,*/ 10);
    mandelbrot_mpfr_c(wsize, hsize, maxiter, &bytearray);

    printf("Testing mandelbrot_mpfr_thread_c\n");
    initialize_c("-2.0", "1.0", "-1.5", "1.5", "0.0", "0.0");
    mandelbrot_mpfr_thread_c(wsize, hsize, maxiter, &bytearray);

    printf("Testing mpfr_zoom_in\n");
    mpfr_zoom_in_via_mouse(4, 4, wsize, hsize, factor);
    
    printf("Testing free_mpfr_mem_c\n");
    free_mpfr_mem_c();
    free(bytearray);

    printf("Test Complete\n");
}
