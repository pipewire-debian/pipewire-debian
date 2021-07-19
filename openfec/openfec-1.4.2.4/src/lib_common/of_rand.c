/* $Id: of_rand.c 186 2014-07-16 07:17:53Z roca $ */
//////////////////////////////////////////////////////////////////////////////
//
// rand31pmc
//
// Robin Whittle  2005 September 20
//
// 31 bit pseudo-random number generator based on:
//
//   Lehmer (1951)
//   Lewis, Goodman & Miller (1969)
//   Park & Miller (1983)
//
// implemented according to the optimisation suggested by David G. Carta
// in 1990 which uses 32 bit math and does not require division.
// Park and Miller rejected Carta's approach in 1993.  Carta provided no
// code examples.  Carta's approach produces identical results to Park
// and Miller's code.
//
// Copyright public domain . . . *but*:
//
// * Please leave the comments intact so inquiring minds have a chance of
// * understanding how this implementation works and chasing the
// * references to see the strengths and limitations of this particular
// * pseudo-random number generator.
//
// Output is a 31 bit unsigned integer.  The range of values output is
// 1 to 2,147,483,646 and the of_seed must be in this range too.  The
// output sequence repeats in a loop of this length = (2^31 - 2).
//
// The output stream has some predictable patterns.  For instance, after
// a very low output, the next one or two outputs will be relatively low
// (compared to the 2 billion range) because the multiplier is only 16,807.
// Linear congruential generators are not suitable for cryptography or
// simulation work (such as Monte Carlo Method), but they are probably
// fine for many uses where the output is sound or vision for human
// perception.
//
// The particular generator implemented here:
//
//   New-value = (old-value * 16807) mod 0x7FFFFFFF
//
// is probably the best studied linear congruentual PRNG.  It is not the very
// best, but it is far from the worst.
//
// For the background on this implementation, and the Park Miller
// "Minimal Standard" linear congruential PRNG, please see:
//
//    http://www.firstpr.com.au/dsp/rand31/
//
//    Stephen K. Park and Keith W. Miller
//    Random Number Generators: Good Ones are Hard to Find
//    Communications of the ACM, Oct 1988, Vol 31 Number 10 1192-1201
//
//    David G. Carta
//    Two Fast Implementations of the "Minimal Standard" Random Number Generator
//    Communications of the ACM, Jan 1990, Vol 33 Number 1 87-88
//
//    George Marsaglia; Stephen J. Sullivan; Stephen K. Park, Keith W. Miller,
//    Paul K. Stockmeyer
//    Remarks on Choosing and Implementing Random Number Generators
//    Communications of the ACM, Jul 1993, Vol 36 Number 7 105-110
//
//    http://random.mat.sbg.ac.at has lots of material on PRNG quality.
//
//
// The sequence of values this PRNG should produce includes:
//
//      Result     Number of results after of_seed of 1
//
//       16807          1
//   282475249          2
//  1622650073          3
//   984943658          4
//  1144108930          5
//   470211272          6
//   101027544          7
//  1457850878          8
//  1458777923          9
//  2007237709         10
//
//   925166085       9998
//  1484786315       9999
//  1043618065      10000
//  1589873406      10001
//  2010798668      10002
//
//  1227283347    1000000
//  1808217256    2000000
//  1140279430    3000000
//   851767375    4000000
//  1885818104    5000000
//
//   168075678   99000000
//  1209575029  100000000
//   941596188  101000000
//
//  1207672015 2147483643
//  1475608308 2147483644
//  1407677000 2147483645
//           1 2147483646
//       16807 2147483647
//
// Carta refers to two registers p (15 bits) and q (31 bits) which
// together hold the 46 bit multiplication product:
//
//         |                   |                   |                   |
//          4444 4444 3333 3333 3322 2222 2222 1111 1111 11
//          7654 3210 9876 5432 1098 7654 3210 9876 5432 1098 7654 3210
//
//   q 31                        qqq qqqq qqqq qqqq qqqq qqqq qqqq qqqq
//   p 15     pp pppp pppp pppp p
//
// The maximum 46 bit result occurs
// when the of_seed is at its highest
// allowable value: 0x7FFFFFFE.
//
//    0x20D37FFF7CB2
//
// which splits up like this
//
//   q 31                        111 1111 1111 1111 0111 1100 1011 0010
//   p 15     10 0000 1101 0011 0
//          =  100 0001 1010 0110
//
// In hex, these maxiumum values are:
//
//   q 31     7FFF7CB2  = 2^31 - (2 * 16807)
//   p 15         41A6  = 16807 - 1
//
//
// The task is to combine the two partial products p and q as if they were
// both parts of a 46 bit number, with the final result being modulo:
//
//                              0111 1111 1111 1111 1111 1111 1111 1111
//
// when we are actually only doing 32 bits at a time.
//
// Here I explain David G. Carta's trick - in a different and much simpler
// way than he does.
//
// We need to deal with the p bits "pp pppp pppp pppp p" shown above.
// These bits carry weights of bits 45 to 31 in the multiplication product
// of the usual Park Miller algorithm.
//
// David Carta writes that in order to calculate mod(0x7FFFFFFF) of the
// complete multiplication product (taking into account the total value
// of p and q) we should simply add the bits of p into the bit positions
// 14 to 0 of q and then do a mod(0x7FFFFFFF) on the result!
//
//         |                   |                   |                   |
//          4444 4444 3333 3333 3322 2222 2222 1111 1111 11
//          7654 3210 9876 5432 1098 7654 3210 9876 5432 1098 7654 3210
//
//     31                        qqq qqqq qqqq qqqq qqqq qqqq qqqq qqqq
//     15                   +                        ppp pppp pppp pppp
//                          =   Cxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx
//
// Highest possible value,
// for q, with a value for
// p which would allow it:
//
//                   7FFFFFFF    111 1111 1111 1111 1111 1111 1111 1111
//                +      41A5                        100 0001 1010 0101
//                = 8000041A4   1000 0000 0000 0000 0100 0001 1010 0100
//
// The result can't be larger than 2 * 0x7FFFFFFF = 0xFFFFFFFE.  So when we
// do the modulus operation, we will have to subtract either nothing or just
// one 0x7FFFFFFF.  With this model of addition, the subtraction only
// occurs very rarely.
//
// David Carta's explanation for why this produces the correct answer is too
// long to repeat here.  Mine is easy to understand.
//
// Lets define some labels:
//
//  Q = 31 bits 30 to 0.
//  P = 15 bits 14 to 0.
//
// If we were doing 46 bit math, the multiplication product (of_seed * 16807)
// would be:
//
//     Q
//  + (P * 0x80000000)
//
// Observe that this is the same as:
//
//     Q
//  + (P * 0x7FFFFFFF)
//  + (P * 0x00000001)
//
// However, we don't need or want a 46 bit result.  We only want that result
// mod(0x7FFFFFFF).  Therfore we can ignore the middle line above and use for
// our result:
//
//    Q
//  + P
//
// This is a lot snappier than using a division, as the Schrage technique
// requires.
//


