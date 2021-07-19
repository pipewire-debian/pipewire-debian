/* $Id: of_reed-solomon_gf_2_8.c 2 2011-03-02 11:01:37Z detchart $ */
/*
 * fec.c -- forward error correction based on Vandermonde matrices
 * 980624
 * (C) 1997-98 Luigi Rizzo (luigi@iet.unipi.it)
 *
 * Portions derived from code by Phil Karn (karn@ka9q.ampr.org),
 * Robert Morelos-Zaragoza (robert@spectra.eng.hawaii.edu) and Hari
 * Thirumoorthy (harit@spectra.eng.hawaii.edu), Aug 1995
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
#include "of_reed-solomon_gf_2_8_includes.h"

#ifdef OF_USE_REED_SOLOMON_CODEC

/* VR: added for WIN CE support */
#ifdef _WIN32_WCE
#define bzero(to,sz)	memset((to), 0, (sz))

#define bcmp(a,b,sz)	memcmp((a), (b), (sz))
#endif /* WIN32_WCE */

/*
 * compatibility stuff
 */
#if defined(WIN32) || defined(__ANDROID__)
#define NEED_BCOPY
#define bcmp(a,b,n) memcmp(a,b,n)
#endif

#ifdef NEED_BCOPY
#define bcopy(s, d, siz)        memcpy((d), (s), (siz))
#define bzero(d, siz)   memset((d), '\0', (siz))
#endif

#ifndef UINT32
#define UINT32 unsigned long
#endif

/*
 * stuff used for testing purposes only
 */

#ifdef TICK		/* VR: avoid a warning under Solaris */
#undef TICK
#endif

//#define TEST
#ifdef	TEST /* { */

#define DEB(x) x
#define DDB(x) x
#define	OF_RS_DEBUG	4	/* minimal debugging */
#if defined(WIN32)
#include <time.h>
struct timeval
{
	unsigned long ticks;
};
#define gettimeofday(x, dummy) { (x)->ticks = clock() ; }
#define DIFF_T(a,b) (1+ 1000000*(a.ticks - b.ticks) / CLOCKS_PER_SEC )
typedef unsigned long UINT32 ;
typedef unsigned short u_short ;
#else /* typically, unix systems */
#include <sys/time.h>
#define DIFF_T(a,b) \
	(1+ 1000000*(a.tv_sec - b.tv_sec) + (a.tv_usec - b.tv_usec) )
#endif

#define TICK(t) \
	{struct timeval x ; \
	gettimeofday(&x, NULL) ; \
	t = x.tv_usec + 1000000* (x.tv_sec & 0xff ) ; \
	}
#define TOCK(t) \
	{ UINT32 t1 ; TICK(t1) ; \
	  if (t1 < t) t = 256000000 + t1 - t ; \
	  else t = t1 - t ; \
	  if (t == 0) t = 1 ;}

UINT32 ticks[10];	/* vars for timekeeping */

#else  /* } { */

#define DEB(x)
#define DDB(x)
#define TICK(x)
#define TOCK(x)

#endif /* } TEST */


/*
 * You should not need to change anything beyond this point.
 * The first part of the file implements linear algebra in GF.
 *
 * gf is the type used to store an element of the Galois Field.
 * Must constain at least GF_BITS bits.
 *
 * Note: unsigned char will work up to GF(256) but int seems to run
 * faster on the Pentium. We use int whenever have to deal with an
 * index, since they are generally faster.
 */
#if (GF_BITS < 2  && GF_BITS >16)
#error "GF_BITS must be 2 .. 16"
#endif
/*#if (GF_BITS <= 8)
typedef unsigned char gf;
#else
typedef unsigned short gf;
#endif*/

#define	GF_SIZE ((1 << GF_BITS) - 1)	/* powers of \alpha */

/*
 * Primitive polynomials - see Lin & Costello, Appendix A,
 * and  Lee & Messerschmitt, p. 453.
 */
static const char *of_rs_allPp[] =      /* GF_BITS	polynomial		*/
{
	NULL,				/*  0	no code			*/
	NULL,				/*  1	no code			*/
	"111",				/*  2	1+x+x^2			*/
	"1101",				/*  3	1+x+x^3			*/
	"11001",			/*  4	1+x+x^4			*/
	"101001",			/*  5	1+x^2+x^5		*/
	"1100001",			/*  6	1+x+x^6			*/
	"10010001",			/*  7	1 + x^3 + x^7		*/
	"101110001",			/*  8	1+x^2+x^3+x^4+x^8	*/
	"1000100001",			/*  9	1+x^4+x^9		*/
	"10010000001",			/* 10	1+x^3+x^10		*/
	"101000000001",			/* 11	1+x^2+x^11		*/
	"1100101000001",		/* 12	1+x+x^4+x^6+x^12	*/
	"11011000000001",		/* 13	1+x+x^3+x^4+x^13	*/
	"110000100010001",		/* 14	1+x+x^6+x^10+x^14	*/
	"1100000000000001",		/* 15	1+x+x^15		*/
	"11010000000010001"		/* 16	1+x+x^3+x^12+x^16	*/
};


