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

//#include "simple_client_server.h"
#include "inc.h"

//#define TEST
//#define TEST_LOCAL_LOSS

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

extern cJSON* mystr2json(char *text);
extern int GetvalueInt(cJSON *json, char *key);
extern float GetvalueFloat(cJSON *json, char *key);
extern cJSON* renewJson(cJSON *json, char *key, int ivalue, char *cvalue, cJSON *subJson);
extern cJSON* renewJsonInt(cJSON *json, char *key, int ivalue);
extern cJSON* renewJsonStr(cJSON *json, char *key, char *cvalue);
extern cJSON* renewJsonArray1(cJSON *json, char *key, short *value, int len);
extern cJSON* deleteJson(cJSON *json);

extern unsigned char test_data[100 * 1024];

//static int global_fec_obj_status = 0;
//FecObj global_fec_objs[MAX_OBJ_NUM];
//char global_fec_outparam[MAX_OBJ_NUM][MAX_OUTCHAR_SIZE];

int fec_encode_close(FecObj *obj)
{
    UINT32		esi;					/* Encoding Symbol ID, used to identify each encoding symbol */
    /* Cleanup everything... */
	if (obj->ses)
	{
		of_release_codec_instance(obj->ses);
		obj->ses = NULL;
		//printf("fec_encode_close: of_release_codec_instance: ok \n");
	}
	if (obj->params)
	{
		free(obj->params);
		obj->params = NULL;
		//printf("fec_encode_close: params: ok \n");
	}
	//printf("fec_encode_close: enc_symbols_tab: obj->rand_order= %d \n", obj->rand_order);

	if (obj->rand_order) {
		free(obj->rand_order);
		obj->rand_order = NULL;
		//printf("fec_encode_close: rand_order: ok \n");
	}

	//printf("fec_encode_close: enc_symbols_tab: obj->n= %d \n", obj->n);
	if (obj->enc_symbols_tab)
	{
		for (esi = 0; esi < obj->n; esi++)
		{
			if (obj->enc_symbols_tab[esi])
			{
			    //printf("fec_encode_close: enc_symbols_tab: esi= %d \n", esi);
				free(obj->enc_symbols_tab[esi]);
				//obj->enc_symbols_tab[esi] = NULL;

			}
		}
		free(obj->enc_symbols_tab);
		obj->enc_symbols_tab = NULL;
		//printf("fec_encode_close: enc_symbols_tab: ok \n");
	}
	if (obj->pkt_with_fpi)
	{
		free(obj->pkt_with_fpi);
		obj->pkt_with_fpi = NULL;
		//printf("fec_encode_close: params: ok \n");
	}
}
int fec_init(FecObj *obj)
{
    int ret = 0;
    obj->Obj_id = -1;
    obj->json = NULL;
    for(int i = 0; i < 4; i++)
    {
        memset(obj->outparam[i], 0, sizeof(char) * MAX_OUTCHAR_SIZE);
    }
    obj->inSize = NULL; //should free when delete obj
    obj->seq_num = 0;
    obj->rtp_head_size;
    obj->codec_id = 0;				/* identifier of the codec to use */
	obj->ses 		= NULL;			/* openfec codec instance identifier */
	obj->params		= NULL;			/* structure used to initialize the openfec session */
	obj->enc_symbols_tab	= NULL;			/* table containing pointers to the encoding (i.e. source + repair) symbols buffers */
	obj->src_symbols_tab	= NULL;
	obj->recvd_symbols_tab= NULL;
	obj->symb_sz_32	= SYMBOL_SIZE / 4;	/* symbol size in units of 32 bit words */
	obj->k = DEFAULT_K;					/* number of source symbols in the block */
	obj->n = 0;					/* number of encoding symbols (i.e. source + repair) in the block */
	obj->loss_rate = 0.0;
	obj->code_rate = 0.0;
	//obj->esi = 0;					/* Encoding Symbol ID, used to identify each encoding symbol */
	obj->rand_order	= NULL;			/* table used to determine a random transmission order. This randomization process
								 * is essential for LDPC-Staircase optimal performance */
    obj->pkt_with_fpi	= NULL;			/* buffer containing a fixed size packet plus a header consisting only of the FPI */
	//obj->fec_oti;				/* FEC Object Transmission Information as sent to the client */
	obj->lost_after_index= -1;			/* all the packets to send after this index are considered as lost during transmission */

    return ret;
}
#if 0
int init_fec_obj(int id)
{
    if (global_fec_obj_status <= 0)
    {
        for (int i = 0; i < MAX_OBJ_NUM; i++)
        {
            fec_init(&global_fec_objs[i]);
        }
        global_fec_obj_status += 1;
    }
    else
    {
    }
    return global_fec_obj_status;
}
#endif
int get_symbol_size(FecObj *obj, char *data, short *inSize)
{
    int ret = 0;
    int i = 0;
    int max_size = 0;
    RTP_FIXED_HEADER  *rtp_hdr = NULL;
    EXTEND_HEADER *rtp_ext = NULL;
    rtp_hdr = (RTP_FIXED_HEADER *)data;
    int extlen = 0;
    int rptHeadSize = sizeof(RTP_FIXED_HEADER);
    if(rtp_hdr->extension)
    {
        rtp_ext = (EXTEND_HEADER *)&data[sizeof(RTP_FIXED_HEADER)];
        int rtp_extend_length = rtp_ext->rtp_extend_length;
		rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
		extlen = (rtp_extend_length + 1) << 2;
    }
    do
    {
        int size = inSize[i] - rptHeadSize - extlen;
        if(size > max_size)
        {
            max_size = size;
        }
        i++;
    }while(inSize[i] > 0);
    obj->k = i;
    int max_size2 = max_size;// + 2;//2自己用於寫入數據大小；
    obj->symbol_size = ((max_size2 >> 2) << 2) + ((max_size2 & 3) ? 4 : 0);
    obj->symb_sz_32	= obj->symbol_size >> 2;
    ret = obj->symbol_size;
    //k/n = 1-lost_rate = code_rate
	obj->n = (UINT32)(((double)obj->k / (double)obj->code_rate) + 0.9);
    if ((obj->enc_symbols_tab = (void**) calloc(obj->n, sizeof(void*))) == NULL) {
		OF_PRINT_ERROR(("no memory (calloc failed for enc_symbols_tab, n=%u)\n", obj->n))
		ret = -1;
	}
	//printf("get_symbol_size: obj->symbol_size= %d \n", obj->symbol_size);
	//printf("get_symbol_size: obj->k= %d \n", obj->k);
	//printf("get_symbol_size: obj->n= %d \n", obj->n);
    return ret;
}
int fec_create_data(FecObj *obj, char *data, short *inSize)
{
    int ret = 0;

    if(!data)
    {
        for (int esi = 0; esi < obj->k; esi++ )
	    {
		    if ((obj->enc_symbols_tab[esi] = calloc(obj->symb_sz_32, sizeof(UINT32))) == NULL)
		    {
			    OF_PRINT_ERROR(("no memory (calloc failed for enc_symbols_tab[%d])\n", esi))
			    ret = -1;
			    return ret;
		    }
		    memset(obj->enc_symbols_tab[esi], (char)(esi + 1), obj->symbol_size);
#ifdef TEST
            memcpy(&test_data[esi * obj->symbol_size], obj->enc_symbols_tab[esi], obj->symbol_size);
#endif
		    if (VERBOSITY > 1)
		    {
			    printf("src[%03d]= ", esi);
			    dump_buffer_32(obj->enc_symbols_tab[esi], 1);
		    }
	    }
    }
    else{
        int offset = 0;
        int frame_size = obj->symb_sz_32 * sizeof(UINT32);
        frame_size = get_symbol_size(obj, data, inSize);
        for (int esi = 0; esi < obj->k; esi++ )
	    {
	        RTP_FIXED_HEADER  *rtp_hdr = NULL;
            EXTEND_HEADER *rtp_ext = NULL;
            rtp_hdr = (RTP_FIXED_HEADER *)&data[offset];
            int extlen = 0;
            int rptHeadSize = sizeof(RTP_FIXED_HEADER);
            if(rtp_hdr->extension)
            {
                rtp_ext = (EXTEND_HEADER *)&data[offset + sizeof(RTP_FIXED_HEADER)];
                int rtp_extend_length = rtp_ext->rtp_extend_length;
			    rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
			    extlen = (rtp_extend_length + 1) << 2;
            }
            int rawSize = inSize[esi] - (rptHeadSize + extlen);
            if(inSize[esi] != rtp_ext->rtp_pkt_size)
            {
                printf("error: fec_create_data: inSize[esi]=%d \n", inSize[esi]);
            }
            //printf("fec_create_data: inSize[esi]=%d \n", inSize[esi]);
            //printf("fec_create_data: rawSize=%d \n", rawSize);
            //printf("fec_create_data: extlen=%d \n", extlen);
            //printf("fec_create_data: inSize[esi]=%d \n", inSize[esi]);
            obj->rtp_head_size = rptHeadSize + extlen + sizeof(FEC_HEADER);
            if ((obj->enc_symbols_tab[esi] = calloc(1, frame_size)) == NULL)
		    {
			    OF_PRINT_ERROR(("fec_create_data: no memory (calloc failed for enc_symbols_tab[%d])\n", esi))
			    ret = -1;
			    return ret;
		    }
		    char *p0 = (char *)obj->enc_symbols_tab[esi];// + obj->rtp_head_size;
		    char *p1 = (char *)&data[offset + rptHeadSize + extlen];
		    //===
		    char *lastbyte = (char *)&p1[rawSize - 1];
		    //int byte3 = lastbyte[-3];
		    //int byte2 = lastbyte[-2];
		    //int byte1 = lastbyte[-1];
		    int byte0 = lastbyte[0];

		    //printf("fec_create_data: lastbyte[-3]=% d \n", byte3);
		    //printf("fec_create_data: lastbyte[-2]=% d \n", byte2);
		    //printf("fec_create_data: lastbyte[-1]=% d \n", byte1);
		    //printf("fec_create_data: lastbyte[0]=% d \n", byte0);
		    //===
		    if(lastbyte[0] == 0)
		    {
		        printf("fec_create_data: lastbyte is zero !!! \n");
		        char *filename = "/home/gxh/works/simple_server_log.txt";
		        FILE *logfp = fopen(filename, "a");
		        if(logfp)
		        {
		            fprintf(logfp, "zero%d\n", rawSize);
		            fflush(logfp);
		            fclose(logfp);
		        }
		    }
		    //short *raw_size = (short *)&p0[frame_size - 2];
		    memcpy(p0, p1, rawSize);
		    //raw_size[0] = rawSize;//包的原始大小保存
		    //printf("fec_create_data: %d, rawSize= %d \n", esi, rawSize);
		    memset(&p0[rawSize], 0, (frame_size - rawSize));

	        //offset += inSize[esi];
#ifdef TEST
            memcpy(&test_data[offset], obj->enc_symbols_tab[esi], frame_size);
#endif
	        offset += inSize[esi];
	    }
    }
    //printf("fec_create_data: end \n");
    return ret;
}
/*
由於存在補齊，導致無法正確獲得包原始大小;
解決的辦法之一：
在每個symbol_size大小的包的最後2個字節，注入該包的原始大小；
NB: the 0x0 value is avoided since it is a neutral element in the target finite fields, i.e.
it prevents the detection of symbol corruption
*/
int packet_fec(FecObj *obj, char *indata, short *inSize, char *outbuf, short *pktSize)
{
    int ret;
    int idx = 0;
    int offset = 0;
    int offset2 = 0;
    int seqnum = 0;
    int extlen = 0;
    int test_idx = 0;//1;//rand() % obj->k;//1;//test
    int loss_num = 0;
    char *rtp_head = NULL;//[sizeof(RTP_FIXED_HEADER) + sizeof(EXTEND_HEADER) + sizeof(FEC_HEADER)];
    int rptHeadSize = sizeof(RTP_FIXED_HEADER);
    RTP_FIXED_HEADER  *rtp_hdr = NULL;
    EXTEND_HEADER *rtp_ext = NULL;
    FEC_HEADER *fec_hdr = NULL;
    int rtp_head_size = obj->rtp_head_size;
    int symbol_size = obj->symbol_size;
    //uint16_t start_seqnum = 0;
    for (int esi = 0; esi < obj->n; esi++)
	{
	    //
	    if(esi < obj->k)
	    {
	        int rawSize = symbol_size;
            rtp_hdr = (RTP_FIXED_HEADER *)&indata[offset2];
            seqnum = rtp_hdr->seq_no;//需要傳回rtp打包代碼中
            //if(!esi)
            //{
            //    start_seqnum = seqnum;
            //}
            if(rtp_hdr->extension)
            {
                rtp_ext = (EXTEND_HEADER *)&indata[offset2 + sizeof(RTP_FIXED_HEADER)];
                int nack_type = rtp_ext->nack_type;
                int rtp_extend_length = rtp_ext->rtp_extend_length;
			    rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
			    int extlen0 = (rtp_extend_length + 1) << 2;
			    //
                rtp_extend_length = ((sizeof(EXTEND_HEADER) + sizeof(FEC_HEADER)) >> 2) - 1;//!!!
                extlen = (rtp_extend_length + 1) << 2;
				rtp_ext->rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));//更新值
			    unsigned int rtp_pkt_size = rtp_ext->rtp_pkt_size;//
			    if(rtp_pkt_size != inSize[esi])
			    {
			        printf("error: packet_fec: rtp_pkt_size= %d \n", rtp_pkt_size);
			    }
			    //printf("packet_fec: rtp_pkt_size= %d \n", rtp_pkt_size);
			    //printf("packet_fec: inSize[esi]= %d \n", inSize[esi]);
			    rawSize = rtp_ext->rtp_pkt_size - (rptHeadSize + extlen0);
			    rtp_ext->rtp_pkt_size += sizeof(FEC_HEADER);//更新值
			    //rtp_ext->start_seqnum = start_seqnum;
			    rtp_ext->enable_fec = 1;
            }
            char *p0 = (char *)&outbuf[offset];
            char *p1 = (char *)obj->enc_symbols_tab[esi];
            //memcpy(&p0[rtp_head_size], p1, symbol_size);//copy raw data
            //printf("packet_fec: rawSize= %d \n", rawSize);
            memcpy(&p0[rtp_head_size], p1, rawSize);
            char *p2 = (char *)&indata[offset2];
#ifdef TEST_LOCAL_LOSS
            int max_loss_num = obj->n - obj->k;
            test_idx = rand() % obj->k;


            //if((esi == test_idx) && (loss_num < max_loss_num))
            if(esi < max_loss_num)
            {
                //loss_num++;
            }
            else{
                memcpy(p0, p2, (rptHeadSize + extlen));//copy head data
            }


#else
            memcpy(p0, p2, (rptHeadSize + extlen));//copy head data
#endif
            if(!esi)
            {
                rtp_head = p0;
            }
            fec_hdr = (FEC_HEADER *)&p0[rptHeadSize + extlen - sizeof(FEC_HEADER)];
            fec_hdr->codec_id = obj->codec_id;
            fec_hdr->k = obj->k;
            fec_hdr->n = obj->n;
            fec_hdr->symbol_size = symbol_size >> 2;
            fec_hdr->fec_seq_no = esi;

            //printf("packet_fec: symbol_size= %d \n", symbol_size);
            //printf("packet_fec: fec_hdr->symbol_size= %d \n", fec_hdr->symbol_size);

            offset2 += inSize[esi];
#ifdef TEST_LOCAL_LOSS
            //if((esi == test_idx) && (loss_num < max_loss_num))
            if(esi < max_loss_num)
            {
                loss_num++;
            }
            else{
                offset += symbol_size + rtp_head_size;
                pktSize[idx] = symbol_size + rtp_head_size;
                idx++;
            }
#else
           //offset += symbol_size + rtp_head_size;
           //pktSize[idx] = symbol_size + rtp_head_size;
           offset += rtp_ext->rtp_pkt_size;
           pktSize[idx] = rtp_ext->rtp_pkt_size;
           //printf("packet_fec: pktSize[idx]= %d, idx=%d \n", pktSize[idx], idx);
           idx++;
#endif
        }
        else{
            seqnum += 1;
            if ((seqnum) >= MAX_USHORT)
			{
				seqnum = 0;
			}
            char *p0 = (char *)&outbuf[offset];
            char *p1 = (char *)obj->enc_symbols_tab[esi];
            memcpy(&p0[rtp_head_size], p1, symbol_size);
            memcpy(p0, rtp_head, (rptHeadSize + extlen));
            rtp_hdr = (RTP_FIXED_HEADER *)p0;
            //NALU_HEADER *nalu_hdr = (NALU_HEADER *)&p0[rptHeadSize + extlen];
            rtp_hdr->seq_no = seqnum;
            rtp_ext = (EXTEND_HEADER *)&p0[sizeof(RTP_FIXED_HEADER)];
            rtp_ext->first_slice = 0;

            unsigned  int timestamp = rtp_hdr->timestamp;
            //printf("packet_fec: timestamp= %d \n", timestamp);
            //int rtp_extend_length = ((rtp_head_size + symbol_size) >> 2) - 1;
            int nack_type = rtp_ext->nack_type;
            int rtp_extend_length = ((sizeof(EXTEND_HEADER) + sizeof(FEC_HEADER)) >> 2) - 1;
            rtp_ext->rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
            rtp_ext->rtp_pkt_size = symbol_size + rtp_head_size;
            //rtp_ext->start_seqnum = start_seqnum;
            //
            fec_hdr = (FEC_HEADER *)&p0[rptHeadSize + extlen - sizeof(FEC_HEADER)];
            fec_hdr->codec_id = obj->codec_id;
            fec_hdr->k = obj->k;
            fec_hdr->n = obj->n;
            fec_hdr->symbol_size = symbol_size >> 2;
            fec_hdr->fec_seq_no = esi;

            offset += symbol_size + rtp_head_size;
            pktSize[idx] = symbol_size + rtp_head_size;
            //printf("packet_fec: pktSize[idx]= %d, idx=%d \n", pktSize[idx], idx);
            idx++;
        }
	}
	seqnum += 1;
    if ((seqnum) >= MAX_USHORT)
	{
		seqnum = 0;
	}
	obj->seq_num = seqnum;
	ret = idx;// obj->n;
	//printf("packet_fec: loss_num=%d \n", loss_num);
	//printf("packet_fec: end \n");
    return ret;
}

