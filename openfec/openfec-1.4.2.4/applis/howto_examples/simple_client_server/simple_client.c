/* $Id: simple_client.c 216 2014-12-13 13:21:07Z roca $ */
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

/* this is the decoder */
#define OF_USE_DECODER

#include "simple_client_server.h"


/*
 * Chose which decoding method to use... Both should be equivalent.
 */
#define USE_DECODE_WITH_NEW_SYMBOL


/* Prototypes */

/**
 * Opens and initializes a UDP socket, ready for receptions.
 */
static SOCKET	init_socket (void);

/**
 * This function receives packets on the incoming UDP socket.
 * It allocates a buffer of size *len and updates the pkt/len arguments with what
 * has been actually received. It works in blocking mode the first time it's called
 * (as the client can be launched a few seconds before the server), and after that
 * in non blocking (i.e. polling) mode. If no packet is received even after having
 * waited a certain time (0.2s), it return OF_STATUS_FAILURE to indicate that the
 * sender probably stopped all transmissions.
 */
static of_status_t	get_next_pkt (SOCKET	so,
				      void	**pkt,
				      INT32	*len);

/**
 * Dumps len32 32-bit words of a buffer (typically a symbol).
 */
static void	dump_buffer_32 (void	*buf,
				UINT32	len32);


/*************************************************************************************************/