/*
 * To speed up computations, we have tables for logarithm, exponent
 * and inverse of a number. If GF_BITS <= 8, we use a table for
 * multiplication as well (it takes 64K, no big deal even on a PDA,
 * especially because it can be pre-initialized an put into a ROM!),
 * otherwhise we use a table of logarithms.
 * In any case the macro of_gf_mul(x,y) takes care of multiplications.
 */

static gf	of_rs_gf_exp[2*GF_SIZE];	/* index->poly form conversion table	*/
static int	of_rs_gf_log[GF_SIZE + 1];	/* Poly->index form conversion table	*/
static gf	of_rs_inverse[GF_SIZE+1];	/* inverse of field elem.		*/
/* inv[\alpha**i]=\alpha**(GF_SIZE-i-1)	*/

/*
 * modnn(x) computes x % GF_SIZE, where GF_SIZE is 2**GF_BITS - 1,
 * without a slow divide.
 */
static gf
of_modnn (INT32 x)
{
	while (x >= GF_SIZE)
	{
		x -= GF_SIZE;
		x = (x >> GF_BITS) + (x & GF_SIZE);
	}
	return x;
}

#define SWAP(a,b,t) {t tmp; tmp=a; a=b; b=tmp;}

/*
 * of_gf_mul(x,y) multiplies two numbers. If GF_BITS<=8, it is much
 * faster to use a multiplication table.
 *
 * USE_GF_MULC, GF_MULC0(c) and GF_ADDMULC(x) can be used when multiplying
 * many numbers by the same constant. In this case the first
 * call sets the constant, and others perform the multiplications.
 * A value related to the multiplication is held in a local variable
 * declared with USE_GF_MULC . See usage in addmul1().
 */
#if (GF_BITS <= 8)
static gf of_gf_mul_table[GF_SIZE + 1][GF_SIZE + 1];

#define of_gf_mul(x,y) of_gf_mul_table[x][y]

#define USE_GF_MULC register gf * __gf_mulc_
#define GF_MULC0(c) __gf_mulc_ = of_gf_mul_table[c]
#define GF_ADDMULC(dst, x) dst ^= __gf_mulc_[x]
#if GF_BITS <8
#define GF_ADDMULC_COMPACT(dst,x) { gf tmp=x>>4;gf _dst1=dst>>4;_dst1^=__gf_mulc_[tmp]; tmp = x & 0x0F;gf _dst2=dst & 0x0F;_dst2 ^= __gf_mulc_[tmp]; dst = _dst1<<4 | _dst2; }
#else
#define GF_ADDMULC_COMPACT(dst,x) { GF_ADDMULC(dst, x);}
#endif

static void
of_rs_init_mul_table()
{
	OF_ENTER_FUNCTION
	int i, j;
	for (i = 0; i < GF_SIZE + 1; i++)
		for (j = 0; j < GF_SIZE + 1; j++)
			of_gf_mul_table[i][j] = of_rs_gf_exp[of_modnn (of_rs_gf_log[i] + of_rs_gf_log[j]) ] ;

	for (j = 0; j < GF_SIZE + 1; j++)
		of_gf_mul_table[0][j] = of_gf_mul_table[j][0] = 0;
	OF_EXIT_FUNCTION
}
#else	/* GF_BITS > 8 */
static inline gf
of_gf_mul (x, y)
{
	if ( (x) == 0 || (y) == 0)
		return 0;

	return of_rs_gf_exp[of_rs_gf_log[x] + of_rs_gf_log[y] ] ;
}
#define of_rs_init_mul_table()

#define USE_GF_MULC register gf * __gf_mulc_
#define GF_MULC0(c) __gf_mulc_ = &of_rs_gf_exp[ of_rs_gf_log[c] ]
#define GF_ADDMULC(dst, x) { if (x) dst ^= __gf_mulc_[ of_rs_gf_log[x] ] ; }
#endif

/*
 * Generate GF(2**m) from the irreducible polynomial p(X) in p[0]..p[m]
 * Lookup tables:
 *     index->polynomial form		gf_exp[] contains j= \alpha^i;
 *     polynomial form -> index form	gf_log[ j = \alpha^i ] = i
 * \alpha=x is the primitive element of GF(2^m)
 *
 * For efficiency, gf_exp[] has size 2*GF_SIZE, so that a simple
 * multiplication of two numbers can be resolved without calling modnn
 */

/*
 * i use malloc so many times, it is easier to put checks all in
 * one place.
 */
static void *
of_my_malloc (INT32 sz, const char *err_string)
{
	OF_ENTER_FUNCTION
	void *p = malloc (sz);
	if (p == NULL)
	{
		OF_PRINT_ERROR (("-- malloc failure allocation %s\n", err_string))
		OF_EXIT_FUNCTION
		return NULL;
	}
	OF_EXIT_FUNCTION
	return p ;
}