int fec_encode2(FecObj *obj, char *indata, short *inSize, char *outbuf, short *pktSize)
{
    int ret = 0;
    UINT32		esi;					/* Encoding Symbol ID, used to identify each encoding symbol */
    int i;
    int offset = 0;
    int idx = 0;

    //printf("0: fec_encode2: enc_symbols_tab: obj->k= %d \n", obj->k);

    fec_create_data(obj, indata, inSize);

    //printf("1: fec_encode2: enc_symbols_tab: obj->k= %d \n", obj->k);

	//obj->n = (UINT32)floor((double)obj->k / (double)obj->code_rate);
	//obj->n = (UINT32)((double)obj->k / (double)obj->code_rate + 0.8);
    if(obj->n >= 1024)
    {
        printf("warning: gxh: fec_encode2: LDPC-obj->n= %d \n", obj->n);
    }
	/* Choose which codec is the most appropriate. If small enough, choose Reed-Solomon (with m=8), otherwise LDPC-Staircase.
	 * Then finish the openfec session initialization accordingly */
	if (obj->n <= 255)
	{
	    //printf("Reed-Solomon \n");
		/* fill in the code specific part of the of_..._parameters_t structure */
		of_rs_2_m_parameters_t	*my_params;

		//printf("\nInitialize a Reed-Solomon over GF(2^m) codec instance, (n, k)=(%u, %u)...\n", obj->n, obj->k);
		obj->codec_id = OF_CODEC_REED_SOLOMON_GF_2_M_STABLE;
		if ((my_params = (of_rs_2_m_parameters_t *)calloc(1, sizeof(* my_params))) == NULL)
		{
			OF_PRINT_ERROR(("no memory for codec %d\n", obj->codec_id))
			ret = -1;
			goto end;
		}
		my_params->m = 8;
		obj->params = (of_parameters_t *) my_params;
	}
	else
	{
	    //printf("gxh: fec_encode2: LDPC-obj->n= %d \n", obj->n);
	    //printf("gxh: fec_encode2: LDPC-Staircase \n");
		/* fill in the code specific part of the of_..._parameters_t structure */
		of_ldpc_parameters_t	*my_params;

		MYPRINT("gxh: fec_encode2: LDPC-Staircase, (n, k)=(%u, %u) \n", obj->n, obj->k);
		obj->codec_id = OF_CODEC_LDPC_STAIRCASE_STABLE;
		if ((my_params = (of_ldpc_parameters_t *)calloc(1, sizeof(* my_params))) == NULL)
		{
			OF_PRINT_ERROR(("no memory for codec %d\n", obj->codec_id))
			ret = -1;
			goto end;
		}
		my_params->prng_seed	= 146348057;//rand();
		my_params->N1		= 7;
		obj->params = (of_parameters_t *) my_params;
	}
	//printf("gxh: fec_encode2: obj->codec_id= %d \n", obj->codec_id);
	obj->params->nb_source_symbols	= obj->k;		/* fill in the generic part of the of_parameters_t structure */
	obj->params->nb_repair_symbols	= obj->n - obj->k;
	obj->params->encoding_symbol_length	= obj->symbol_size;

    //printf("2: fec_encode2: enc_symbols_tab: obj->k= %d \n", obj->k);

	/* Open and initialize the openfec session now... */
	if ((ret = of_create_codec_instance(&obj->ses, obj->codec_id, OF_ENCODER, VERBOSITY)) != OF_STATUS_OK)
	{
		OF_PRINT_ERROR(("of_create_codec_instance() failed\n"))
		ret = -1;
		goto end;
	}
	if (of_set_fec_parameters(obj->ses, obj->params) != OF_STATUS_OK)
	{
		OF_PRINT_ERROR(("of_set_fec_parameters() failed for codec_id %d\n", obj->codec_id))
		ret = -1;
		goto end;
	}

    //printf("3: fec_encode2: enc_symbols_tab: obj->k= %d \n", obj->k);

	/* Allocate and initialize our source symbols...
	 * In case of a file transmission, the opposite takes place: the file is read and partitionned into a set of k source symbols.
	 * At the end, it's just equivalent since there is a set of k source symbols that need to be sent reliably thanks to an FEC
	 * encoding. */
	//printf("\nFilling source symbols...\n");
	//if ((obj->enc_symbols_tab = (void**) calloc(obj->n, sizeof(void*))) == NULL) {
	//	OF_PRINT_ERROR(("no memory (calloc failed for enc_symbols_tab, n=%u)\n", obj->n))
	//	ret = -1;
	//	goto end;
	//}
	/* In order to detect corruption, the first symbol is filled with 0x1111..., the second with 0x2222..., etc.
	 * NB: the 0x0 value is avoided since it is a neutral element in the target finite fields, i.e. it prevents the detection
	 * of symbol corruption */


	/* Now build the n-k repair symbols... */
	//printf("\nBuilding repair symbols...\n");
	for (esi = obj->k; esi < obj->n; esi++)
	{
		if ((obj->enc_symbols_tab[esi] = (char*)calloc(obj->symb_sz_32, sizeof(UINT32))) == NULL)
		{
			OF_PRINT_ERROR(("no memory (calloc failed for enc_symbols_tab[%d])\n", esi))
			ret = -1;
			goto end;
		}
		if (of_build_repair_symbol(obj->ses, obj->enc_symbols_tab, esi) != OF_STATUS_OK) {
			OF_PRINT_ERROR(("ERROR: of_build_repair_symbol() failed for esi=%u\n", esi))
			ret = -1;
			goto end;
		}
		if (VERBOSITY > 1)
		{
			//printf("repair[%03d]= ", esi);
			///dump_buffer_32(obj->enc_symbols_tab[esi], 4);
		}
	}

	//printf("4: fec_encode2: enc_symbols_tab: obj->k= %d \n", obj->k);

    //no used
	obj->lost_after_index = obj->n * (1 - obj->loss_rate);
    //printf("fec_encode: obj->lost_after_index= %d \n", obj->lost_after_index);
	if (obj->lost_after_index < obj->k)
	{
	    //printf("fec_encode: obj->k= %d \n", obj->k);
	    //printf("fec_encode: obj->n= %d \n", obj->n);
	    obj->lost_after_index = obj->k;
	    //printf("fec_encode: obj->lost_after_index= %d \n", obj->lost_after_index);
		//OF_PRINT_ERROR(("The loss rate %f is to high: only %u packets will be sent, whereas k=%u\n", LOSS_RATE, obj->lost_after_index, obj->k))
		//ret = -1;
		//goto end;
	}

    ret = packet_fec(obj, indata, inSize, outbuf, pktSize);

    //printf("5: fec_encode2: enc_symbols_tab: obj->k= %d \n", obj->k);

    //printf("fec_encode: ret= %d \n", ret);
end:
	fec_encode_close(obj);
	//printf("6: fec_encode2: enc_symbols_tab: obj->k= %d \n", obj->k);

    return ret;
}

