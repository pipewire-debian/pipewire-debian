/* $Id: of_mem.c 182 2014-07-15 09:27:51Z roca $ */
/*
 * OpenFEC.org AL-FEC Library.
 * (c) Copyright 2009 - 2011 INRIA - All rights reserved
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

#include <stdio.h>
#include <stdlib.h>
#include "of_openfec_api.h"

void* of_malloc (size_t	size)
{
	return malloc (size);
}


void* of_calloc (size_t	nmemb,
		 size_t	size)
{
	return calloc (nmemb, size);
}


void* of_realloc (void* ptr,
		  size_t size)
{
	return realloc (ptr, size);

}


void  of_free (void* ptr)
{
	if (ptr) {
		free (ptr);
	}
}


#if 0
void of_dump_buffer (char* buf, UINT32 size)
{
	UINT32	i;
	UINT32	*s32;

	if (size % 4 != 0)
	{
		size = (size % 4) * 4;
	}
	for (i = 0, s32 = (UINT32*)buf; i < size; i += 4, s32++)
	{
		printf ("%08x ", *s32);
	}
	printf ("\n");
}
#endif

