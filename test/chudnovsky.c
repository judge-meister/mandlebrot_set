
/*
 * Calculate pi using the Chudnovsky method utilising the mpfr library
 *
   gcc -o chudnovsky -lmpfr chudnovsky.c 
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpfr.h>

#define PREC (mpfr_prec_t)(1024*5)

mpfr_t chud_frac;

void chudnovsky_fraction(unsigned int q)
{
	mpfr_t top, bot, a, b;
	mpfr_inits2(PREC, top, bot, chud_frac, a, b, (mpfr_ptr)0);
	mpfr_set_d(chud_frac, 0.0, MPFR_RNDN);
	
	// (-1)^q * (6q)! * (545140134q + 13591409)
	mpfr_set_si(top, ((545140134*q) + 13591409), MPFR_RNDN);
	mpfr_fac_ui(a, (6*q), MPFR_RNDN);
	mpfr_mul(top, top, a, MPFR_RNDN);
	mpfr_set_si(a, -1, MPFR_RNDN);
	mpfr_pow_ui(a, a, q, MPFR_RNDN);
	mpfr_mul(top, top, a, MPFR_RNDN);
	//printf("top ");
	//mpfr_out_str(stdout, 10, 0, top, MPFR_RNDN);
	//printf("\n");
	
	// (3q)! * (q!)^3  * (640320)^(3q+3/2)
	mpfr_set_d(a, ((3.0*q) + (3.0/2.0)), MPFR_RNDN);
	mpfr_ui_pow(bot, 640320, a, MPFR_RNDN);
	mpfr_fac_ui(a, q, MPFR_RNDN);
	mpfr_pow_si(a, a, 3, MPFR_RNDN);
	mpfr_mul(bot, bot, a, MPFR_RNDN);
	mpfr_fac_ui(a, (3*q), MPFR_RNDN);
	mpfr_mul(bot, bot, a, MPFR_RNDN);
	//printf("bot ");
	//mpfr_out_str(stdout, 10, 0, bot, MPFR_RNDN);
	//printf("\n");
	
	mpfr_div(chud_frac, top, bot, MPFR_RNDN);
}

void chudnovsky(unsigned int iterations)
{
	unsigned int q = 0;
	mpfr_t sum;
	
	mpfr_inits2(PREC, sum, (mpfr_ptr)0);
	mpfr_set_d(sum, 0.0, MPFR_RNDN);
	
	for(q=0; q < iterations; q++)
	{
		chudnovsky_fraction(q);
		mpfr_out_str(stderr, 10, 0, chud_frac, MPFR_RNDN);
		fprintf(stderr,"\n");
		mpfr_add(sum, sum, chud_frac, MPFR_RNDN);
	}
	
	mpfr_mul_si(sum, sum, 12, MPFR_RNDN);
	mpfr_d_div(sum, 1.0, sum, MPFR_RNDN);
	
	mpfr_out_str(stdout, 10, 0, sum, MPFR_RNDN);
	printf("\n");
}

int main()
{
	unsigned int m = 0;
	
	/*for (m=50; m<100; m++)
	{
		chudnovsky(m);
	}*/
	chudnovsky(100);
}