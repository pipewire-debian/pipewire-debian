/* $Id: README.TXT 202 2014-11-05 09:42:33Z roca $ */
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


Principles:
-----------

This directory contains a simple client-server application, showing how to use
the OpenFEC Reed-Solomon GF(2^m) and LDPC-Staircase codecs in a simple case,
with a single object, using a UDP socket and rudimentary signaling. Many
parameters are set statically (e.g. the code rate) in order to simplify the
example as much as possible. The object size (in number of symbols) is the only
input argument.

    +--------------+                     +--------------+
    |    SERVER    |                     |    CLIENT    |
    +--------------+                     +--------------+
    |{set of source|                     |{set of source|
    |   symbols}   |                     |   symbols}   |
    |      |       |                     |      ^       |
    |      v       |                     |      |       |
    | FEC encoding |                     | FEC decoding |
    |      |       |                     |      ^       |
    |      v       |                     |      |       |
    | UDP sendto   |                     | UDP recvfrom |
    |      |       |                     |      |       |
    +------|-------+                     +------|-------+
           |           UDP connection           |       
           +------------------------------------+
              signaling traffic + data traffic

Signaling is rudimentary in this toy example:
	- FEC-Object Transmission Information (FEC-OTI) that enables encoder/
	  decoder synchronisation is not [RFC5170] and [RFC5510] compliant;
	- FEC Payload ID (FPI) that carries information to identify each symbol
	  sent in a UDP packet is not [RFC5170] and [RFC5510] compliant;
We also assume that the object is small enough to enable encoding in a single
block. More precisely, if the number of symbols in the object is small enough
for Reed-Solomon to be used, use it, otherwise use LDPC-Staircase if the number
of symbols remains below the maximum block size. In practice, a block
partitionning algorithm should be used to bypass this limit, partitionning large
objects into as many blocks as needed (see section 9.1 of [RFC5052]).


About packet erasures:
----------------------

We simulate packet erasures on the UDP connection. See simple_client_server.h
header file to control the loss rate (BTW, make sure it is compatible with the
code rate). By default:
#define CODE_RATE	0.667		/* k/n = 2/3 means we add 50% of repair symbols */
#define LOSS_RATE	0.30		/* we consider 30% of packet losses... It assumes
					   there's no additional loss during UDP transmissions */

However, since this is a UDP connection, since the server transmits at full
rate, the client may lose additional packets at the incoming UDP socket, even if
we already increased this socket size. With a real server, transmissions would
be rate-controlled with a leaky bucket, or a token bucket, or any similar system.

So if you experience such errors as:
	Failed to recover all erased source symbols even after receiving XXX packets
then you can try to add a usleep(500) (for instance) in file simple_server.c in the
transmission loop in order to slow down transmissions. It should help! And as
explained above, the true solution is to implement rate control.


Usage: 
------

On the same machine (we are sending at 127.0.0.1 by default), use two terminals. Start the receiver first:
$ ./simple_client

Then the server, using default parameters:
$ ./simple_server
or, if you want to specify the object size (in symbols):
$ ./simple_server 1000

The codec being used will be selected based on the object size (small => RS, medium or high => LDPC-Staircase).


References:
-----------

[RFC5052] "Forward Error Correction (FEC) Building Block" (Proposed Standard)
[RFC5170] "Low Density Parity Check (LDPC) Staircase and Triangle Forward Error Correction (FEC) Schemes" (Proposed Standard) 
[RFC5510] "Reed-Solomon Forward Error Correction (FEC) Schemes" (Proposed Standard) 