int
main (int argc, char* argv[])
{
	of_codec_id_t	codec_id;				/* identifier of the codec to use */
	of_session_t	*ses 		= NULL;			/* openfec codec instance identifier */
	of_parameters_t	*params		= NULL;			/* structure used to initialize the openfec session */
	void**		recvd_symbols_tab= NULL;		/* table containing pointers to received symbols (no FPI here).
								 * The allocated buffer start 4 bytes (i.e., sizeof(FPI)) before... */
	void**		src_symbols_tab	= NULL;			/* table containing pointers to the source symbol buffers (no FPI here) */
	UINT32		symb_sz_32	= SYMBOL_SIZE / 4;	/* symbol size in units of 32 bit words */
	UINT32		k;					/* number of source symbols in the block */
	UINT32		n;					/* number of encoding symbols (i.e. source + repair) in the block */
	UINT32		esi;					/* Encoding Symbol ID, used to identify each encoding symbol */
	SOCKET		so		= INVALID_SOCKET;	/* UDP socket for server => client communications */
	void		*pkt_with_fpi	= NULL;			/* pointer to a buffer containing the FPI followed by the fixed size packet */
	fec_oti_t	*fec_oti	= NULL;			/* FEC Object Transmission Information as received from the server */
	INT32		len;					/* len of the received packet */
	SOCKADDR_IN	dst_host;
	UINT32		n_received	= 0;			/* number of symbols (source or repair) received so far */
	bool		done		= false;		/* true as soon as all source symbols have been received or recovered */
	UINT32		ret;


	/* First of all, initialize the UDP socket and wait for the FEC OTI to be received. This is absolutely required to
	 * synchronize encoder and decoder. We assume this first packet is NEVER lost otherwise decoding is not possible.
	 * In practice the sender can transmit it periodically, or it is sent through a separate reliable channel. */
	if ((so = init_socket()) == INVALID_SOCKET)
	{
		OF_PRINT_ERROR(("Error initializing socket!\n"))
		ret = -1;
		goto end;
	}
	len = sizeof(fec_oti_t);		/* size of the expected packet */
	if ((ret = get_next_pkt(so, (void**)&fec_oti, &len)) != OF_STATUS_OK)
	{
		OF_PRINT_ERROR(("get_next_pkt failed (FEC OTI reception)\n"))
		ret = -1;
		goto end;
	}
	if (len != sizeof(fec_oti_t))
	{
		OF_PRINT_ERROR(("FEC OTI reception failed: bad size, expected %lu but received %d instead\n", sizeof(fec_oti_t), ret))
		ret = -1;
		goto end;
	}
	/* convert back to host endianess */
	codec_id = fec_oti->codec_id	= ntohl(fec_oti->codec_id);
	k = fec_oti->k			= ntohl(fec_oti->k);
	n = fec_oti->n			= ntohl(fec_oti->n);

	printf("\nReceiving packets from %s/%d\n", DEST_IP, DEST_PORT);

	/* and check the correctness of data received */
	if (k > n || k > 40000 || n > 40000)
	{
		OF_PRINT_ERROR(("Invalid FEC OTI received: k=%u or n=%u received are probably out of range\n", k, n))
		ret = -1;
		goto end;
	}
	/* now we know which codec the sender has used along with the codec parameters, we can prepar the params structure accordingly */
	switch (codec_id) {
	case OF_CODEC_REED_SOLOMON_GF_2_M_STABLE: {
		/* fill in the code specific part of the of_..._parameters_t structure */
		of_rs_2_m_parameters_t	*my_params;

		printf("\nInitialize a Reed-Solomon over GF(2^m) codec instance, (n, k)=(%u, %u)...\n", n, k);
		if ((my_params = (of_rs_2_m_parameters_t *)calloc(1, sizeof(* my_params))) == NULL)
		{
			OF_PRINT_ERROR(("no memory for codec %d\n", codec_id))
			ret = -1;
			goto end;
		}
		my_params->m = 8;
		params = (of_parameters_t *) my_params;
		break;
		}

	case OF_CODEC_LDPC_STAIRCASE_STABLE: {
		/* fill in the code specific part of the of_..._parameters_t structure */
		of_ldpc_parameters_t	*my_params;

		printf("\nInitialize an LDPC-Staircase codec instance, (n, k)=(%u, %u)...\n", n, k);
		if ((my_params = (of_ldpc_parameters_t *)calloc(1, sizeof(* my_params))) == NULL)
		{
			OF_PRINT_ERROR(("no memory for codec %d\n", codec_id))
			ret = -1;
			goto end;
		}
		my_params->prng_seed	= rand();
		my_params->N1		= 7;
		params = (of_parameters_t *) my_params;
		break;
		}

	default:
		OF_PRINT_ERROR(("Invalid FEC OTI received: codec_id=%u received is not valid\n", codec_id))
		ret = -1;
		goto end;
	}
	params->nb_source_symbols	= k;		/* fill in the generic part of the of_parameters_t structure */
	params->nb_repair_symbols	= n - k;
	params->encoding_symbol_length	= SYMBOL_SIZE;

	/* Open and initialize the openfec decoding session now that we know the various parameters used by the sender/encoder... */
	if ((ret = of_create_codec_instance(&ses, codec_id, OF_DECODER, VERBOSITY)) != OF_STATUS_OK)
	{
		OF_PRINT_ERROR(("of_create_codec_instance() failed\n"))
		ret = -1;
		goto end;
	}
	if (of_set_fec_parameters(ses, params) != OF_STATUS_OK)
	{
		OF_PRINT_ERROR(("of_set_fec_parameters() failed for codec_id %d\n", codec_id))
		ret = -1;
		goto end;
	}

	printf( "\nDecoding in progress. Waiting for new packets...\n" );

	/* allocate a table for the received encoding symbol buffers. We'll update it progressively */
	if (((recvd_symbols_tab = (void**) calloc(n, sizeof(void*))) == NULL) ||
	    ((src_symbols_tab = (void**) calloc(n, sizeof(void*))) == NULL))
	{
		OF_PRINT_ERROR(("no memory (calloc failed for enc_symbols_tab, n=%u)\n", n))
		ret = -1;
		goto end;
	}

	len = SYMBOL_SIZE + 4;	/* size of the expected packet */
#ifdef USE_DECODE_WITH_NEW_SYMBOL
	/*
	 * this is the standard method: submit each fresh symbol to the library ASAP, upon reception
	 * (or later, but using the standard of_decode_with_new_symbol() function).
	 */
	while ((ret = get_next_pkt(so, &pkt_with_fpi, &len)) == OF_STATUS_OK)
	{
		/* OK, new packet received... */
		n_received++;
		esi = ntohl(*(UINT32*)pkt_with_fpi);
		if (esi > n)		/* a sanity check, in case... */
		{
			OF_PRINT_ERROR(("invalid esi=%u received in a packet's FPI\n", esi))
			ret = -1;
			goto end;
		}
		recvd_symbols_tab[esi] = (char*)pkt_with_fpi + 4;	/* remember */
		printf("%05d => receiving symbol esi=%u (%s)\n", n_received, esi, (esi < k) ? "src" : "repair");
		if (of_decode_with_new_symbol(ses, (char*)pkt_with_fpi + 4, esi) == OF_STATUS_ERROR) {
			OF_PRINT_ERROR(("of_decode_with_new_symbol() failed\n"))
			ret = -1;
			goto end;
		}
		/* check if completed in case we received k packets or more */
		if ((n_received >= k) && (of_is_decoding_complete(ses) == true)) {
			/* done, we recovered everything, no need to continue reception */
			done = true;
			break;
		}
		len = SYMBOL_SIZE + 4;	/* make sure len contains the size of the expected packet */
	}
#else
	/*
	 * this is the alternative method: wait to receive all the symbols, then submit them all to
	 * the library using the of_set_available_symbols() function. In that case decoding will occur
	 * during the of_finish_decoding() call.
	 */
	while ((ret = get_next_pkt(so, &pkt_with_fpi, &len)) == OF_STATUS_OK)
	{
		/* OK, new packet received... */
		n_received++;
		esi = ntohl(*(UINT32*)pkt_with_fpi);
		if (esi > n)		/* a sanity check, in case... */
		{
			OF_PRINT_ERROR(("invalid esi=%u received in a packet's FPI\n", esi))
			ret = -1;
			goto end;
		}
		recvd_symbols_tab[esi] = (char*)pkt_with_fpi + 4;	/* remember */
		printf("%05d => receiving symbol esi=%u (%s)\n", n_received, esi, (esi < k) ? "src" : "repair");
		len = SYMBOL_SIZE + 4;	/* make sure len contains the size of the expected packet */
	}
	/* now we received everything, submit them all to the codec if we received a sufficiently high number of symbols (i.e. >= k) */
	if (n_received >= k && (of_set_available_symbols(ses, recvd_symbols_tab) != OF_STATUS_OK))
	{
		OF_PRINT_ERROR(("of_set_available_symbols() failed with error (%d)\n", ret))
		ret = -1;
		goto end;
	}
#endif
	if (!done && (ret == OF_STATUS_FAILURE) && (n_received >= k))
	{
		/* there's no packet any more but we received at least k, and the use of of_decode_with_new_symbol() didn't succedd to decode,
		 * so try with of_finish_decoding.
		 * NB: this is useless with MDS codes (e.g. Reed-Solomon), but it is essential with LDPC-Staircase as of_decode_with_new_symbol
		 * performs ITerative decoding, whereas of_finish_decoding performs ML decoding */
		ret = of_finish_decoding(ses);
		if (ret == OF_STATUS_ERROR || ret == OF_STATUS_FATAL_ERROR)
		{
			OF_PRINT_ERROR(("of_finish_decoding() failed with error (%d)\n", ret))
			ret = -1;
			goto end;
		}
		else if (ret == OF_STATUS_OK)
		{
			done = true;
		}
		/* else ret == OF_STATUS_FAILURE, meaning of_finish_decoding didn't manage to recover all source symbols */
	}
	if (done)
	{
		/* finally, get a copy of the pointers to all the source symbols, those received (that we already know) and those decoded.
		 * In case of received symbols, the library does not change the pointers (same value). */
		if (of_get_source_symbols_tab(ses, src_symbols_tab) != OF_STATUS_OK)
		{
			OF_PRINT_ERROR(("of_get_source_symbols_tab() failed\n"))
			ret = -1;
			goto end;
		}
		printf("\nDone! All source symbols rebuilt after receiving %u packets\n", n_received);
		if (VERBOSITY > 1)
		{
			for (esi = 0; esi < k; esi++) {
				printf("src[%u]= ", esi);
				dump_buffer_32(src_symbols_tab[esi], 1);
			}
		}
	}
	else
	{
		printf("\nFailed to recover all erased source symbols even after receiving %u packets\n", n_received);
	}


end:
	/* Cleanup everything... */
	if (so!= INVALID_SOCKET)
	{
		close(so);
	}
	if (ses)
	{
		of_release_codec_instance(ses);
	}
	if (params)
	{
		free(params);
	}
	if (fec_oti)
	{
		free(fec_oti);
	}
	if (recvd_symbols_tab && src_symbols_tab)
	{
		for (esi = 0; esi < n; esi++)
		{
			if (recvd_symbols_tab[esi])
			{
				/* this is a symbol received from the network, without its FPI that starts 4 bytes before */
				free((char*)recvd_symbols_tab[esi] - 4);
			}
			else if (esi < k && src_symbols_tab[esi])
			{
				/* this is a source symbol decoded by the openfec codec, so free it */
				ASSERT(recvd_symbols_tab[esi] == NULL);
				free(src_symbols_tab[esi]);
			}
		}
		free(recvd_symbols_tab);
		free(src_symbols_tab);
	}
	return ret;
}