#define NEW_GF_MATRIX(rows, cols) \
    (gf *)of_my_malloc(rows * cols * sizeof(gf), " ## __LINE__ ## " )

/*
 * initialize the data structures used for computations in GF.
 */
static void
of_generate_gf (void)
{
	OF_ENTER_FUNCTION
	INT32 i;
	gf mask;
	const char *Pp =  of_rs_allPp[GF_BITS] ;

	mask = 1;	/* x ** 0 = 1 */
	of_rs_gf_exp[GF_BITS] = 0; /* will be updated at the end of the 1st loop */
	/*
	 * first, generate the (polynomial representation of) powers of \alpha,
	 * which are stored in gf_exp[i] = \alpha ** i .
	 * At the same time build gf_log[gf_exp[i]] = i .
	 * The first GF_BITS powers are simply bits shifted to the left.
	 */
	for (i = 0; i < GF_BITS; i++, mask <<= 1)
	{
		of_rs_gf_exp[i] = mask;
		of_rs_gf_log[of_rs_gf_exp[i]] = i;
		/*
		 * If Pp[i] == 1 then \alpha ** i occurs in poly-repr
		 * gf_exp[GF_BITS] = \alpha ** GF_BITS
		 */
		if (Pp[i] == '1')
			of_rs_gf_exp[GF_BITS] ^= mask;
	}
	/*
	 * now gf_exp[GF_BITS] = \alpha ** GF_BITS is complete, so can als
	 * compute its inverse.
	 */
	of_rs_gf_log[of_rs_gf_exp[GF_BITS]] = GF_BITS;
	/*
	 * Poly-repr of \alpha ** (i+1) is given by poly-repr of
	 * \alpha ** i shifted left one-bit and accounting for any
	 * \alpha ** GF_BITS term that may occur when poly-repr of
	 * \alpha ** i is shifted.
	 */
	mask = 1 << (GF_BITS - 1) ;
	for (i = GF_BITS + 1; i < GF_SIZE; i++)
	{
		if (of_rs_gf_exp[i - 1] >= mask)
			of_rs_gf_exp[i] = of_rs_gf_exp[GF_BITS] ^ ( (of_rs_gf_exp[i - 1] ^ mask) << 1);
		else
			of_rs_gf_exp[i] = of_rs_gf_exp[i - 1] << 1;
		of_rs_gf_log[of_rs_gf_exp[i]] = i;
	}
	/*
	 * log(0) is not defined, so use a special value
	 */
	of_rs_gf_log[0] =	GF_SIZE ;
	/* set the extended gf_exp values for fast multiply */
	for (i = 0 ; i < GF_SIZE ; i++)
		of_rs_gf_exp[i + GF_SIZE] = of_rs_gf_exp[i] ;

	/*
	 * again special cases. 0 has no inverse. This used to
	 * be initialized to GF_SIZE, but it should make no difference
	 * since noone is supposed to read from here.
	 */
	of_rs_inverse[0] = 0 ;
	of_rs_inverse[1] = 1;
	for (i = 2; i <= GF_SIZE; i++)
		of_rs_inverse[i] = of_rs_gf_exp[GF_SIZE-of_rs_gf_log[i]];
	OF_EXIT_FUNCTION
}

/*
 * Various linear algebra operations that I often use.
 */

/*
 * addmul() computes dst[] = dst[] + c * src[]
 * This is used often, so better optimize it! Currently the loop is
 * unrolled 16 times, a good value for 486 and pentium-class machines.
 * The case c=0 is also optimized, whereas c=1 is not. These
 * calls are unfrequent in my typical apps so I did not bother.
 */
#define addmul(dst, src, c, sz) \
    if (c != 0) of_addmul1(dst, src, c, sz)



#if defined (__LP64__) || (__WORDSIZE == 64) // {
#define UNROLL 16	/* loop unrolling, must be equal to 16 in code below */
static void
of_addmul1 (gf *dst1, gf *src1, gf c, int sz)
{
	USE_GF_MULC ;
	register gf *dst = dst1, *src = src1 ;

	gf *lim = &dst[sz - UNROLL + 1] ;
	UINT64 tmp;
	UINT64 *dst_64 = (UINT64*)dst1;
	GF_MULC0 (c) ;
	/* with 64-bit CPUs, unroll the loop and work on two 64-bit words at a time. */
	for (; dst < lim ; dst += UNROLL, src += UNROLL)
	{

		tmp = ((UINT64)__gf_mulc_[src[0]]) | ((UINT64)__gf_mulc_[src[1]]<<8) | ((UINT64)__gf_mulc_[src[2]]<<16) |
				((UINT64)__gf_mulc_[src[3]]<<24) | ((UINT64)__gf_mulc_[src[4]]<<32) | ((UINT64)__gf_mulc_[src[5]]<<40) |
				((UINT64)__gf_mulc_[src[6]]<<48) | ((UINT64)__gf_mulc_[src[7]]<<56) ;
		*dst_64 ^= tmp;
		dst_64++;

		tmp = ((UINT64)__gf_mulc_[src[8]]) | ((UINT64)__gf_mulc_[src[9]]<<8) | ((UINT64)__gf_mulc_[src[10]]<<16) |
				((UINT64)__gf_mulc_[src[11]]<<24) | ((UINT64)__gf_mulc_[src[12]]<<32) | ((UINT64)__gf_mulc_[src[13]]<<40) |
				((UINT64)__gf_mulc_[src[14]]<<48) | ((UINT64)__gf_mulc_[src[15]]<<56) ;
		*dst_64 ^= tmp;
		dst_64++;
	}
	lim += UNROLL - 1 ;
	for (; dst < lim; dst++, src++)		/* final components */
		GF_ADDMULC (*dst , *src);
}

