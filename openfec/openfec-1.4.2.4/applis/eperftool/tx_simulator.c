/* $Id: tx_simulator.c 199 2014-10-21 14:25:02Z roca $ */
/*
 * OpenFEC.org AL-FEC Library.
 * (c) Copyright 2009-2011 INRIA - All rights reserved
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


#include "eperftool.h"


/*
 * Local functions
 */
static void	randomize_array (UINT32 *array, UINT32 array_len);

static UINT32	define_symbol_tx_order (UINT32	*array);

static bool*	create_loss_array_from_erasure_proba	(UINT32 size, double desired_loss_proba);

static bool*	create_loss_array_from_erasure_nb	(UINT32 size, UINT32 desired_loss_nb);

static bool*	create_loss_array_from_markov_model	(UINT32 size);

/** This table gives the order in which transmissions take place. */
static UINT32*	tx_order_tab = NULL;

/** This table indicates which of the symbols are lost. */
static bool*	is_lost_tab = NULL;

/** control the number of received symbols in find_min_overhead_mode. We need a counter for that. */
static UINT32	tot_nb_recvd_symbols;

/* This is the index for next symbol sent and perhaps received, unless it has been lost */
static UINT32	next_idx = 0;

/** PRNG state. */
static unsigned long next = 1;

void mysrand (UINT32 seed)
{
	next = seed;
}


/* RAND_MAX assumed to be 32767 */
UINT32 myrand ()
{
	next = next * 1103515245 + 12345;
	return(next & 0x7fffffff);
}


of_status_t
init_tx_simulator()
{
	UINT32		i;

	/* first of all, fix the seed */
	init_prng_with_seed(suggested_seed);
	/*
	 * Randomize the symbol order... This order is used for the "transmission" of symbols.
	 * It provides a list of indexes that can be used later to access the right
	 */
	if ((tx_order_tab = (UINT32*)calloc(tot_nb_encoding_symbols, sizeof(UINT32))) == NULL) {
		OF_PRINT_ERROR(("no mem\n"))
		goto error;
	}
	if ((max_decoding_steps = define_symbol_tx_order(tx_order_tab)) == 0) {
		OF_PRINT_ERROR(("ERROR, define_symbol_tx_order() failed\n"))
		goto error;
	}
	/*
	 * Create a loss array, that defines which of the symbols in the tx_order_tab are lost.
	 * This can be made either from the target erasure probability, or by giving an
	 * actual number of erasures. The loss model must also be considered during this
	 * process.
	 */
	switch (loss_model) {
	case 0:
		/* no loss */
		is_lost_tab = create_loss_array_from_erasure_nb(max_decoding_steps, 0);
		break;
	case 1:
		is_lost_tab = create_loss_array_from_markov_model(max_decoding_steps);
		break;
	case 2:
		is_lost_tab = create_loss_array_from_erasure_proba(max_decoding_steps, p_loss);
		break;
	case 3:
		if (tx_mode == TX_MODE_SIMUL_BCAST_WITH_UNICAST_REPAIR) {
			/* do things in two steps: loss array for the first repair symbols, then
			 * extend it to simulate the reception of all source symbols. */
			if (nb_loss > tot_nb_repair_symbols) {
				OF_PRINT_ERROR(("number of desired losses (%d) cannot exceed number of repair symbols (%d)\n",
						nb_loss, tot_nb_repair_symbols))
				goto error;
			}
			is_lost_tab = create_loss_array_from_erasure_nb(tot_nb_repair_symbols, nb_loss);
			for (i = 0; i < src_pkt_nb; i++) {	/* mark the following (source) symbols as received */
				is_lost_tab[tot_nb_repair_symbols + i] = false;
			}
		} else {
			if (nb_loss > max_decoding_steps) {
				OF_PRINT_ERROR(("number of desired losses (%d) cannot exceed total number of symbols that could be received (%d)\n",
						nb_loss, max_decoding_steps))
				goto error;
			}
			is_lost_tab = create_loss_array_from_erasure_nb(max_decoding_steps, nb_loss);
		}
		break;
	case 4:
		is_lost_tab = create_loss_array_from_erasure_nb(max_decoding_steps, 0);
		break;
	case 5:
		/* loss only occur after the first trim_after_this_nb_rx_pkts received packets */
		if (trim_after_this_nb_rx_pkts > max_decoding_steps) {
			OF_PRINT_ERROR(("number of symbols to receive (%d) cannot exceed total number of symbols that could be received (%d)\n",
					trim_after_this_nb_rx_pkts, max_decoding_steps))
			goto error;
		}
		is_lost_tab = create_loss_array_from_erasure_nb(max_decoding_steps, 0);
		for (i = trim_after_this_nb_rx_pkts; i < max_decoding_steps; i++) {
			is_lost_tab[i] = true;
		}
		break;
	default:
		OF_PRINT_ERROR(("loss model (%d) is not supported\n", loss_model))
		goto error;
	}
	/* reset the number of received symbol counter and next_idx counter (required to
	 * supportfind_min_overhead_mode) */
	tot_nb_recvd_symbols = 0;
	next_idx = 0;
	OF_TRACE_LVL(1, ("init_tx_simulator: %d symbols can be received\n", max_decoding_steps))
	return OF_STATUS_OK;

error:
	return OF_STATUS_ERROR;
}