HCSVC_API
int api_fec_encode(char *handle, char *data, char *param, char *outbuf, char *outparam[])
{
    int ret = 0;
    //init_fec_obj(id);
    //if (id < MAX_OBJ_NUM)
    {
        long long *testp = (long long *)handle;
        CodecObj *codecObj = (CodecObj *)testp[0];
        FecObj *obj = (FecObj *)codecObj->fecEncObj;//&global_fec_objs[id];
        int id = codecObj->Obj_id;
        if (!obj)
        {
            obj = (FecObj *)calloc(1, sizeof(FecObj));
            codecObj->fecEncObj = obj;
            fec_init(obj);
            obj->Obj_id = id;
        }
        //printf("api_fec_encode: obj->rand_order= %d \n", obj->rand_order);

        obj->json = mystr2json(param);
        int symbol_size = GetvalueInt(obj->json, "symbol_size");
        if(symbol_size)
        {
            obj->symbol_size = symbol_size;
            obj->symb_sz_32	= obj->symbol_size / 4;
        }
        int k = GetvalueInt(obj->json, "k");
        if(k)
        {
            obj->k = k;
        }
        //printf("api_fec_encode: symbol_size= %d \n", symbol_size);
        //printf("api_fec_encode: k= %d \n", k);
        //printf("api_fec_encode: param= %s \n", param);
        float loss_rate = GetvalueFloat(obj->json, "loss_rate");
        if(loss_rate)
        {
            obj->loss_rate = loss_rate;
        }
        float code_rate = GetvalueFloat(obj->json, "code_rate");
        if(code_rate)
        {
            obj->code_rate = code_rate;
        }
        //printf("1: api_fec_encode: obj->k= %d \n", obj->k);
        //printf("api_fec_encode: obj->loss_rate= %f \n", obj->loss_rate);
        //printf("api_fec_encode: obj->code_rate= %f \n", obj->code_rate);
        cJSON *cjsonArr = cJSON_GetObjectItem(obj->json, "inSize");
        int pkt_mem_num = 0;
        int pktLen = 0;
        if( NULL != cjsonArr ){
            int i = 0;
            do
            {
                cJSON *cjsonTmp = cJSON_GetArrayItem(cjsonArr, i);
                if( NULL == cjsonTmp )
                {
                    //printf("rtp_packet2raw: no member \n");
                    break;
                }
                int num = cjsonTmp->valueint;
                //printf("rtp_packet2raw: num= %d \n", num);
                i++;
            }while(1);

            //int  array_size = cJSON_GetArraySize(cjsonArr);
            //nal_mem_num = array_size;
            pkt_mem_num = i;
            //printf("rtp_packet2raw: pkt_mem_num= %d \n", pkt_mem_num);
            obj->inSize = calloc(1, sizeof(short) * (pkt_mem_num + 1));

            for( int i = 0 ; i < pkt_mem_num ; i ++ ){
                cJSON * pSub = cJSON_GetArrayItem(cjsonArr, i);
                if(NULL == pSub ){ continue ; }
                //char * ivalue = pSub->valuestring ;
                int ivalue = pSub->valueint;
                obj->inSize[i] = (short)ivalue;//可以不用傳入，通過擴展字段讀入：rtpSize[idx] = rtp_pkt_size;
                //printf("api_fec_encode: i=%d, obj->inSize[i]= %d \n", i, obj->inSize[i]);
                pktLen += ivalue;
            }
            obj->inSize[i] = 0;
        }
        else{
            //return ret;
        }
        //printf("2: api_fec_encode: obj->rand_order= %d \n", obj->rand_order);
        if(pkt_mem_num >= 1024)
        {
            printf("warning: api_fec_encode: pkt_mem_num= %d \n", pkt_mem_num);
        }
        //
        short pktSize[MAX_FEC_PKT_NUM];
        //ret = fec_encode(obj, NULL, obj->inSize, outbuf, pktSize);
        ret = fec_encode2(obj, data, obj->inSize, outbuf, pktSize);
        //printf("api_fec_encode: ret=%d \n ", ret);
        outparam[0] = obj->outparam[0];
        outparam[1] = obj->outparam[1];
        if(ret > 0)
        {
            //char text[2048] = "";//2048/4=512//512*1400=700kB
            sprintf(obj->outparam[1], "%d", obj->seq_num);
	        int sum = 0;

            for (int i = 0; i < ret; i++)
	        {
	            int size = pktSize[i];
	            //printf("api_fec_encode: size=%d, i=%d \n", size, i);
                sum += size;
	        }

	        cJSON *json2 = NULL;
            json2 = renewJsonArray1(json2, "rtpSize", pktSize, ret);
            char *jsonStr = cJSON_Print(json2);//比较耗时
            //printf("api_fec_encode: jsonStr=%s \n", jsonStr);
            deleteJson(json2);
            strcpy(obj->outparam[0], jsonStr);
            cJSON_free(jsonStr);
            ret = sum;
        }
        if(obj->inSize)
        {
            free(obj->inSize);
            obj->inSize = NULL;
        }
        deleteJson(obj->json);
        obj->json = NULL;
        //printf("3: api_fec_encode: obj->rand_order= %d \n", obj->rand_order);
    }
    return ret;
}
HCSVC_API
int api_fec_server_main() //int argc, char* argv[]
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