#else // ! (__LP64__) || (__WORDSIZE == 64)

#define UNROLL 16 /* loop unrolling. Value must be one of {1, 4, 8, 16}.
				   * However, operations remain on the basis of bytes with GF(2^^8). */
static void
of_addmul1 (gf *dst1, gf *src1, gf c, int sz)
{
	USE_GF_MULC ;
	register gf *dst = dst1, *src = src1 ;

	gf *lim = &dst[sz - UNROLL + 1] ;
	GF_MULC0 (c) ;
#if (UNROLL > 1) /* unrolling by 8/16 is quite effective on the pentium */
	for (; dst < lim ; dst += UNROLL, src += UNROLL)
	{
		GF_ADDMULC (dst[0] , src[0]);
		GF_ADDMULC (dst[1] , src[1]);
		GF_ADDMULC (dst[2] , src[2]);
		GF_ADDMULC (dst[3] , src[3]);
#if (UNROLL > 4)
		GF_ADDMULC (dst[4] , src[4]);
		GF_ADDMULC (dst[5] , src[5]);
		GF_ADDMULC (dst[6] , src[6]);
		GF_ADDMULC (dst[7] , src[7]);
#endif
#if (UNROLL > 8)
		GF_ADDMULC (dst[8] , src[8]);
		GF_ADDMULC (dst[9] , src[9]);
		GF_ADDMULC (dst[10] , src[10]);
		GF_ADDMULC (dst[11] , src[11]);
		GF_ADDMULC (dst[12] , src[12]);
		GF_ADDMULC (dst[13] , src[13]);
		GF_ADDMULC (dst[14] , src[14]);
		GF_ADDMULC (dst[15] , src[15]);
#endif
	}
#endif
	lim += UNROLL - 1 ;
	for (; dst < lim; dst++, src++)		/* final components */
		GF_ADDMULC (*dst , *src);
}

#endif //defined (__LP64__) || (__WORDSIZE == 64)

/*
 * computes C = AB where A is n*k, B is k*m, C is n*m
 */
static void
of_matmul (gf *a, gf *b, gf *c, int n, int k, int m)
{
	OF_ENTER_FUNCTION
	int row, col, i ;

	for (row = 0; row < n ; row++)
	{
		for (col = 0; col < m ; col++)
		{
			gf *pa = &a[ row * k ];
			gf *pb = &b[ col ];
			gf acc = 0 ;
			for (i = 0; i < k ; i++, pa++, pb += m)
				acc ^= of_gf_mul (*pa, *pb) ;
			c[ row * m + col ] = acc ;
		}
	}
	OF_EXIT_FUNCTION
}

#ifdef OF_RS_DEBUG
/*
 * returns 1 if the square matrix is identiy
 * (only for test)
 */
/*
static int
is_identity(gf *m, int k)
{
    int row, col ;
    for (row=0; row<k; row++)
	for (col=0; col<k; col++)
	    if ( (row==col && *m != 1) ||
		 (row!=col && *m != 0) )
		 return 0 ;
	    else
		m++ ;
    return 1 ;
}
*/
#endif /* debug */

/*
 * invert_mat() takes a matrix and produces its inverse
 * k is the size of the matrix.
 * (Gauss-Jordan, adapted from Numerical Recipes in C)
 * Return non-zero if singular.
 */