of_status_t
close_tx_simulator ()
{
	if (ldpc_matrix_file_name)
	{
		free(ldpc_matrix_file_name);
		ldpc_matrix_file_name=NULL;
	}
	if (is_lost_tab) {
		free(is_lost_tab);
		is_lost_tab = NULL;
	}
	if (tx_order_tab) {
		free(tx_order_tab);
		tx_order_tab = NULL;
	}
	if (ldpc_matrix_file_name)
	{
		free(ldpc_matrix_file_name);
		ldpc_matrix_file_name=NULL;
	}
	return OF_STATUS_OK;
}


/**
 * Return the next symbol received or NULL if none is available.
 * It goes through the list of (permuted in a certain manner) symbols sent and applies
 * the desired loss model on top of it.
 */
symbol_cb_t *
get_next_symbol_received ()
{
	symbol_cb_t		*symb;

	while (next_idx < max_decoding_steps) {
		if (is_lost_tab[next_idx] == true) {
			/* this symbol is lost, switch to the next one */
			next_idx++;
			continue;
		} else {
			/* this symbol is received, return it */
			if ((find_min_overhead_mode == true) &&
			    (tot_nb_recvd_symbols >= trim_after_this_nb_rx_pkts)) {
				/* stop test in find_min_overhead mode if we achieved the maximum value */
				break;
			}
			tot_nb_recvd_symbols++;
			if (loss_model == 4)
			{
				symb = &(symb_cb_tab[tx_order_tab[myrand() % max_decoding_steps]]);
			}
			else
			{
				symb = &(symb_cb_tab[tx_order_tab[next_idx]]);
			}
			OF_TRACE_LVL(1, ("%s: next_idx: %u, tx_order_tab: %u, symb esi: %u, sbn: %u\n",
				__FUNCTION__, next_idx, tx_order_tab[next_idx], symb->esi, symb->sbn))
			next_idx++;
			return symb;
		}
	}
	OF_TRACE_LVL(1, ("%s: no symbol available any more after receiving %d symbols...\n",
			__FUNCTION__, tot_nb_recvd_symbols))
	return NULL;
}


/**
 * Randomize an array of integers
 */
void
randomize_array (UINT32		*array,
		 UINT32		array_len)
{
	UINT32	backup = 0;
	UINT32	rand_idx = 0;
	UINT32	i;

	for (i = 0; i < array_len; i++ ) {
		array[i] = i;
	}
	for (i = 0; i < array_len; i++) {
		backup = array[i];
		rand_idx = myrand() % array_len;
		array[i] = array[rand_idx];
		array[rand_idx] = backup;
	}
}