#if 1
    k = DEFAULT_K;
#else
	if (argc == 1)
	{
		/* k value is ommited, so use default */
		k = DEFAULT_K;
	}
	else
	{
		k = atoi(argv[1]);
	}
#endif
	n = (UINT32)floor((double)k / (double)CODE_RATE);
	/* Choose which codec is the most appropriate. If small enough, choose Reed-Solomon (with m=8), otherwise LDPC-Staircase.
	 * Then finish the openfec session initialization accordingly */
	if (n <= 255)//if(n < 1024)
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
		my_params->m = 8;//10;//9;//8;
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
		my_params->prng_seed	= 463480570;//rand();
		printf("gxh: api_fec_server_main: my_params->prng_seed= %d \n", my_params->prng_seed);
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
		memset(enc_symbols_tab[esi], (char)(0), 12);


		if (VERBOSITY > 1)
		{
			printf("src[%03x]= ", esi + 1);
			dump_buffer_32(enc_symbols_tab[esi], 4);
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
			printf("repair[%03x]= ", esi + 1);
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
	//lost_after_index = n * (1 - LOSS_RATE) + 1;//gxh
	if (lost_after_index < k)
	{
		OF_PRINT_ERROR(("The loss rate %f is to high: only %u packets will be sent, whereas k=%u\n", LOSS_RATE, lost_after_index, k))
		ret = -1;
		goto end;
	}
	lost_after_index += 2;//gxh
	printf("gxh: api_fec_server_main: k= %d \n", k);
	printf("gxh: api_fec_server_main: n= %d \n", n);
	printf("gxh: api_fec_server_main: lost_after_index= %d \n", lost_after_index);
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
		//printf("%05d => sending symbol %u (%s)\n", i + 1, rand_order[i], (rand_order[i] < k) ? "src" : "repair");
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

