/* $Id: of_hamming_weight.c 186 2014-07-16 07:17:53Z roca $ */
/*
 * Source: Wikipedia http://en.wikipedia.org/wiki/Hamming_weight
 *
 * Creative Commons Attribution-ShareAlike 3.0 Unported License.
 *
 * You are free:
 *
 *   to Share — to copy, distribute and transmit the work
 *   to Remix — to adapt the work
 *
 * Under the following conditions:
 *
 *   Attribution — You must attribute the work in the manner specified by the author or licensor
 *                 (but not in any way that suggests that they endorse you or your use of the work).
 *   Share Alike — If you alter, transform, or build upon this work, you may distribute the
 *                  resulting work only under the same, similar or a compatible license.
 *
 * With the understanding that:
 *
 *   Waiver — Any of the above conditions can be waived if you get permission from the copyright holder.
 *   Public Domain — Where the work or any of its elements is in the public domain under applicable law,
 *                   that status is in no way affected by the license.
 *   Other Rights — In no way are any of the following rights affected by the license:
 *     Your fair dealing or fair use rights, or other applicable copyright exceptions and limitations;
 *     The author's moral rights;
 *     Rights other persons may have either in the work itself or in how the work is used, such as publicity
 *                   or privacy rights.
 *   Notice — For any reuse or distribution, you must make clear to others the license terms of this work.
 *
 * The best way to do this is with a link to this web page: http://creativecommons.org/licenses/by-sa/3.0/
 */

#include "../of_linear_binary_code.h"


#ifdef OF_USE_LINEAR_BINARY_CODES_UTILS

static const UINT64 of_m1  = 0x5555555555555555LL; //binary: 0101...
static const UINT64 of_m2  = 0x3333333333333333LL; //binary: 00110011..
static const UINT64 of_m4  = 0x0f0f0f0f0f0f0f0fLL; //binary:  4 zeros,  4 ones ...
static const UINT64 of_m8  = 0x00ff00ff00ff00ffLL; //binary:  8 zeros,  8 ones ...
static const UINT64 of_m16 = 0x0000ffff0000ffffLL; //binary: 16 zeros, 16 ones ...
static const UINT64 of_m32 = 0x00000000ffffffffLL; //binary: 32 zeros, 32 ones
static const UINT64 of_hff = 0xffffffffffffffffLL; //binary: all ones
static const UINT64 of_h01 = 0x0101010101010101LL; //the sum of 256 to the power of 0,1,2,3...


UINT8 of_hw8table[256] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8};


//This uses fewer arithmetic operations than any other known
//implementation on machines with fast multiplication.
//It uses 12 arithmetic operations, one of which is a multiply.
INT32 of_popcount_3 (UINT64 x)
{
	x -= (x >> 1) & of_m1;             //put count of each 2 bits into those 2 bits
	x = (x & of_m2) + ( (x >> 2) & of_m2);      //put count of each 4 bits into those 4 bits
	x = (x + (x >> 4)) & of_m4;        //put count of each 8 bits into those 8 bits
	return (x * of_h01) >> 56;    //returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ...
}


UINT32 of_hweight32 (UINT32 w)
{
	UINT32 res = w - ( (w >> 1) & 0x55555555);
	res = (res & 0x33333333) + ( (res >> 2) & 0x33333333);
	res = (res + (res >> 4)) & 0x0F0F0F0F;
	res = res + (res >> 8);
	return (res + (res >> 16)) & 0x000000FF;
}


UINT8 of_hweight8_table (UINT8 w)
{
	//unsigned char res;
	//res=of_hw8table[w];

	//printf("of_hw8table[%d]=%d\n",w,res);
	//return res;

	return of_hw8table[w];
}


UINT32 of_hweight32_table (UINT32 w)
{
	UINT32 res = 0;
	UINT8 *w8 = (UINT8*) (&w);
// 	for (int j = 0; j<4; j++){
// 		res+=hweight8_table(w8[j]);
// 	}
// 	res=hweight8_table(w8[0])+hweight8_table(w8[1])+hweight8_table(w8[2])+hweight8_table(w8[3]);
	res = of_hw8table[w8[0]] + of_hw8table[w8[1]] + of_hw8table[w8[2]] + of_hw8table[w8[3]];
	return res;
}


UINT32 of_hweight32_naive (UINT32 w)
{
	INT32 j;
	UINT32 res = 0;
	UINT32 x = w;
	for (j = 0; j < sizeof (UINT32); j++)
	{
		res += x & 1;
		x = x >> 1;
	}
	return res;
}


/* compute the hamming weight of an array containg 32bits words*/
UINT32 of_hweight_array (UINT32 *array, INT32 size)
{
	UINT32 weight = 0;
	UINT32 i;
	UINT32 array_size_32, array_size_32_rem;
	UINT32 *v32;

	array_size_32	= size >> 5;
	array_size_32_rem = size % 32;	// Remaining bytes
	if (array_size_32_rem > 0)
	{
		array_size_32++;
	}

#if defined (__LP64__) || (__WORDSIZE == 64)
	UINT32	array_size64;	// Size of array in 64bits unit
	UINT32	array_size64rem;
	array_size64	= array_size_32 >> 1;
	array_size64rem  = array_size_32 % 2;

	// 64-bit machines
	/* First perform as many 64-bit XORs as needed... */
	UINT64		*v64 = (UINT64*) array;	// to pointer to 64-bit integers
	for (i = array_size64; i > 0; i--)
	{
		weight += of_popcount_3 (*v64);
		v64++;
	}
	/* then perform a 32-bit XOR if needed... */
	if (array_size64rem > 0)
	{
		v32 = (UINT32*) v64;	// to pointer to 32-bit integers
		weight += of_hweight32_table (*v32);
		v32++;
	}
#else // 32-bit machines
	v32 = (UINT32*) array;
	for (i = array_size_32;i > 0;i--)
	{
		weight += of_hweight32_table (*v32);
		v32++;
	}
#endif
	return weight;
}

#endif //OF_USE_LINEAR_BINARY_CODES_UTILS