/**
 * Opens and initializes a UDP socket, ready for receptions.
 */
static SOCKET
init_socket ()
{
	SOCKET		s;
	SOCKADDR_IN	bindAddr;
	UINT32		sz = 1024 * 1024;

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Error: call to socket() failed\n");
		return INVALID_SOCKET;
	}
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_port = htons((short)DEST_PORT);
	bindAddr.sin_addr.s_addr = INADDR_ANY;
	if (bind(s, (SOCKADDR*) &bindAddr, sizeof(bindAddr)) == SOCKET_ERROR)
	{
		printf("bind() failed. Port %d may be already in use\n", DEST_PORT);
		return INVALID_SOCKET;
	}
	/* increase the reception socket size as the default value may lead to a high datagram loss rate */
	if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, &sz, sizeof(sz)) == -1) {
		printf("setsockopt() failed to set new UDP socket size to %u\n", sz);
		return INVALID_SOCKET;
	}
	return s;
}


/**
 * Receives packets on the incoming UDP socket.
 */
static of_status_t
get_next_pkt   (SOCKET		so,
		void		**pkt,
		INT32		*len)
{
	static bool	first_call = true;
	INT32		saved_len = *len;	/* save it, in case we need to do several calls to recvfrom */

	if ((*pkt = malloc(saved_len)) == NULL)
	{
		OF_PRINT_ERROR(("no memory (malloc failed for p)\n"))
		return OF_STATUS_ERROR;
	}
	if (first_call)
	{
		/* the first time we must be in blocking mode since the flow may be launched after a few seconds... */
		first_call = false;
		*len = recvfrom(so, *pkt, saved_len, 0, NULL, NULL);
		if (*len < 0)
		{
			/* this is an anormal error, exit */
			perror("recvfrom");
			OF_PRINT_ERROR(("recvfrom failed\n"))
			free(*pkt);	/* don't forget to free it, otherwise it will leak */
			return OF_STATUS_ERROR;
		}
		/* set the non blocking mode for this socket now that the flow has been launched */
		if (fcntl(so, F_SETFL, O_NONBLOCK) < 0)
		{
			OF_PRINT_ERROR(("ERROR, fcntl failed to set non blocking mode\n"))
			exit(-1);
		}
		if (VERBOSITY > 1)
			printf("%s: pkt received 0, len=%u\n", __FUNCTION__, *len);
		return OF_STATUS_OK;
	}
	/* otherwise we are in non-blocking mode... */
	*len = recvfrom(so, *pkt, saved_len, 0, NULL, NULL);
	if (*len > 0)
	{
		if (VERBOSITY > 1)
			printf("%s: pkt received 1, len=%u\n", __FUNCTION__, *len);
		return OF_STATUS_OK;
	}
	else if (errno == EAGAIN || errno == EWOULDBLOCK)
	{
		/* no packet available, sleep a little bit and retry */
		SLEEP(200);	/* (in milliseconds) */
		*len = recvfrom(so, *pkt, saved_len, 0, NULL, NULL);
		if (*len > 0)
		{
			if (VERBOSITY > 1)
				printf("%s: pkt received 2, len=%u\n", __FUNCTION__, *len);
			return OF_STATUS_OK;
		}
		else
		{
			/* that's the end of the test, no packet available any more, we're sure of that now... */
			if (VERBOSITY > 1)
				printf("%s: end of test, no packet after the sleep\n", __FUNCTION__);
			free(*pkt);	/* don't forget to free it, otherwise it will leak */
			return OF_STATUS_FAILURE;
		}
	}
	else
	{
		/* this is an anormal error, exit */
		perror("recvfrom");
		OF_PRINT_ERROR(("ERROR, recvfrom failed\n"))
		free(*pkt);	/* don't forget to free it, otherwise it will leak */
		return OF_STATUS_ERROR;
	}
	return OF_STATUS_ERROR;	/* never called */
}


/**
 * Dumps len32 32-bit words of a buffer (typically a symbol).
 */
static void
dump_buffer_32 (void	*buf,
		UINT32	len32)
{
	UINT32	*ptr;
	UINT32	j = 0;

	printf("0x");
	for (ptr = (UINT32*)buf; len32 > 0; len32--, ptr++) {
		/* convert to big endian format to be sure of byte order */
		printf( "%08X", htonl(*ptr));
		if (++j == 10)
		{
			j = 0;
			printf("\n");
		}
	}
	printf("\n");
}