UINT32
define_symbol_tx_order (UINT32	*array)
{
	UINT32	max_steps;	// total number of packets received
	UINT32	i;

	switch (tx_mode) {
	case TX_MODE_ALL_RANDOM:		// random permutation of all symbols
		OF_TRACE_LVL(1, ("Randomly send all source + repair symbols...\n"))
		max_steps = tot_nb_encoding_symbols;
		randomize_array(array, max_steps);
		break;

	case TX_MODE_FEW_SRC_SYMBOLS: {	// all repair pkts + a few source packets
		UINT32	*rand_src_idx;	// tab of randomly sorted src pkt idx
		UINT32	nb_src_considered; // nb of source packets sent
		UINT32	offset;		// offset to add to all indexes
		UINT32	idx;		// packet index

		// src_pkt_ratio is the % of source data packets that are sent
		if (use_src_pkt_ratio) {
			nb_src_considered = (UINT32)((double)src_pkt_ratio / 100 * (double)tot_nb_source_symbols);
		} else {
			nb_src_considered = src_pkt_nb;
		}
		max_steps = tot_nb_repair_symbols + nb_src_considered;
		if ((rand_src_idx = (UINT32*)calloc(nb_src_considered, sizeof(int))) == NULL) {
			goto no_mem;
		}
		OF_TRACE_LVL(1, ("Randomly send %d source symbols (not necessarily received) and all the repair symbols in a random order...\n",
			nb_src_considered))
		randomize_array(rand_src_idx, nb_src_considered);
		randomize_array(array, max_steps);
		offset = tot_nb_encoding_symbols - max_steps;
		for (i = 0; i < max_steps; i++ ) {
			idx = array[i];
			if (idx < nb_src_considered) {
				// it's a source packet
				array[i] = rand_src_idx[idx];
			} else {
				// it's a repair packet
				array[i] += offset;
			}
		}
		free(rand_src_idx);
		break;
		}

	case TX_MODE_FEW_SRC_SYMBOLS_FIRST: {
					// all repair pkts + a few source packets
					// sent first to be sure they will be received
		UINT32	*rand_src_idx;	// tab of randomly sorted src pkt idx
		UINT32	nb_src_considered; // nb of source packets sent

		if (use_src_pkt_ratio) {
			nb_src_considered = (UINT32)((double)src_pkt_ratio / 100 * (double)tot_nb_source_symbols);
		} else {
			nb_src_considered = src_pkt_nb;
		}
		max_steps = tot_nb_repair_symbols + nb_src_considered;
		if ((rand_src_idx = (UINT32*)calloc(nb_src_considered, sizeof(UINT32))) == NULL) {
			goto no_mem;
		}
		OF_TRACE_LVL(1, ("Randomly send %d source symbols first (always received), then randomly all repair symbols...\n",
			nb_src_considered))
		randomize_array(rand_src_idx, nb_src_considered);
		randomize_array(array + nb_src_considered, tot_nb_repair_symbols);
		for (i = 0; i < max_steps; i++) {
			if (i < nb_src_considered) {
				// it's a source packet
				array[i] = rand_src_idx[i];
			} else {
				// it's a repair packet
				array[i] += tot_nb_source_symbols;
			}
		}
		free(rand_src_idx);
		break;
		}

	case TX_MODE_NON_SYSTEMATIC:	// no source packet transmission
		OF_TRACE_LVL(1, ("Randomly send only repair symbols (non systematic)...\n"))
		max_steps = tot_nb_repair_symbols;
		randomize_array(array, max_steps);
		for (i = 0; i < max_steps; i++ ) {
			array[i] += tot_nb_source_symbols;
		}
		break;

	case TX_MODE_SEQUENTIAL:
		OF_TRACE_LVL(1, ("Sequentially send all source symbols first, then repair symbols...\n"))
		max_steps = tot_nb_encoding_symbols;
		for (i = 0; i < max_steps; i++ ) {
			array[i] = i;
		}
		break;

	case TX_MODE_INVERSE_SEQUENTIAL:
		OF_TRACE_LVL(1, ("Sequentially send all repair symbols first, then source symbols...\n"))
		max_steps = tot_nb_encoding_symbols;
		for (i = 0; i < max_steps; i++ ) {
			if (i < tot_nb_repair_symbols) {
				array[i] = i + tot_nb_source_symbols;
			} else {
				// it's a repair packet
				array[i] = i - tot_nb_repair_symbols;
			}
		}
		break;

	case TX_MODE_SEQUENTIAL_SRC_THEN_RANDOM_REPAIR:
		OF_TRACE_LVL(1, ("Sequentially send all source symbols first, then randomly repair symbols...\n"))
		max_steps = tot_nb_encoding_symbols;
		randomize_array(array + tot_nb_source_symbols, tot_nb_repair_symbols);
		for (i = 0; i < max_steps; i++ ) {
			if (i < tot_nb_source_symbols) {
				array[i] = i;
			} else {
				// it's a repair packet
				array[i] += tot_nb_source_symbols;
			}
		}
		break;

	case TX_MODE_SEQUENTIAL_REPAIR_THEN_RANDOM_SRC:
		OF_TRACE_LVL(1, ("Sequentially send all repair symbols first, then randomly source symbols...\n"))
		max_steps = tot_nb_encoding_symbols;
		randomize_array(array + tot_nb_repair_symbols, tot_nb_source_symbols);
		for (i = 0; i < max_steps; i++ ) {
			if (i < tot_nb_repair_symbols) {
				array[i] = i + tot_nb_source_symbols;
			}
		}
		break;

	case TX_MODE_SIMUL_BCAST_WITH_UNICAST_REPAIR: {
					// simulate ISDB-Tmm with unicast repair:
					// send all repair randomly, subject to packet
					// losses, then send the first few source packets
					// sequentially, not subject to any loss (3G connection)
		UINT32	*src_idx;	// tab of src pkt idx
		UINT32	nb_src_considered; // nb of source packets sent
		UINT32	i;

		if (use_src_pkt_ratio) {
			//nb_src_considered = (UINT32)((double)src_pkt_ratio / 100 * (double)tot_nb_source_symbols);
			OF_PRINT_ERROR(("ERROR, use -rx_src_nb=<n> to specify number of erasures in tx_mode TX_MODE_SIMUL_BCAST_WITH_UNICAST_REPAIR\n"))
			goto error;
		} else {
			nb_src_considered = src_pkt_nb;
		}
		max_steps = tot_nb_repair_symbols + nb_src_considered;
		if ((src_idx = (UINT32*)calloc(nb_src_considered, sizeof(UINT32))) == NULL) {
			goto no_mem;
		}
		OF_TRACE_LVL(1, ("simulate broadcast tx + unicast repair (send all repair randomly, subject to packet losses, then send the first %d source packets sequentially, not subject to any loss (3G connection)\n",
			nb_src_considered))
		for (i = 0; i < nb_src_considered; i++ ) {	// in sequence
			src_idx[i] = i;
		}
		randomize_array(array, tot_nb_repair_symbols);
		for (i = 0; i < max_steps; i++) {
			if (i < tot_nb_repair_symbols) {
				// it's a repair packet
				array[i] += tot_nb_source_symbols;
			} else {
				// it's a source packet
				array[i] = src_idx[i - tot_nb_repair_symbols];
			}
		}
		free(src_idx);
		break;
		}

#if 0
	case TX_MODE_PKT_INTERLEAVING:
		UINT32	j;
		OF_TRACE_LVL(1, ("Send source and repair symbol in an interleaved way...\n"))
		max_steps = tot_nb_encoding_symbols;
		for (i = 0; i < (UINT32)floor((double)bs.A_large * fec_ratio); i++ ) {
			UINT32	k = 0;
			UINT32	k_for_this_blk;
			UINT32	idx = i;
			UINT32	repair_idx = tot_nb_source_symbols;
			for (j = 0; j < (UINT32)bs.nb_blocks; j++ ) {
				if (j < (UINT32)bs.I) {
					k_for_this_blk = bs.A_large;
				} else {
					k_for_this_blk = bs.A_small;
				}
				if (i < k_for_this_blk) {
					array[k] = idx; /* source */
					printf("k=%i,array[%i]=%i\n",k,k,array[k]);
					k++;
				} else if (i < (UINT32)floor((double)k_for_this_blk * fec_ratio)) {
					array[k] = repair_idx + (i - k_for_this_blk); /* repair */
					printf("k=%i,array[%i]=%i\n",k,k,array[k]);
					k++;
				}
				idx += k_for_this_blk;
				repair_idx += (UINT32)floor((double)k_for_this_blk * fec_ratio) - k_for_this_blk;
			}
		}
		break;
#endif
	default:
		OF_PRINT_ERROR(("ERROR: transmission mode %d not supported!\n", tx_mode))
		goto error;
	}
	return max_steps;

no_mem:
	OF_PRINT_ERROR(("ERROR: no memory.\n"))
error:
	return 0;
}

