/* $Id: algebra_2_8.c 185 2014-07-15 09:57:16Z roca $ */
/*
 * OpenFEC.org AL-FEC Library.
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
#include "algebra_2_8.h"

#ifdef OF_USE_REED_SOLOMON_2_M_CODEC

#define UNROLL			16 
#define USE_GF_MULC		register gf * __gf_mulc_
#define GF_MULC0(c)		__gf_mulc_ = (gf*)of_gf_2_8_mul_table[c]
#define GF_ADDMULC(dst, x)	{dst ^= __gf_mulc_[x];}

/*
 * compatibility stuff
 */
#if defined(WIN32) || defined(__ANDROID__)
#define NEED_BCOPY
#define bcmp(a,b,n) error
#endif

#ifdef NEED_BCOPY
#define bcopy(s, d, siz)        memcpy((d), (s), (siz))
#define bzero(d, siz)   memset((d), '\0', (siz))
#endif

/*
 * addmul() computes dst[] = dst[] + c * src[]
 * This is used often, so better optimize it! Currently the loop is
 * unrolled 16 times, a good value for 486 and pentium-class machines.
 * The case c=0 is also optimized, whereas c=1 is not. These
 * calls are unfrequent in my typical apps so I did not bother.
 */
void 	of_galois_field_2_8_addmul1(gf *dst1, gf *src1, gf c, int sz) {
	USE_GF_MULC ;
	register gf *dst = dst1, *src = src1 ;
	gf *lim = &dst[sz - UNROLL + 1] ;
#if ((defined (__LP64__) || (__WORDSIZE == 64)) && !defined (OF_RS_2M_USE_32BITS))	
	UINT64 tmp;
	UINT64 *dst_64 = (UINT64*)dst1;
#else
	UINT32 tmp;
	UINT32 *dst_32 = (UINT32*)dst1;
#endif
	GF_MULC0 (c) ;
	
	
#if (UNROLL > 1) /* unrolling by 8/16 is quite effective on the pentium */
	//for (; dst < lim ; dst += UNROLL, src += UNROLL)
	for (; dst < lim ;dst += UNROLL, src += UNROLL)
	{
#if ((defined (__LP64__) || (__WORDSIZE == 64)) && !defined (OF_RS_2M_USE_32BITS))		
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
#else
		tmp = ((UINT32)__gf_mulc_[src[0]]) | ((UINT32)__gf_mulc_[src[1]]<<8) | ((UINT32)__gf_mulc_[src[2]]<<16) |
		((UINT32)__gf_mulc_[src[3]]<<24);
		*dst_32 ^= tmp;
		dst_32++;
		tmp = ((UINT32)__gf_mulc_[src[4]]) | ((UINT32)__gf_mulc_[src[5]]<<8) | ((UINT32)__gf_mulc_[src[6]]<<16) |
		((UINT32)__gf_mulc_[src[7]]<<24);
		*dst_32 ^= tmp;
		dst_32++;
		tmp = ((UINT32)__gf_mulc_[src[8]]) | ((UINT32)__gf_mulc_[src[9]]<<8) | ((UINT32)__gf_mulc_[src[10]]<<16) |
		((UINT32)__gf_mulc_[src[11]]<<24);
		*dst_32 ^= tmp;
		dst_32++;		
		tmp = ((UINT32)__gf_mulc_[src[12]]) | ((UINT32)__gf_mulc_[src[13]]<<8) | ((UINT32)__gf_mulc_[src[14]]<<16) |
		((UINT32)__gf_mulc_[src[15]]<<24);
		*dst_32 ^= tmp;
		dst_32++;			
#endif		
	}
#endif
	lim += UNROLL - 1 ;
	for (; dst < lim; dst++, src++)		/* final components */
		GF_ADDMULC (*dst , *src);
}

/*
 * computes C = AB where A is n*k, B is k*m, C is n*m
 */
void 	of_galois_field_2_8_matmul (gf *a, gf *b, gf *c, int n, int k, int m) {
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
				acc ^= of_gf_2_8_mul_table[*pa][ *pb] ;
			c[ row * m + col ] = acc ;
		}
	}
	OF_EXIT_FUNCTION
}

int	of_galois_field_2_8_invert_mat (of_galois_field_code_cb_t* ofcb, gf *src, int k) {
	OF_ENTER_FUNCTION
	gf c, *p ;
	int irow, icol, row, col, i, ix ;

	int error = 1 ;
	int *indxc =	(int*) of_malloc (k * sizeof (int));
	int *indxr =	(int*) of_malloc (k * sizeof (int));
	int *ipiv =	(int*) of_malloc (k * sizeof (int));
	gf  *id_row =	(gf*)  of_malloc (1 * k * sizeof(gf));
	gf  *temp_row =	(gf*)  of_malloc (1 * k * sizeof(gf));
	
	bzero (id_row, k*sizeof (gf));
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
					if (ipiv[ix] == 0)
					{
						if (src[row*k + ix] != 0)
						{
							irow = row ;
							icol = ix ;
							goto found_piv ;
						}
					}
					else if (ipiv[ix] > 1)
					{
						// PRINT_ERR((mcl_stderr, "singular matrix\n"))
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
			c = of_gf_2_8_inv[ c ] ;
			pivot_row[icol] = 1 ;
			for (ix = 0 ; ix < k ; ix++)
				pivot_row[ix] = of_gf_2_8_mul_table[c][ pivot_row[ix]];
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
					if (c != 0)
						of_galois_field_2_8_addmul1 (p, pivot_row, c, k);
				}
			}
		}
		id_row[icol] = 0;
	} /* done all columns */
	for (col = k - 1 ; col >= 0 ; col--)
	{
		if (indxr[col] < 0 || indxr[col] >= k)
			OF_PRINT_ERROR(("AARGH, indxr[col] %d\n", indxr[col]))
		else if (indxc[col] < 0 || indxc[col] >= k)
			OF_PRINT_ERROR(( "AARGH, indxc[col] %d\n", indxc[col]))
		else if (indxr[col] != indxc[col])
		{
			for (row = 0 ; row < k ; row++)
			{
				SWAP (src[row*k + indxr[col]], src[row*k + indxc[col]], gf) ;
			}
		}
	}
	error = 0 ;
fail:
	of_free (indxc);
	of_free (indxr);
	of_free (ipiv);
	of_free (id_row);
	of_free (temp_row);
	OF_EXIT_FUNCTION
	return error ;
}

int 	of_galois_field_2_8_invert_vdm (of_galois_field_code_cb_t* ofcb, gf *src, int k) {
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
	c = (gf*) of_malloc(1 * k * sizeof(gf));
	b = (gf*) of_malloc(1 * k * sizeof(gf));
	p = (gf*) of_malloc(1 * k * sizeof(gf));

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
			c[j] ^= of_gf_2_8_mul_table[p_i][ c[j+1]] ;
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
			b[i] = c[i+1] ^ of_gf_2_8_mul_table[xx][ b[i+1]] ;
			t = of_gf_2_8_mul_table[xx][ t] ^ b[i] ;
		}
		for (col = 0 ; col < k ; col++)
			src[col*k + row] = of_gf_2_8_mul_table [of_gf_2_8_inv[t]][ b[col]];
	}
	of_free (c);
	of_free (b);
	of_free (p);
	OF_EXIT_FUNCTION
	return 0 ;
}

#endif
