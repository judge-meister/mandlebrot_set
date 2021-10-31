
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

    setup_c();
    initialize_c("-2.0", "1.0", "-1.5", "1.5");

    /* create an array of integers to store the result of the mandlebrot calculation */
    int bytearray[wsize * hsize * 3];

    /* call mandlebrot_bytearray */
    mandlebrot_mpfr_c(wsize, hsize, maxiter, bytearray);
    mpfr_zoom_in(4, 4, wsize, hsize, factor);
    mandlebrot_mpfr_c(wsize, hsize, maxiter, bytearray);

    free_mpfr_mem_c();

    printf("End\n");
}