/**
 * Simulate packets losses randomly using a simple two state Markov model.
 * @return	Returns 0 if OK, 1 if packet should be lost.
 */
int	random_loss(void)
{
	static UINT32	state = 0;	/* loss state in the Markov model: 0: NO_LOSS,  1: LOSS */
	UINT32		is_lost = 0;

	switch (state) {
	case 0: /* last packet was sent OK. */
		if (((double)myrand() * 100.0 / (double)RAND_MAX) < (double)p_loss_when_ok) {
			is_lost = 1;
			state = 1;
		}
		break;
	case 1: /* last packet was lost */
		if (((double)myrand() * 100.0 / (double)RAND_MAX) < (double)p_success_when_losses) {
			state = 0;
		} else {
			is_lost = 1;
		}
		break;
	default:
		OF_PRINT_ERROR(("random_loss: unknown state"))
		exit(-1);
	}
	if (is_lost)
		tx_simul_loss++;
	return is_lost;
}

/**
 * simulate random losses using the markov model
 */
bool*
create_loss_array_from_markov_model(UINT32 size)
{
	bool*	is_lost_array;		/* array that will designate the erasures */
	UINT32	i;

	is_lost_array = (bool*) calloc(size, sizeof(bool));
	for (i = 0; i < size; i++)
	{
		if (random_loss() == 1)
			is_lost_array[i] = true;
	}
	return is_lost_array;
}

