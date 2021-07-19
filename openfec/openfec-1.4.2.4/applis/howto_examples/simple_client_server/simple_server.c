/* $Id: simple_server.c 216 2014-12-13 13:21:07Z roca $ */
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

/* this is the encoder */
#define OF_USE_ENCODER

#include "simple_client_server.h"


/* Prototypes */

/**
 * Opens and initializes a UDP socket, ready for receptions.
 */
static SOCKET	init_socket (SOCKADDR_IN	*dst_host);

/**
 * Shuffles the array randomly.
 */
static void	randomize_array (UINT32		**array,
				 UINT32		arrayLen);

/**
 * Dumps len32 32-bit words of a buffer (typically a symbol).
 */
static void	dump_buffer_32 (void	*buf,
				UINT32	len32);


/*************************************************************************************************/


int
main(int argc, char* argv[])
{
	of_codec_id_t	codec_id;				/* identifier of the codec to use */
	of_session_t	*ses 		= NULL;			/* openfec codec instance identifier */
	of_parameters_t	*params		= NULL;			/* structure used to initialize the openfec session */
	void**		enc_symbols_tab	= NULL;			/* table containing pointers to the encoding (i.e. source + repair) symbols buffers */
	UINT32		symb_sz_32	= SYMBOL_SIZE / 4;	/* symbol size in units of 32 bit words */
	UINT32		k;					/* number of source symbols in the block */
	UINT32		n;					/* number of encoding symbols (i.e. source + repair) in the block */
	UINT32		esi;					/* Encoding Symbol ID, used to identify each encoding symbol */
	UINT32		i;
	UINT32*		rand_order	= NULL;			/* table used to determine a random transmission order. This randomization process
								 * is essential for LDPC-Staircase optimal performance */
	SOCKET		so		= INVALID_SOCKET;	/* UDP socket for server => client communications */
	char		*pkt_with_fpi	= NULL;			/* buffer containing a fixed size packet plus a header consisting only of the FPI */
	fec_oti_t	fec_oti;				/* FEC Object Transmission Information as sent to the client */
	INT32		lost_after_index= -1;			/* all the packets to send after this index are considered as lost during transmission */
	SOCKADDR_IN	dst_host;
	UINT32		ret		= -1;

	
	if (argc == 1)
	{
		/* k value is ommited, so use default */
		k = DEFAULT_K;
	}
	else
	{
		k = atoi(argv[1]);
	}
	n = (UINT32)floor((double)k / (double)CODE_RATE);
	/* Choose which codec is the most appropriate. If small enough, choose Reed-Solomon (with m=8), otherwise LDPC-Staircase.
	 * Then finish the openfec session initialization accordingly */
	if (n <= 255)
	{
		/* fill in the code specific part of the of_..._parameters_t structure */
		of_rs_2_m_parameters_t	*my_params;

		printf("\nInitialize a Reed-Solomon over GF(2^m) codec instance, (n, k)=(%u, %u)...\n", n, k);
		codec_id = OF_CODEC_REED_SOLOMON_GF_2_M_STABLE;
		if ((my_params = (of_rs_2_m_parameters_t *)calloc(1, sizeof(* my_params))) == NULL)
		{
			OF_PRINT_ERROR(("no memory for codec %d\n", codec_id))
			ret = -1;
			goto end;
		}
		my_params->m = 8;
		params = (of_parameters_t *) my_params;
	}
	else
	{
		/* fill in the code specific part of the of_..._parameters_t structure */
		of_ldpc_parameters_t	*my_params;

		printf("\nInitialize an LDPC-Staircase codec instance, (n, k)=(%u, %u)...\n", n, k);
		codec_id = OF_CODEC_LDPC_STAIRCASE_STABLE;
		if ((my_params = (of_ldpc_parameters_t *)calloc(1, sizeof(* my_params))) == NULL)
		{
			OF_PRINT_ERROR(("no memory for codec %d\n", codec_id))
			ret = -1;
			goto end;
		}
		my_params->prng_seed	= rand();
		my_params->N1		= 7;
		params = (of_parameters_t *) my_params;
	}
	params->nb_source_symbols	= k;		/* fill in the generic part of the of_parameters_t structure */
	params->nb_repair_symbols	= n - k;
	params->encoding_symbol_length	= SYMBOL_SIZE;

	/* Open and initialize the openfec session now... */
	if ((ret = of_create_codec_instance(&ses, codec_id, OF_ENCODER, VERBOSITY)) != OF_STATUS_OK)
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

	/* Allocate and initialize our source symbols...
	 * In case of a file transmission, the opposite takes place: the file is read and partitionned into a set of k source symbols.
	 * At the end, it's just equivalent since there is a set of k source symbols that need to be sent reliably thanks to an FEC
	 * encoding. */
	printf("\nFilling source symbols...\n");
	if ((enc_symbols_tab = (void**) calloc(n, sizeof(void*))) == NULL) {
		OF_PRINT_ERROR(("no memory (calloc failed for enc_symbols_tab, n=%u)\n", n))
		ret = -1;
		goto end;
	}
	/* In order to detect corruption, the first symbol is filled with 0x1111..., the second with 0x2222..., etc.
	 * NB: the 0x0 value is avoided since it is a neutral element in the target finite fields, i.e. it prevents the detection
	 * of symbol corruption */
	for (esi = 0; esi < k; esi++ )
	{
		if ((enc_symbols_tab[esi] = calloc(symb_sz_32, sizeof(UINT32))) == NULL)
		{
			OF_PRINT_ERROR(("no memory (calloc failed for enc_symbols_tab[%d])\n", esi))
			ret = -1;
			goto end;
		}
		memset(enc_symbols_tab[esi], (char)(esi + 1), SYMBOL_SIZE);
		if (VERBOSITY > 1)
		{
			printf("src[%03d]= ", esi);
			dump_buffer_32(enc_symbols_tab[esi], 1);
		}
	}

	/* Now build the n-k repair symbols... */
	printf("\nBuilding repair symbols...\n");
	for (esi = k; esi < n; esi++)
	{
		if ((enc_symbols_tab[esi] = (char*)calloc(symb_sz_32, sizeof(UINT32))) == NULL)
		{
			OF_PRINT_ERROR(("no memory (calloc failed for enc_symbols_tab[%d])\n", esi))
			ret = -1;
			goto end;
		}
		if (of_build_repair_symbol(ses, enc_symbols_tab, esi) != OF_STATUS_OK) {
			OF_PRINT_ERROR(("ERROR: of_build_repair_symbol() failed for esi=%u\n", esi))
			ret = -1;
			goto end;
		}
		if (VERBOSITY > 1)
		{
			printf("repair[%03d]= ", esi);
			dump_buffer_32(enc_symbols_tab[esi], 4);
		}
	}

	/* Randomize the packet order, it's important for LDPC-Staircase codes for instance... */
	printf("\nRandomizing transmit order...\n");
	if ((rand_order = (UINT32*)calloc(n, sizeof(UINT32))) == NULL)
	{
		OF_PRINT_ERROR(("no memory (calloc failed for rand_order)\n"))
		ret = -1;
		goto end;
	}
	randomize_array(&rand_order, n);

	/* Finally initialize the UDP socket and throw our packets... */
	if ((so = init_socket(&dst_host)) == INVALID_SOCKET)
	{
		OF_PRINT_ERROR(("Error initializing socket!\n"))
		ret = -1;
		goto end;
	}
	printf("First of all, send the FEC OTI for this object to %s/%d\n", DEST_IP, DEST_PORT);
	/* Initialize and send the FEC OTI to the client */
	/* convert back to host endianess */
	fec_oti.codec_id	= htonl(codec_id);
	fec_oti.k		= htonl(k);
	fec_oti.n		= htonl(n);
	if ((ret = sendto(so, (void*)&fec_oti, sizeof(fec_oti), 0, (SOCKADDR *)&dst_host, sizeof(dst_host))) != sizeof(fec_oti)) {
		OF_PRINT_ERROR(("Error while sending the FEC OTI\n"))
		ret = -1;
		goto end;
	}

	lost_after_index = n * (1 - LOSS_RATE);
	if (lost_after_index < k)
	{
		OF_PRINT_ERROR(("The loss rate %f is to high: only %u packets will be sent, whereas k=%u\n", LOSS_RATE, lost_after_index, k))
		ret = -1;
		goto end;
	}
	printf("Sending %u source and repair packets to %s/%d. All packets sent at index %u and higher are considered as lost\n",
		n, DEST_IP, DEST_PORT, lost_after_index);
	/* Allocate a buffer where we'll copy each symbol plus its simplistif FPI (in this example consisting only of the ESI).
	 * This needs to be fixed in real applications, with the actual FPI required for this code. Also doing a memcpy is
	 * rather suboptimal in terms of performance! */
	if ((pkt_with_fpi = malloc(4 + SYMBOL_SIZE)) == NULL)
	{
		OF_PRINT_ERROR(("no memory (malloc failed for pkt_with_fpi)\n"))
		ret = -1;
		goto end;
	}
	for (i = 0; i < n; i++)
	{
		if (i == lost_after_index)
		{
			/* the remaining packets are considered as lost, exit loop */
			break;
		}
		/* Add a pkt header wich only countains the ESI, i.e. a 32bits sequence number, in network byte order in order
		 * to be portable regardless of the local and remote byte endian representation (the receiver will do the
		 * opposite with ntohl()...) */
		*(UINT32*)pkt_with_fpi = htonl(rand_order[i]);
		memcpy(4 + pkt_with_fpi, enc_symbols_tab[rand_order[i]], SYMBOL_SIZE);
		printf("%05d => sending symbol %u (%s)\n", i + 1, rand_order[i], (rand_order[i] < k) ? "src" : "repair");
		if ((ret = sendto(so, pkt_with_fpi, SYMBOL_SIZE + 4, 0, (SOCKADDR *)&dst_host, sizeof(dst_host))) == SOCKET_ERROR)
		{
			OF_PRINT_ERROR(("sendto() failed!\n"))
			ret = -1;
			goto end;
		}
		/* Perform a short usleep() to slow down transmissions and avoid UDP socket saturation at the receiver.
		 * Note that the true solution consists in adding some rate control mechanism here, like a leaky or token bucket. */
		usleep(500);
	}
	printf( "\nCompleted! %d packets sent successfully.\n", i);
	ret = 1;

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
	if (rand_order) {
		free(rand_order);
	}
	if (enc_symbols_tab)
	{
		for (esi = 0; esi < n; esi++)
		{
			if (enc_symbols_tab[esi])
			{
				free(enc_symbols_tab[esi]);
			}
		}
		free(enc_symbols_tab);
	}
	if (pkt_with_fpi)
	{
		free(pkt_with_fpi);
	}
	return ret;
}


/* Randomize an array of integers */
void
randomize_array (UINT32		**array,
		 UINT32		arrayLen)
{
	UINT32		backup	= 0;
	UINT32		randInd	= 0;
	UINT32		seed;		/* random seed for the srand() function */
	UINT32		i;

	struct timeval	tv;
	if (gettimeofday(&tv, NULL) < 0) {
		OF_PRINT_ERROR(("gettimeofday() failed"))
		exit(-1);
	}
	seed = (int)tv.tv_usec;
	srand(seed);
	for (i = 0; i < arrayLen; i++)
	{
		(*array)[i] = i;
	}
	for (i = 0; i < arrayLen; i++)
	{
		backup = (*array)[i];
		randInd = rand()%arrayLen;
		(*array)[i] = (*array)[randInd];
		(*array)[randInd] = backup;
	}
}


/* Initialize our UDP socket */
static SOCKET
init_socket (SOCKADDR_IN	*dst_host)
{
	SOCKET s;

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Error: call to socket() failed\n");
		return INVALID_SOCKET;
	}
	dst_host->sin_family = AF_INET;
	dst_host->sin_port = htons((short)DEST_PORT);
	dst_host->sin_addr.s_addr = inet_addr(DEST_IP);
	return s;
}


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