DEB (int pivloops = 0; int pivswaps = 0 ; /* diagnostic */)
static int
of_invert_mat (gf *src, int k)
{
	OF_ENTER_FUNCTION
	gf c, *p ;
	int irow, icol, row, col, i, ix ;

	int error = 1 ;
	int *indxc = (int*) of_my_malloc (k * sizeof (int), "indxc");
	int *indxr = (int*) of_my_malloc (k * sizeof (int), "indxr");
	int *ipiv = (int*) of_my_malloc (k * sizeof (int), "ipiv");
	gf *id_row = NEW_GF_MATRIX (1, k);
	gf *temp_row = NEW_GF_MATRIX (1, k);

	bzero (id_row, k*sizeof (gf));
	DEB (pivloops = 0; pivswaps = 0 ; /* diagnostic */)
	/*
	 * ipiv marks elements already used as pivots.
	 */
	for (i = 0; i < k ; i++)
		ipiv[i] = 0 ;

	for (col = 0; col < k ; col++)
	{
		gf *pivot_row ;
		/*
		 * Zeroing column 'col', look for a non-zero element.
		 * First try on the diagonal, if it fails, look elsewhere.
		 */
		irow = icol = -1 ;
		if (ipiv[col] != 1 && src[col*k + col] != 0)
		{
			irow = col ;
			icol = col ;
			goto found_piv ;
		}
		for (row = 0 ; row < k ; row++)
		{
			if (ipiv[row] != 1)
			{
				for (ix = 0 ; ix < k ; ix++)
				{
					DEB (pivloops++ ;)
					if (ipiv[ix] == 0)
					{
						if (src[row*k + ix] != 0)
						{
							irow = row ;
							icol = ix ;
							goto found_piv ;
						}
					}
					else
						if (ipiv[ix] > 1)
						{
//			PRINT_ERR((mcl_stderr, "singular matrix\n"))
							goto fail ;
						}
				}
			}
		}
		if (icol == -1)
		{
//	    PRINT_ERR((mcl_stderr, "XXX pivot not found!\n"))
			goto fail ;
		}
found_piv:
		++ (ipiv[icol]) ;
		/*
		 * swap rows irow and icol, so afterwards the diagonal
		 * element will be correct. Rarely done, not worth
		 * optimizing.
		 */
		if (irow != icol)
		{
			for (ix = 0 ; ix < k ; ix++)
			{
				SWAP (src[irow*k + ix], src[icol*k + ix], gf) ;
			}
		}
		indxr[col] = irow ;
		indxc[col] = icol ;
		pivot_row = &src[icol*k] ;
		c = pivot_row[icol] ;
		if (c == 0)
		{
			OF_PRINT_ERROR(("singular matrix 2\n"))
			goto fail ;
		}
		if (c != 1)
		{
			/* otherwhise this is a NOP */
			/*
			 * this is done often , but optimizing is not so
			 * fruitful, at least in the obvious ways (unrolling)
			 */
			DEB (pivswaps++ ;)
			c = of_rs_inverse[ c ] ;
			pivot_row[icol] = 1 ;
			for (ix = 0 ; ix < k ; ix++)
				pivot_row[ix] = of_gf_mul (c, pivot_row[ix]);
		}
		/*
		 * from all rows, remove multiples of the selected row
		 * to zero the relevant entry (in fact, the entry is not zero
		 * because we know it must be zero).
		 * (Here, if we know that the pivot_row is the identity,
		 * we can optimize the addmul).
		 */
		id_row[icol] = 1;
		if (bcmp (pivot_row, id_row, k*sizeof (gf)) != 0)
		{
			for (p = src, ix = 0 ; ix < k ; ix++, p += k)
			{
				if (ix != icol)
				{
					c = p[icol] ;
					p[icol] = 0 ;
					addmul (p, pivot_row, c, k);
				}
			}
		}
		id_row[icol] = 0;
	} /* done all columns */
	for (col = k - 1 ; col >= 0 ; col--)
	{
		if (indxr[col] < 0 || indxr[col] >= k)
			OF_PRINT_ERROR(("AARGH, indxr[col] %d\n", indxr[col]))
			else
				if (indxc[col] < 0 || indxc[col] >= k)
					OF_PRINT_ERROR(( "AARGH, indxc[col] %d\n", indxc[col]))
					else
						if (indxr[col] != indxc[col])
						{
							for (row = 0 ; row < k ; row++)
							{
								SWAP (src[row*k + indxr[col]], src[row*k + indxc[col]], gf) ;
							}
						}
	}
	error = 0 ;
fail:
	free (indxc);
	free (indxr);
	free (ipiv);
	free (id_row);
	free (temp_row);
	OF_EXIT_FUNCTION
	return error ;
}

/*
 * fast code for inverting a vandermonde matrix.
 * XXX NOTE: It assumes that the matrix
 * is not singular and _IS_ a vandermonde matrix. Only uses
 * the second column of the matrix, containing the p_i's.
 *
 * Algorithm borrowed from "Numerical recipes in C" -- sec.2.8, but
 * largely revised for my purposes.
 * p = coefficients of the matrix (p_i)
 * q = values of the polynomial (known)
 */

