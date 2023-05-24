/* ----------------------------------------------------------------------------
 * Test program to exercise the functions in mandelbrot.c
 *
 * This is currently enough to provide over 99% branch coverage.
 * - use 'make coverage' to generate the results
 */ 

#include <stdio.h>
#include <stdlib.h>

#include "mandlebrot.h"

int main(int argc, char *argv[])
{
    unsigned int wsize, hsize, maxiter, factor;

    wsize = 32; //640;
    hsize = 32; //640;
    factor = 5;
    maxiter = 1000;

    printf("Testing setup_c\n");
    setup_c();
    printf("Testing initialize_c\n");
    initialize_c("-2.0", "1.0", "-1.5", "1.5", "9.0", "9.0");

    /* create an array of integers to store the result of the mandlebrot calculation */
    int *bytearray; /* [wsize * hsize * 3]; */
    bytearray = (int*)calloc((size_t)(wsize * hsize * 3), sizeof(int));

    /* call mandlebrot_bytearray */
    printf("Testing mandlebrot_bytearray_c\n");
    mandlebrot_bytearray_c(wsize, hsize, maxiter, -2.0, 1.0, -1.5, 1.5, &bytearray);

    /* call mandlebrot_mpfr_c */
    printf("Testing mandlebrot_mpfr_c\n");
    mandlebrot_mpfr_c(wsize, hsize, maxiter, &bytearray);

    printf("Testing mpfr_zoom_in\n");
    mpfr_zoom_in(4, 4, wsize, hsize, factor);
    mandlebrot_mpfr_c(wsize, hsize, maxiter, &bytearray);

    printf("Testing mpfr_zoom_out\n");
    mpfr_zoom_out(4, 4, wsize, hsize, factor);
    mandlebrot_mpfr_c(wsize, hsize, maxiter, &bytearray);

    printf("Testing mandlebrot_mpfr_slice_c - slice 0\n");
    mandlebrot_mpfr_slice_c(wsize, hsize, 4, 0, maxiter, &bytearray);

    unsigned int i = 0;
    printf("Testing bytearray 0\n");
    for(i=0; i < wsize*(hsize/4)*3; i++)
    {
        if (bytearray[i] > 255) { printf("%d ", bytearray[i]); }
    }

    printf("Testing mandlebrot_mpfr_slice_c - slice 1\n");
    mandlebrot_mpfr_slice_c(wsize, hsize, 4, 1, maxiter, &bytearray);

    printf("Testing bytearray 1\n");
    for(i=0; i < wsize*(hsize/4)*3; i++)
    {
        if (bytearray[i] > 255) { printf("%d ", bytearray[i]); }
    }

    printf("Testing mandlebrot_mpfr_slice_c - slice 2\n");
    mandlebrot_mpfr_slice_c(wsize, hsize, 4, 2, maxiter, &bytearray);

    printf("Testing bytearray 2\n");
    for(i=0; i < wsize*(hsize/4)*3; i++)
    {
        if (bytearray[i] > 255) { printf("%d ", bytearray[i]); }
    }

    printf("Testing mandlebrot_mpfr_slice_c - slice 3\n");
    mandlebrot_mpfr_slice_c(wsize, hsize, 4, 3, maxiter, &bytearray);

    printf("Testing bytearray 3\n");
    for(i=0; i < wsize*(hsize/4)*3; i++)
    {
        if (bytearray[i] > 255) { printf("%d ", bytearray[i]); }
    }
    
    printf("Testing mandlebrot_mpfr_thread_c\n");
    initialize_c("-2.0", "1.0", "-1.5", "1.5", "0.0", "0.0");
    mandlebrot_mpfr_thread_c(wsize, hsize, maxiter, &bytearray);

    printf("Testing mpfr_zoom_in\n");
    mpfr_zoom_in(4, 4, wsize, hsize, factor);
    
    printf("Testing free_mpfr_mem_c\n");
    free_mpfr_mem_c();
    free(bytearray);

    printf("Test Complete\n");
}