#include <stdio.h>
#include "of_openfec_api.h"
#include "of_rand.h"


UINT64	of_seed;

/**
 *  Set this constant to check the conformity for this prng.
 *  Do that only in debug mode since it's time consumming.
 */
//#define CHECK_PRNG_CONFORMITY

#if CHECK_PRNG_CONFORMITY
/**
 * Check that the PRNG is compliant with [Park88].
 */
void
of_check_prng (void)
{
	OF_ENTER_FUNCTION
	INT32		i;
	UINT64	val;

	of_seed = 1;
	for (i = 0; i < 10000; i++)
	{
		val = of_rand (0x7FFFFFFF);
	}
	if (val != 1043618065)
	{
		fprintf (stderr, "ldpc_rand: ERROR, invalid PRNG, 10,000th value is %d\n", val);
		OF_EXIT_FUNCTION
		return;
	}
	else
	{
		fprintf (stderr, "ldpc_rand: okay, PRNG is valid, 10,000th value is %d\n", val);
	}
	OF_EXIT_FUNCTION
}
#endif


/**
 * Initialize the PRNG with a of_seed between 1 and 0x7FFFFFFE
 * (2^^31-2) inclusive.
 */
void of_rfc5170_srand (UINT64 s)
{
	OF_ENTER_FUNCTION
	if ( (s >= 1) && (s <= 0x7FFFFFFE))
		of_seed = s;
	else
	{
		fprintf (stderr, "ldpc_rand: ERROR, seed (%llu) out of range\n", of_seed);
		OF_EXIT_FUNCTION
		return;
	}
#if CHECK_PRNG_CONFORMITY
	check_PRNG();
	of_seed = s;
#endif
	OF_EXIT_FUNCTION
}


/**
 * Returns a random integer between 0 and maxv-1 inclusive.
 * Derived from rand31pmc, Robin Whittle, Sept 20th 2005.
 * http://www.firstpr.com.au/dsp/rand31/
 *	16807		multiplier constant (7^^5)
 *	0x7FFFFFFF	modulo constant (2^^31-1)
 * The inner PRNG produces a value between 1 and 0x7FFFFFFE
 * (2^^31-2) inclusive.
 * This value is then scaled between 0 and maxv-1 inclusive.
 * This is the PRNG required by the LDPC-staircase RFC 5170.
 */
UINT64
of_rfc5170_rand (UINT64	maxv)
{
	//OF_ENTER_FUNCTION
	UINT64	hi, lo;
	lo = 16807 * (of_seed & 0xFFFF);
	hi = 16807 * (of_seed >> 16);
	lo += (hi & 0x7FFF) << 16;
	lo += hi >> 15;
	if (lo > 0x7FFFFFFF)
		lo -= 0x7FFFFFFF;
	of_seed = (UINT64) lo;
	//OF_EXIT_FUNCTION
	return ( (UINT64)
		 ( (double) of_seed * (double) maxv / (double) 0x7FFFFFFF));
}