int
of_invert_vdm (gf *src, int k)
{
	OF_ENTER_FUNCTION
	int i, j, row, col ;
	gf *b, *c, *p;
	gf t, xx ;

	if (k == 1) 	/* degenerate case, matrix must be p^0 = 1 */
		return 0 ;
	/*
	 * c holds the coefficient of P(x) = Prod (x - p_i), i=0..k-1
	 * b holds the coefficient for the matrix inversion
	 */
	c = NEW_GF_MATRIX (1, k);
	b = NEW_GF_MATRIX (1, k);

	p = NEW_GF_MATRIX (1, k);

	for (j = 1, i = 0 ; i < k ; i++, j += k)
	{
		c[i] = 0 ;
		p[i] = src[j] ;    /* p[i] */
	}
	/*
	 * construct coeffs. recursively. We know c[k] = 1 (implicit)
	 * and start P_0 = x - p_0, then at each stage multiply by
	 * x - p_i generating P_i = x P_{i-1} - p_i P_{i-1}
	 * After k steps we are done.
	 */
	c[k-1] = p[0] ;	/* really -p(0), but x = -x in GF(2^m) */
	for (i = 1 ; i < k ; i++)
	{
		gf p_i = p[i] ; /* see above comment */
		for (j = k - 1  - (i - 1) ; j < k - 1 ; j++)
			c[j] ^= of_gf_mul (p_i, c[j+1]) ;
		c[k-1] ^= p_i ;
	}

	for (row = 0 ; row < k ; row++)
	{
		/*
		 * synthetic division etc.
		 */
		xx = p[row] ;
		t = 1 ;
		b[k-1] = 1 ; /* this is in fact c[k] */
		for (i = k - 2 ; i >= 0 ; i--)
		{
			b[i] = c[i+1] ^ of_gf_mul (xx, b[i+1]) ;
			t = of_gf_mul (xx, t) ^ b[i] ;
		}
		for (col = 0 ; col < k ; col++)
			src[col*k + row] = of_gf_mul (of_rs_inverse[t], b[col]);
	}
	free (c) ;
	free (b) ;
	free (p) ;
	OF_EXIT_FUNCTION
	return 0 ;
}

static int of_rs_initialized = 0 ;
/* static */
void		/* VR: removed static */
of_rs_init()
{
	OF_ENTER_FUNCTION
	TICK (ticks[0]);
	of_generate_gf();
	TOCK (ticks[0]);
	DDB (printf("generate_gf took %ldus\n", ticks[0]);)
	TICK (ticks[0]);
	of_rs_init_mul_table();
	TOCK (ticks[0]);
	DDB (printf("init_mul_table took %ldus\n", ticks[0]);)
	of_rs_initialized = 1 ;
	OF_EXIT_FUNCTION
}

/*
 * This section contains the proper FEC encoding/decoding routines.
 * The encoding matrix is computed starting with a Vandermonde matrix,
 * and then transforming it into a systematic matrix.
 */

#define FEC_MAGIC	0xFECC0DEC

struct fec_parms
{
	UINT32 magic ;
	INT32 k, n ;		/* parameters of the code */
	gf *enc_matrix ;
} ;


#define CPLUSPLUS_COMPATIBLE				/* VR: added */
#ifdef CPLUSPLUS_COMPATIBLE
void of_rs_free (void *p_vp)
#else
void of_rs_free (struct fec_parms *p)
#endif /* CPLUSPLUS_COMPATIBLE */
{
	OF_ENTER_FUNCTION
#ifdef CPLUSPLUS_COMPATIBLE
	struct fec_parms *p = (struct fec_parms *) p_vp;	/* VR */
#endif /* CPLUSPLUS_COMPATIBLE */
	if (p == NULL) //||
			//p->magic != ( ( (FEC_MAGIC ^ p->k) ^ p->n) ^ (int)(p->enc_matrix)) ) {
			//p->magic != (( (FEC_MAGIC ^ p->k) ^ p->n) ^ (UINT32) (p->enc_matrix)))
	{
		OF_PRINT_ERROR (("bad parameters to fec_free\n"))
		return ;
	}
	free (p->enc_matrix);
	free (p);
	OF_EXIT_FUNCTION
}

/*
 * create a new encoder, returning a descriptor. This contains k,n and
 * the encoding matrix.
 */
