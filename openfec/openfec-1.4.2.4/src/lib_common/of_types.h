/* $Id: of_types.h 72 2012-04-13 13:27:26Z detchart $ */
/*
 * OpenFEC.org AL-FEC Library.
 * (c) Copyright 2009 - 2012 INRIA - All rights reserved
 * Contact: vincent.roca@inria.fr
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software.  You can  use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and,  more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */


#ifndef OF_TYPES
#define OF_TYPES


#ifndef __cplusplus
#ifndef bool
#define		bool		UINT32
#define		true		1
#define		false		0
#endif
#endif /* !__cplusplus */

#ifndef UINT32
#define		INT8		char
#define		INT16		short
#define		UINT8		unsigned char
#define		UINT16		unsigned short

#if defined(__LP64__) || (__WORDSIZE == 64) /* 64 bit architectures */
#define		INT32		int		/* yes, it's also true in LP64! */
#define		UINT32		unsigned int	/* yes, it's also true in LP64! */

#else  /* 32 bit architectures */

#define		INT32		int		/* int creates fewer compilations pbs than long */
#define		UINT32		unsigned int	/* int creates fewer compilations pbs than long */
#endif /* 32/64 architectures */

#endif /* !UINT32 */

#ifndef UINT64
#ifdef WIN32
#define		INT64		__int64
#define		UINT64		__uint64
#else  /* UNIX */
#define		INT64		long long
#define		UINT64		unsigned long long
#endif /* OS */
#endif /* !UINT64 */


/**
 * gf type is used for Reed-Solomon codecs (over 2^8 and 2^m with m <=8).
 * It reprensents a Galois field element.
 */
typedef unsigned char gf;

#endif //OF_TYPES
