/* $Id: simple_client_server.h 207 2014-12-10 19:47:50Z roca $ */
/*
 * OpenFEC.org AL-FEC Library.
 * (c) Copyright 2009-2014 INRIA - All rights reserved
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
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h> 
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>	/* for gettimeofday */

#include "../../../src/lib_common/of_openfec_api.h"

/*
 * OS dependant definitions
 */
#define SOCKET		int
#define SOCKADDR	struct sockaddr
#define SOCKADDR_IN	struct sockaddr_in
#define INVALID_SOCKET	(-1)
#define SOCKET_ERROR	(-1)
#define closesocket	close
#define SLEEP(t)	usleep(t*1000)


/*
 * Simulation parameters...
 * Change as required
 */
#define SYMBOL_SIZE	1024		/* symbol size, in bytes (must be multiple of 4 in this simple example) */
#define	DEFAULT_K	100		/* default k value */
#define CODE_RATE	0.667		/* k/n = 2/3 means we add 50% of repair symbols */
#define LOSS_RATE	0.30		/* we consider 30% of packet losses... It assumes there's no additional loss during UDP transmissions */

#define VERBOSITY	2		/* Define the verbosity level:
					 *	0 : no trace
					 *	1 : main traces
					 *	2 : full traces with packet dumps */

#define DEST_IP		"127.0.0.1"	/* Destination IPv4 address */
#define DEST_PORT	10978		/* Destination port (UDP) */


/*
 * Simplified FEC Object Transmission Information structure, used to synchronize sender and receiver.
 *
 * NB: all the fields MUST be in Network Endian while sent over the network, so use htonl (resp. ntohl) at the sender (resp. receiver).
 */
typedef struct {
	UINT32		codec_id;	/* identifies the code/codec being used. In practice, the "FEC encoding ID" that identifies the FEC Scheme should
					 * be used instead (see [RFC5052]). In our example, we are not compliant with the RFCs anyway, so keep it simple. */
	UINT32		k;
	UINT32		n;
} fec_oti_t;