#if 0		/* VR: changed as it creates problems with C++ compilers */
struct fec_parms *
#else
void *
#endif
of_rs_new (UINT32 k, UINT32 n)
{
	OF_ENTER_FUNCTION
	INT32 row, col ;
	gf *p, *tmp_m ;

	struct fec_parms *retval ;

	if (of_rs_initialized == 0)
		of_rs_init();

	if (k > GF_SIZE + 1 || n > GF_SIZE + 1 || k > n)
	{
		OF_PRINT_ERROR(( "Invalid parameters k %d n %d GF_SIZE %d\n",
			     k, n, GF_SIZE))
		OF_EXIT_FUNCTION
		return NULL ;
	}
	retval = (struct fec_parms*) of_my_malloc (sizeof (struct fec_parms), "new_code");
	retval->k = k ;
	retval->n = n ;
	retval->enc_matrix = NEW_GF_MATRIX (n, k);
	/* cast the pointer (a 64 bit integer on LP64 systems) to uintptr_t first, and then
	 * truncate it to keep only the lowest 32 bits. Works the same on both 32 bit and 64
	 * bit systems */
	//retval->magic = ( ( FEC_MAGIC ^ k) ^ n) ^ (int)(retval->enc_matrix) ;
	retval->magic = ( (FEC_MAGIC ^ k) ^ n) ^ ((uintptr_t)(retval->enc_matrix) & 0xFFFFFFFF);
	tmp_m = NEW_GF_MATRIX (n, k);
	/*
	 * fill the matrix with powers of field elements, starting from 0.
	 * The first row is special, cannot be computed with exp. table.
	 */
	tmp_m[0] = 1 ;
	for (col = 1; col < k ; col++)
		tmp_m[col] = 0 ;
	for (p = tmp_m + k, row = 0; row < n - 1 ; row++, p += k)
	{
		for (col = 0 ; col < k ; col ++)
			p[col] = of_rs_gf_exp[of_modnn (row*col) ];
	}

	/*
	 * quick code to build systematic matrix: invert the top
	 * k*k vandermonde matrix, multiply right the bottom n-k rows
	 * by the inverse, and construct the identity matrix at the top.
	 */
	TICK (ticks[3]);
	of_invert_vdm (tmp_m, k);   /* much faster than invert_mat */
	of_matmul (tmp_m + k*k, tmp_m, retval->enc_matrix + k*k, n - k, k, k);
	/*
	 * the upper matrix is I so do not bother with a slow multiply
	 */
	bzero (retval->enc_matrix, k*k*sizeof (gf));
	for (p = retval->enc_matrix, col = 0 ; col < k ; col++, p += k + 1)
		*p = 1 ;
	free (tmp_m);
	TOCK (ticks[3]);

	DDB (printf( "--- %ld us to build encoding matrix\n", ticks[3]);)
	//DEB (pr_matrix (retval->enc_matrix, n, k, "encoding_matrix");)
	OF_EXIT_FUNCTION
#if 0		/* VR: changed as it creates problems with C++ compilers */
	return retval ;
#else
	return (void*) retval ;
#endif
}

/*
 * of_rs_encode accepts as input pointers to n data packets of size sz,
 * and produces as output a packet pointed to by fec, computed
 * with index "index".
 */
/*
 * VR: changed for C++ compilers who don't accept diff in parameters...
 * Use a definition that matches prototype in fec.h
 */
#define CPLUSPLUS_COMPATIBLE			/* VR: added */
#ifdef CPLUSPLUS_COMPATIBLE
of_status_t
of_rs_encode (void *code_vp, void **src_vp, void *fec_vp, int index, int sz)
#else
of_status_t
of_rs_encode (struct fec_parms *code, gf *src[], gf *fec, int index, int sz)
#endif
{
	OF_ENTER_FUNCTION
#ifdef CPLUSPLUS_COMPATIBLE
	struct fec_parms *code = (struct fec_parms*) code_vp;  /* VR */
	gf **src = (gf**) src_vp;				/* VR */
	gf *fec = (gf*) fec_vp;				/* VR */
#endif /* CPLUSPLUS_COMPATIBLE */
	int i, k = code->k ;
	gf *p ;

	if (GF_BITS > 8)
		sz /= 2 ;

	if (index < k)
		bcopy (src[index], fec, sz*sizeof (gf)) ;
	else if (index < code->n)
	{
		p = & (code->enc_matrix[index*k]);
		bzero (fec, sz*sizeof (gf));
		for (i = 0; i < k ; i++)
			addmul (fec, src[i], p[i], sz) ;
		return OF_STATUS_OK;
	}
	else
	{
		OF_PRINT_ERROR (("Invalid index %d (max %d)\n", index, code->n - 1))
	}
	OF_EXIT_FUNCTION
	return OF_STATUS_ERROR;
}

/*
 * shuffle move src packets in their position
 */
static int
of_shuffle (gf *pkt[], int index[], int k)
{
	OF_ENTER_FUNCTION
	int i;

	for (i = 0 ; i < k ;)
	{
		if (index[i] >= k || index[i] == i)
			i++ ;
		else
		{
			/*
			 * put pkt in the right position (first check for conflicts).
			 */
			int c = index[i] ;

			if (index[c] == c)
			{
				DEB (printf("\nshuffle, error at %d\n", i);)
				OF_EXIT_FUNCTION
				return 1 ;
			}
			SWAP (index[i], index[c], int) ;
			SWAP (pkt[i], pkt[c], gf *) ;
		}
	}
	DEB ( /* just test that it works... */
		for (i = 0 ; i < k ; i++)
		{
			if (index[i] < k && index[i] != i)
			{
				printf("shuffle: after\n");
				for (i = 0; i < k ; i++)
					printf ("%3d ", index[i]);
					printf ("\n");
					OF_EXIT_FUNCTION
					return 1 ;
			}
		}
	)
	OF_EXIT_FUNCTION
	return 0 ;
}


/*
 * build_decode_matrix constructs the encoding matrix given the
 * indexes. The matrix must be already allocated as
 * a vector of k*k elements, in row-major order
 */
