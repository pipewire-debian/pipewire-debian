/* $Id: of_debug.h 72 2012-04-13 13:27:26Z detchart $ */
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

#ifndef OF_DEBUG_H
#define   OF_DEBUG_H /* { */

/****** general macros ******/


#define OF_ENTER_FUNCTION		OF_TRACE_LVL(2, ("-> %s:\n",__FUNCTION__))
#define OF_EXIT_FUNCTION		OF_TRACE_LVL(2, ("<- %s:\n",__FUNCTION__))

/*
 * Message print macros exist into two categories:
 * - OF_PRINT_*: print either to stdout or stderr, possibly controlled by a verbosity
 *   level in case of OF_PRINT_LVL. The printf code is always compiled.
 * - OF_TRACE_LVL: print debug messages, possibly controlled by a verbosity level.
 *   The printf code is only compiled in OF_DEBUG mode, there's no code otherwise.
 */

/**
 * Print to stdout.
 */
#define OF_PRINT(a)		{ printf a; fflush(stdout); }

/**
 * Print to stdout with a verbosity level.
 */
#define OF_PRINT_LVL(l, a)	if (of_verbosity >= (l)) {		\
					printf a;			\
					fflush(stdout);			\
				}

/**
 * Print to stderr.
 */
#define OF_PRINT_ERROR(a)	{ fprintf(stderr, "ERROR in \"%s\":%d:%s(): ", \
						__FILE__, __LINE__, __FUNCTION__); \
				printf a; fflush(stderr); fflush(stdout); }
#define OF_PRINT_FAILURE(a)	{ fprintf(stderr, "Failure in \"%s\":%d:%s(): ", \
						__FILE__, __LINE__, __FUNCTION__); \
				printf a; fflush(stderr); fflush(stdout); }

/**
 * Trace with a level in OF_DEBUG mode only
 */
#ifdef OF_DEBUG
#define OF_TRACE_LVL(l, a) 	if (of_verbosity >= (l)) {		\
					printf a;			\
					fflush(stdout);			\
				}
#else
#define OF_TRACE_LVL(l, a)
#endif


/**
 * assertion in OF_DEBUG mode
 */
#ifdef ASSERT
#undef ASSERT
#endif
#ifdef OF_DEBUG
#define ASSERT(c)	if (!(c)) { \
				fprintf(stderr, "ASSERT [%s:%d] failed\n", \
					__FILE__, __LINE__);		\
				fflush(stderr);				\
				exit(-1);				\
			}
#else /* OF_DEBUG */
#define ASSERT(c)
#endif /* OF_DEBUG */

#endif /* } OF_DEBUG_H */