/**
 * Choose randomly desired_loss_nb "1s" in an array of the given size and allocated by this
 * function.
 */
bool*
create_loss_array_from_erasure_nb (UINT32	size,
				   UINT32	desired_loss_nb)
{
	bool*	is_lost_array;		/* array that will designate the erasures */
	UINT32	actual_loss_nb;
	UINT32	i;
	UINT32	j;

	is_lost_array = (bool*) calloc(size, sizeof(bool));
	actual_loss_nb = 0;
	for (i = 0; i < desired_loss_nb; i++) {
		do {
			j = myrand() % size;
		} while (is_lost_array[j] == true);
		actual_loss_nb++;
		is_lost_array[j] = true;
	}
	if (actual_loss_nb != desired_loss_nb) {
		OF_PRINT_ERROR(("create_loss_arry_from_erasure_nb: WARNING actual_loss_nb (%d) != desired_loss_nb (%d)\n",
				actual_loss_nb, desired_loss_nb))
	}
	return is_lost_array;
}


/**
 * Choose randomly a number of "1s" corresponding to the desired loss probability in an array
 * of the given size and allocated by this function.
 */
bool*
create_loss_array_from_erasure_proba (UINT32	size,
				      double	desired_loss_proba)
{
	bool	*array;	/* array that will designate the erasures */
	UINT32	desired_loss_nb;
	UINT32	actual_loss_nb;
	UINT32	i;
	UINT32	j;

	array = (bool*) calloc(size, sizeof(bool));
	desired_loss_nb = (UINT32)((double)size * desired_loss_proba / 100.0);
	actual_loss_nb = 0;
	for (i = 0; i < desired_loss_nb; i++) {
		do {
			j = myrand() % size;
		} while (array[j] != 0);
		actual_loss_nb++;
		array[j] = 1;
	}
	if (actual_loss_nb != desired_loss_nb) {
		OF_PRINT_ERROR(("create_loss_arry_from_erasure_nb: WARNING actual_loss_nb (%d) != desired_loss_nb (%d)\n",
				actual_loss_nb, desired_loss_nb))
	}
	return array;
}