static gf *
of_build_decode_matrix (struct fec_parms *code, gf *pkt[], int index[])
{
	OF_ENTER_FUNCTION
	int i , k = code->k ;
	gf *p, *matrix = NEW_GF_MATRIX (k, k);

	TICK (ticks[9]);
	for (i = 0, p = matrix ; i < k ; i++, p += k)
	{
#if 1 /* this is simply an optimization, not very useful indeed */
		if (index[i] < k)
		{
			bzero (p, k*sizeof (gf));
			p[i] = 1 ;
		}
		else
#endif
			if (index[i] < code->n)
				bcopy (& (code->enc_matrix[index[i]*k]), p, k*sizeof (gf));
			else
			{
				OF_PRINT_ERROR ( ("decode: invalid index %d (max %d)\n",
					     index[i], code->n - 1))
				free (matrix);
				OF_EXIT_FUNCTION
				return NULL ;
			}
	}
	TICK (ticks[9]);
	if (of_invert_mat (matrix, k))
	{
		free (matrix);
		matrix = NULL ;
	}
	TOCK (ticks[9]);
	OF_EXIT_FUNCTION
	return matrix ;
}

/*
 * of_rs_decode receives as input a vector of packets, the indexes of
 * packets, and produces the correct vector as output.
 *
 * Input:
 *	code: pointer to code descriptor
 *	pkt:  pointers to received packets. They are modified
 *	      to store the output packets (in place)
 *	index: pointer to packet indexes (modified but not updated upon return)
 *	sz:    size of each packet
 */
/*
 * VR: changed for C++ compilers who don't accept diff in parameters...
 * Use a definition that matches prototype in fec.h
 */
#define CPLUSPLUS_COMPATIBLE			/* VR: added */
#ifdef CPLUSPLUS_COMPATIBLE
of_status_t
of_rs_decode (void *code_vp, void **pkt_vp, int index[], int sz)
#else
int
of_rs_decode (struct fec_parms *code, gf *pkt[], int index[], int sz)
#endif
{
	OF_ENTER_FUNCTION
#ifdef CPLUSPLUS_COMPATIBLE
	struct fec_parms *code = (struct fec_parms*) code_vp;  /* VR */
	gf **pkt = (gf**) pkt_vp;				/* VR */
#endif /* CPLUSPLUS_COMPATIBLE */
	gf *m_dec ;
	gf **new_pkt ;
	int row, col, k = code->k ;

	if (GF_BITS > 8)
		sz /= 2 ;

	if (of_shuffle (pkt, index, k))
	{
		/* error if true */
		OF_EXIT_FUNCTION
		return OF_STATUS_ERROR ;
	}
	m_dec = of_build_decode_matrix (code, pkt, index);

	if (m_dec == NULL)
	{
		OF_EXIT_FUNCTION
		return OF_STATUS_ERROR ; /* error */
	}
	/*
	 * do the actual decoding
	 */
	new_pkt = (gf **) of_my_malloc (k * sizeof (gf *), "new pkt pointers");
	for (row = 0 ; row < k ; row++)
	{
		if (index[row] >= k)
		{
			new_pkt[row] = (gf *) of_my_malloc (sz * sizeof (gf), "new pkt buffer");
			bzero (new_pkt[row], sz * sizeof (gf)) ;
			for (col = 0 ; col < k ; col++)
			{
				addmul (new_pkt[row], pkt[col], m_dec[row*k + col], sz) ;
			}
		}
	}
	/*
	 * move pkts to their final destination
	 * Warning: this function does not update the index[] table to contain
	 * the actual reconstructed packet index.
	 */
	for (row = 0 ; row < k ; row++)
	{
		if (index[row] >= k)
		{
			bcopy (new_pkt[row], pkt[row], sz*sizeof (gf));
			free (new_pkt[row]);
		}
	}
	free (new_pkt);
	free (m_dec);

	OF_EXIT_FUNCTION
	return OF_STATUS_OK;
}

/*********** end of FEC code -- beginning of test code ************/

#if defined(TEST) /* || OF_RS_DEBUG*/
void
test_gf()
{
	OF_ENTER_FUNCTION
	int i ;
	/*
	 * test gf tables. Sufficiently tested...
	 */
	for (i = 0; i <= GF_SIZE; i++)
	{
		if (of_rs_gf_exp[of_rs_gf_log[i]] != i)
			OF_PRINT_ERROR(("bad exp/log i %d log %d exp(log) %d\n",
				     i, of_rs_gf_log[i], of_rs_gf_exp[of_rs_gf_log[i]]))

			if (i != 0 && of_gf_mul (i, of_rs_inverse[i]) != 1)
				OF_PRINT_ERROR(( "bad mul/inv i %d inv %d i*inv(i) %d\n",
					     i, of_rs_inverse[i], of_gf_mul (i, of_rs_inverse[i])))
				if (of_gf_mul (0, i) != 0)
					OF_PRINT_ERROR(("bad mul table 0,%d\n", i))
					if (of_gf_mul (i, 0) != 0)
						OF_PRINT_ERROR(("bad mul table %d,0\n", i))
					}
	OF_EXIT_FUNCTION
}
#endif /* TEST */

#endif //#ifdef OF_USE_REED_SOLOMON_CODEC
