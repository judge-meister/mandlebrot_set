
#include <stdio.h>
#include <stdlib.h>

#include "mandlebrot.h"

int main(int argc, char *argv[])
{
    unsigned int wsize, hsize, maxiter, factor;

    wsize = 640; //640;
    hsize = 640; //640;
    factor = 5;
    maxiter = 1000;

    setup_c();
    initialize_c("-2.0", "1.0", "-1.5", "1.5");

    /* create an array of integers to store the result of the mandlebrot calculation */
    int bytearray[wsize * hsize * 3];

    /* call mandlebrot_bytearray */
    mandlebrot_mpfr_c(wsize, hsize, maxiter, bytearray);
    mpfr_zoom_in(4, 4, wsize, hsize, factor);
    mandlebrot_mpfr_c(wsize, hsize, maxiter, bytearray);
    mpfr_zoom_out(4, 4, wsize, hsize, factor);
    mandlebrot_mpfr_c(wsize, hsize, maxiter, bytearray);

    mandlebrot_mpfr_slice_c(wsize, hsize, 4, 0, maxiter, bytearray);

    unsigned int i = 0;
    printf("Testing bytarray 0\n");
    for(i=0; i < wsize*(hsize/4)*3; i++)
    {
        if (bytearray[i] > 255) { printf("%d ", bytearray[i]); }
    }

    mandlebrot_mpfr_slice_c(wsize, hsize, 4, 1, maxiter, bytearray);

    printf("Testing bytarray 1\n");
    for(i=0; i < wsize*(hsize/4)*3; i++)
    {
        if (bytearray[i] > 255) { printf("%d ", bytearray[i]); }
    }

    mandlebrot_mpfr_slice_c(wsize, hsize, 4, 2, maxiter, bytearray);

    printf("Testing bytarray 2\n");
    for(i=0; i < wsize*(hsize/4)*3; i++)
    {
        if (bytearray[i] > 255) { printf("%d ", bytearray[i]); }
    }

    mandlebrot_mpfr_slice_c(wsize, hsize, 4, 3, maxiter, bytearray);

    printf("Testing bytarray 3\n");
    for(i=0; i < wsize*(hsize/4)*3; i++)
    {
        if (bytearray[i] > 255) { printf("%d ", bytearray[i]); }
    }

    free_mpfr_mem_c();

    printf("End\n");
}
