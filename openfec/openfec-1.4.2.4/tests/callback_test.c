/* $Id: callback_test.c 530 2010-10-25 14:43:42Z roca $ */
/*
 * OpenFEC.org AL-FEC Library.
 * (c) Copyright 2009 INRIA - All rights reserved
 * Main authors:	Mathieu Cunche (INRIA)
 *			Jonathan Detchart (INRIA)
 *			Julien Laboure (INRIA)
 *			Christoph Neumann (INRIA)
 *			Vincent Roca (INRIA)
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

#define OF_USE_ENCODER
#define OF_USE_DECODER
#include "../src/lib_common/of_openfec_api.h"
#include "../src/lib_stable/ldpc_staircase/of_ldpc_staircase_api.h"
#include "../src/lib_stable/reed-solomon_gf_2_8/of_reed-solomon_gf_2_8_api.h"
#include "../src/lib_common/of_debug.h"

#include <stdio.h>
#include <stdlib.h>
#define CODEC_MAX 10

int nb_source=0,nb_repair=0;

void* source_cb(void *context,UINT32 size,UINT32 esi)
{
    nb_source++;
    printf ("source\n");
    return malloc(1024);
}

void* repair_cb(void *context,UINT32 size,UINT32 esi)
{
  nb_repair++;
  printf ("repair\n");
}

int main()
{
    int context;
    int i,j,k=OF_CODEC_LDPC_STAIRCASE_STABLE;
    of_session_t *ses;
    of_status_t ret;
    // LDPC parameters
      of_ldpc_parameters_t		params;
      params.nb_source_symbols		= 1000;
      params.nb_repair_symbols		= 500;
      params.encoding_symbol_length	= 1024;
      params.prng_seed			= rand();
      params.N1				= 5;


    char **orig_symb_tab;
    void **encoding_symb_tab;
    //for (k=0;k<CODEC_MAX;k++)
    {
	// create an instance of an encoder
      if ((ret = of_create_codec_instance(&ses,k,OF_ENCODER,1)) != OF_STATUS_OK)
      {
	printf("error in of_create_codec_instance\n");
	goto error;      
      }

      if (of_set_fec_parameters(ses, (of_parameters_t*)&params) != OF_STATUS_OK) {
	      printf("error in of_set_fec_parameters\n");
	     goto error;
      }
      if( of_set_callback_functions(ses,source_cb,repair_cb,&context) != OF_STATUS_OK) {
	  printf ("error in of_set_fec_parameters\n");
      }
      
      orig_symb_tab = (char**)calloc(params.nb_source_symbols,sizeof(char*));
      encoding_symb_tab = (void*)calloc(params.nb_source_symbols+params.nb_repair_symbols,sizeof(void*));
      for(i=0;i<params.nb_source_symbols;i++)
      {
	  orig_symb_tab[i] = (char*)calloc(1,params.encoding_symbol_length);
	  encoding_symb_tab[i] = (void*)orig_symb_tab[i];
	  for (j=0;j<params.encoding_symbol_length;j++)
	  {
		orig_symb_tab[i][j] = rand();
	  }
      }
      
      for(i=params.nb_source_symbols;i<params.nb_source_symbols+params.nb_repair_symbols;i++)
      {
	  encoding_symb_tab[i] = (void*)calloc(1,params.encoding_symbol_length);
	  if (of_build_repair_symbol(ses,encoding_symb_tab,i) != OF_STATUS_OK)
	  {
	      printf("of_build_repair_symbol: ");;
	      //goto error;
	  }
      }
       if ((ret = of_release_codec_instance(ses)) != OF_STATUS_OK)
      {
	printf("of_release_codec_instance: ");;
	//goto error;
      }

	//create a decoder instance
      if ((ret = of_create_codec_instance(&ses,k,OF_DECODER,1)) != OF_STATUS_OK)
      {
	printf("of_create_codec_instance: ");
	;
	//goto error;
      }

      if (of_set_fec_parameters(ses, (of_parameters_t*)&params) != OF_STATUS_OK) {
	      printf(("of_set_fec_parameters(): "));;
	     // goto error;
      }
      if( of_set_callback_functions(ses,source_cb,repair_cb,&context) != OF_STATUS_OK) {
	  printf (("of_set_fec_parameters(): "));;
      }     
      for (i=380;i<1400;i++) //take 1020 symbols
      {
		  of_decode_with_new_symbol(ses,encoding_symb_tab[i],i);
      }

      of_finish_decoding(ses);
      printf("fini\n");

      if ((ret = of_release_codec_instance(ses)) != OF_STATUS_OK)
      {
	printf("of_release_codec_instance: ");;
	//goto error;
      }      
      free(encoding_symb_tab);
      free(orig_symb_tab);
    }
    printf ("nb : %i,%i\n",nb_source,nb_repair);
    printf("OK\n");
    return 0;
    
error:
  printf("error\n");
  return -1;
}