#if 0
int
loss_oracle (double	p)
{
	double	r;

	r = ((double)myrand() / (double)RAND_MAX);
	if (r < p)
		return 1;	/* lost */
	else
		return 0;	/* not lost */

	return is_lost;
}
#endif


/**
 * Initialize the Pseudo-random number generator with a random seed.
 */
of_status_t
init_prng_with_seed (UINT32	suggested_seed)
{
	UINT32	seed = 0;	/* random seed for the mysrand() function */

	if (suggested_seed != 0) {
		seed = suggested_seed;
	} else {
		/* determine our own random seed */
#ifdef WIN32
		seed = timeGetTime();
#else  /* UNIX */
		/*
		 * the use of /dev/random is safer than urandom but can be a blocking call if the
		 * entropy pool is empty ... so with urandom we often have a value ...
		 * char* dev_random_path="/dev/random";
		 */
		char* dev_random_path = "/dev/urandom";
		FILE* dev_random;
		if ((dev_random = fopen(dev_random_path, "r"))) {
			fread(&seed, sizeof(int), 1, dev_random);
			OF_TRACE_LVL(1,("init_prng_with_seed: getting the seed %d from /dev/urandom\n", seed));
			fclose(dev_random);
		} else {
			/* /dev/random is not available so we take usec as random number generator*/
			struct timeval	tv;
			if (gettimeofday(&tv, NULL) < 0) {
				OF_PRINT_ERROR(("init_prng_with_seed: ERROR, gettimeofday() failed:"))
				return OF_STATUS_ERROR;
			}
			seed = (int)tv.tv_usec;
		}
#endif /* OS */
	}
	mysrand(seed);
	OF_PRINT(("prng seed=%u\n", seed))
	return OF_STATUS_OK;
}


#if 0
//#ifdef DEBUG
int
count_received_pkts (block_t	*blk,
		     packet_t	*pkt,
		     char**	dataDest)
{
	UINT32	count1 = 0;
	UINT32	count2 = 0;
	UINT32	i;

	// check there are indeed that number of received pkts
	for (i = blk->first_pkt_idx;
	     i < blk->first_pkt_idx + blk->k;
	     i++) {
		if (dataDest[i] == NULL) {
			/* not received */
			continue;
		}
		/* received */
		count1++;
	}
	for (i = blk->first_repair_pkt_idx;
	     i < blk->first_repair_pkt_idx + blk->n - blk->k;
	     i++) {
		if (dataDest[i] == NULL) {
			/* not received */
			continue;
		}
		/* received */
		count2++;
	}
	OF_TRACE_LVL(2, ("BLOCK seq=%d: k/n=(%d/%d), pkt_rx=%d\n\tDATA recvd=%d, REPAIR recvd=%d\n",
		pkt->blk_seq, blk->k, blk->n, blk->pkt_rx, count1, count2))
	return (count1+count2);
}
#endif // DEBUG


#ifdef OF_DEBUG
/*
 * Warning, this function is not compatible with a normal use of eperftool...
 * Said differently, do not expect to be able to decode afterwards with the
 * standard receive_and_decode() function. Reason is that get_next_symbol_received
 * is not re-entrant.
 */
void
print_rx_stats (void)
{
	symbol_cb_t	*new_symb_cb;	/* pointer to the symbol just received */
	UINT32		nb_recvd = 0;
	UINT32		nb_lost  = 0;

	while ((new_symb_cb = get_next_symbol_received()) != NULL) {
		nb_recvd++;
		OF_TRACE_LVL(1, ("rx: \tsbn=%u    esi=%u\n", new_symb_cb->sbn, new_symb_cb->esi))
	}
	nb_lost = tot_nb_encoding_symbols - nb_recvd;
	OF_PRINT(("rx stats: %u recvd, %u lost\n", nb_recvd, nb_lost))
}
#endif // OF_DEBUG

