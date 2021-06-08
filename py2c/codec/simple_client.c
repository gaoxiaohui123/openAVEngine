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

//#include "simple_client_server.h"
#include "inc.h"

//#define TEST

/*
 * Chose which decoding method to use... Both should be equivalent.
 */
#define USE_DECODE_WITH_NEW_SYMBOL

#ifdef linux
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
#endif
/**
 * Dumps len32 32-bit words of a buffer (typically a symbol).
 */
static void	dump_buffer_32 (void	*buf,
				UINT32	len32);


/*************************************************************************************************/
extern int GetvalueInt(cJSON *json, char *key);
extern cJSON* renewJsonArray1(cJSON *json, char *key, short *value, int len);
//extern int init_fec_obj(int id);
extern int fec_init(FecObj *obj);
//extern FecObj global_fec_objs[MAX_OBJ_NUM];
//extern char global_fec_outparam[MAX_OBJ_NUM][MAX_OUTCHAR_SIZE];

extern unsigned char test_data[100 * 1024];

int fec_decode_close2(FecObj *obj)
{
    UINT32		esi;					/* Encoding Symbol ID, used to identify each encoding symbol */
    /* Cleanup everything... */
	if (obj->ses)
	{
		of_release_codec_instance(obj->ses);
		obj->ses = NULL;
	}
	if (obj->params)
	{
		free(obj->params);
		obj->params = NULL;
	}
	//if (obj->pkt_with_fpi)
	//{
	//	free(obj->pkt_with_fpi);
	//	obj->pkt_with_fpi = NULL;
	//}
	obj->pkt_with_fpi = NULL;

	if (obj->recvd_symbols_tab && obj->src_symbols_tab)
	{
		for (esi = 0; esi < obj->n; esi++)
		{
			if (obj->recvd_symbols_tab[esi])
			{
				/* this is a symbol received from the network, without its FPI that starts 4 bytes before */
				//free((char*)obj->recvd_symbols_tab[esi]);
				//printf("fec_decode_close2: free recvd_symbols_tab \n");
			}
			else if (esi < obj->k && obj->src_symbols_tab[esi])
			{
				/* this is a source symbol decoded by the openfec codec, so free it */
				ASSERT(obj->recvd_symbols_tab[esi] == NULL);
				///free(obj->src_symbols_tab[esi]);
				//printf("fec_decode_close2: free src_symbols_tab \n");
			}
		}
		free(obj->recvd_symbols_tab);
		obj->recvd_symbols_tab = NULL;
		free(obj->src_symbols_tab);
		obj->src_symbols_tab = NULL;
	}
}


void fec_free(FecObj *obj)
{
    if(obj->recvd_symbols_tab)
    {
        for(int i = 0; i < obj->n; i++)
        {
            if(obj->recvd_symbols_tab[i])
            {
                free(obj->recvd_symbols_tab[i]);
            }
            else if (i < obj->k && obj->src_symbols_tab[i])
			{
			    free(obj->src_symbols_tab[i]);//added by_gxh_20210304
			}
        }
        free(obj->recvd_symbols_tab);
        obj->recvd_symbols_tab = NULL;
    }
    if(obj->src_symbols_tab)
    {
        free(obj->src_symbols_tab);
        obj->src_symbols_tab = NULL;
    }
}
int unpacket_fec(FecObj *obj, char *indata, short *inSize, char *outbuf, short *pktSize)
{
    int ret = 0;
    int i = 0;
    int offset = 0;
    int offset2 = 0;
    int last_seq_num = -1;
    int loss_num = 0;
    int marker = 0;
    int sps_flag = 0;
    int pps_flag = 0;
    int idr_flag = 0;
    int rptHeadSize = sizeof(RTP_FIXED_HEADER);
    int extlen0 = sizeof(EXTEND_HEADER);
    int headSize = sizeof(RTP_FIXED_HEADER) + sizeof(EXTEND_HEADER) + sizeof(FEC_HEADER);
    int k = -1;
    int n = -1;
    RTP_FIXED_HEADER  *rtp_hdr = NULL;
    EXTEND_HEADER *rtp_ext = NULL;
    FEC_HEADER *fec_hdr = NULL;
    //for(int i = 0; i < (obj->k + 1); i++)
    //{
    //    pktSize[i] = 0;
    //}
    i = 0;
    do{
        int size = inSize[i];
        char *p0 = (char *)&indata[offset];
        rtp_hdr = (RTP_FIXED_HEADER *)p0;
        rtp_ext = (EXTEND_HEADER *)&p0[rptHeadSize];

        int this_seqnum = rtp_hdr->seq_no;
        int refresh_idr = rtp_ext->refresh_idr;

        int rtp_extend_length = rtp_ext->rtp_extend_length;
		rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
		int	extlen = (rtp_extend_length + 1) << 2;
		unsigned int	rtp_pkt_size = rtp_ext->rtp_pkt_size;

        int nal_unit_type = rtp_ext->nal_type;

        fec_hdr = (FEC_HEADER *)&p0[rptHeadSize + extlen0];
        //printf("unpacket_fec: extlen0= %d \n", extlen0);
        //printf("unpacket_fec: extlen= %d \n", extlen);

        if(nal_unit_type == 5)
	    {
	        idr_flag += 1;
	    }
	    else if(nal_unit_type == 7)
	    {
	        sps_flag += 1;
	    }
	    else if(nal_unit_type == 8)
	    {
	        pps_flag += 1;
	    }
	    else if(nal_unit_type == 24)
	    {
	        sps_flag += 1;
	        pps_flag += 1;
	    }
	    if(rtp_hdr->marker)
        {
            marker += 1;
        }

        obj->codec_id = fec_hdr->codec_id;
        //k =
        obj->k = fec_hdr->k;
        obj->n = fec_hdr->n;
        if(k < 0)
        {
            k = obj->k;
            n = obj->n;
            //for(int l = 0; l < (k + 1); l++)
            for(int l = 0; l < (n + 1); l++)
            {
                pktSize[l] = 0;
            }
        }
        //obj->symbol_size = fec_hdr->symbol_size;
        obj->symb_sz_32 = fec_hdr->symbol_size;
	    obj->symbol_size = obj->symb_sz_32 << 2;
        int esi = fec_hdr->fec_seq_no;

        //printf("unpacket_fec: fec_hdr->n= %d \n", fec_hdr->n);
        //printf("unpacket_fec: fec_hdr->k= %d \n", fec_hdr->k);
        //printf("unpacket_fec: i= %d \n", i);
        //printf("unpacket_fec: extlen= %d \n", extlen);
        if(extlen != (headSize - rptHeadSize))
		{
		    printf("error: unpacket_fec: extlen= %d \n", extlen);
		    printf("error: unpacket_fec: headSize= %d \n", headSize);
		    printf("error: unpacket_fec: rptHeadSize= %d \n", rptHeadSize);
		    fec_free(obj);
		    ret = -2;
            return ret;
		}
		//if(rtp_pkt_size > (headSize + obj->symbol_size) ||
		//    rtp_pkt_size > size ||
		//    (headSize + obj->symbol_size) != size
		//    )
		if(rtp_pkt_size != size)
		{
		    printf("error: unpacket_fec: rtp_pkt_size= %d \n", rtp_pkt_size);
		    printf("error: unpacket_fec: headSize= %d \n", headSize);
		    printf("error: unpacket_fec: obj->symbol_size= %d \n", obj->symbol_size);
		    printf("error: unpacket_fec: size= %d \n", size);
		    printf("error: unpacket_fec: fec_hdr->symbol_size= %d \n", fec_hdr->symbol_size);
		    fec_free(obj);
		    ret = -2;
            return ret;
		}

        if(obj->recvd_symbols_tab == NULL)
        {
            obj->recvd_symbols_tab = (void**) calloc(obj->n, sizeof(void*));
            if(obj->recvd_symbols_tab == NULL)
            {
                fec_free(obj);
                ret = -1;
                return ret;
            }
        }
        if(obj->src_symbols_tab == NULL)
        {
            obj->src_symbols_tab = (void**) calloc(obj->k, sizeof(void*));
            if(obj->src_symbols_tab == NULL)
            {
                fec_free(obj);
                ret = -1;
                return ret;
            }
        }
        //printf("unpacket_fec: esi= %d \n", esi);
        //printf("unpacket_fec: obj->k= %d \n", obj->k);
        char *p1 = calloc(1, obj->symbol_size * sizeof(char));
        obj->recvd_symbols_tab[esi] = p1;
        memcpy(p1, (char*)&p0[headSize], inSize[i] - headSize);

        //printf("unpacket_fec: inSize[i]=%d, i=%d \n", inSize[i], i);
        if(esi < obj->k)
        {
            //printf("unpacket_fec: (esi < obj->k)= %d \n", (esi < obj->k));
            char *p1 = (char *)&outbuf[offset2];
            //short *raw_size = (short *)&p0[size - 2];
            //if(raw_size[0] > obj->symbol_size)
            //{
		    //    printf("error: unpacket_fec: raw_size[0]= %d \n", raw_size[0]);
		    //    fec_free(obj);
		    //    ret = -2;
            //    return ret;
		    //}
		    //int size2 = headSize + raw_size[0];
		    unsigned int size2 = rtp_pkt_size;//get_pkt_size(p0, (headSize ));
		    unsigned int rawSize = rtp_pkt_size - headSize;
		    int padSize = obj->symbol_size - rawSize;
		    if(rawSize != obj->symbol_size)
		    {
		        //printf("unpacket_fec: rawSize= %d \n", rawSize);
		    }
		    memcpy(p1, p0, size2);
		    //memset(&p1[size2], 0, padSize);
            pktSize[i] = size2;// + padSize;
            offset2 += size2;// + padSize;
            //printf("unpacket_fec: raw_size[0]= %d \n", raw_size[0]);
            //printf("unpacket_fec: offset2= %d \n", offset2);
        }

        if(last_seq_num >= 0 && i < obj->k)
        {
            int diff_seqnum = this_seqnum - last_seq_num;
            if(diff_seqnum < -HALF_USHORT)
            {
                diff_seqnum = MAX_USHORT + this_seqnum - last_seq_num;
            }
            loss_num += (diff_seqnum - 1);
        }

        offset += size;
        i++;

        //if((marker || (esi == (obj->k - 1))) && (i == obj->k) && !loss_num)
        if((esi == (obj->k - 1)) && (i == obj->k) && !loss_num)
        {
            //no fec decode
            //complete frame
            //printf("unpacket_fec: complete frame   \n");
            fec_free(obj);
            ret = obj->k;
            return ret;
        }
        //else if(esi == (obj->k - 1))
        //{
        //    if(!refresh_idr)
        //    {
        //    }
        //}

        if(i >= obj->k)
        {
            ret = obj->k;
            //break;
        }

    }while(inSize[i] > 0);

    if(ret <= 0)
    {
        if((sps_flag || pps_flag || idr_flag) && !idr_flag)
        {
            if((i + sps_flag + pps_flag) == k)
            {
                //no fec decode

            }
        }
        fec_free(obj);
    }
    ret = i;
    //pktSize[i] = 0;
    //printf("unpacket_fec: i= %d\n", i);
    for(int i = 0; i < (obj->k + 1); i++)
    {
        //printf("unpacket_fec: pktSize[i]= %d\n", pktSize[i]);
    }
    return ret;
}
int get_payload_size(char *data, int size)
{
    int ret = 0;
    int step = sizeof(uint64_t);
    uint64_t *p0 = (uint64_t *)data;
    uint64_t *p1 = (uint64_t *)&data[size - step];
    int count = 0;
    if(!p1[0])
    {
        while(p1 > p0)
        {
            if(p1[0])
            {
                break;
            }
            p1--;
            count++;
        }
    }
    else{
        //printf("get_payload_size: 0: no zero \n");
    }
    ret = (int64_t)p1 - (int64_t)data;
    //printf("get_payload_size: p1=%x \n", p1);
    //printf("get_payload_size: data=%x \n", data);
    //printf("get_payload_size: 0: count=%d \n", count);
    //printf("get_payload_size: 0: ret=%d \n", ret);

    uint8_t *p2 = (uint8_t *)p1;
    uint8_t *p3 = (uint8_t *)&p2[step - 1];
    //count = 0;
    if(!p3[0])
    {
        while(p3 > p2)
        {
            if(p3[0])
            {
                break;
            }
            p3--;
            //count++;
        }
    }
    else{
        //printf("get_payload_size: 1: no zero \n");
    }
    ret = (char *)p3 - data + 1;
    return ret;
}
int fec2packet(FecObj *obj, char *indata, short *inSize, char *outbuf, short *pktSize)
{
    int ret = 0;

	int i = 0, j = 0;
	int offset = 0, offset2 = 0;
	int total_size = 0, total_size2 = 0;
	int rptHeadSize = sizeof(RTP_FIXED_HEADER);
    int extlen0 = sizeof(EXTEND_HEADER);
	int headSize = sizeof(RTP_FIXED_HEADER) + sizeof(EXTEND_HEADER) + sizeof(FEC_HEADER);
	RTP_FIXED_HEADER  *rtp_hdr = NULL;
    EXTEND_HEADER *rtp_ext = NULL;
    FEC_HEADER *fec_hdr = NULL;
    char *rtp_head = indata;
    int base_esi = -1;
    int base_seqnum = -1;
    int *rawSize = calloc(1, obj->k * sizeof(int));
	for(i = 0; i < obj->k; i++)
	{
	    char *p0 = (char *)obj->src_symbols_tab[i];
	    //short *raw_size = (short *)&p0[obj->symbol_size - 2];
	    //printf("fec2packet: raw_size[0]=%d \n", raw_size[0]);
#ifndef TEST_RAW_DATA
        //printf("fec2packet: obj->symbol_size=%d \n", obj->symbol_size);
        //int payload_size = get_payload_size(p0, obj->symbol_size - 2);
        int payload_size = get_payload_size(p0, obj->symbol_size);
        rawSize[i] = payload_size;
        //printf("fec2packet: raw_size[0]=%d \n", raw_size[0]);
        //printf("fec2packet: payload_size=%d \n", payload_size);
#endif
	    //total_size += raw_size[0] + headSize;
	    total_size += payload_size + headSize;
	}
	while(pktSize[j] > 0){

	    {
	        char *p2 = (char *)&outbuf[total_size2];
	        rtp_hdr = (RTP_FIXED_HEADER *)p2;
	        int this_seqnum = rtp_hdr->seq_no;
            rtp_ext = (EXTEND_HEADER *)&p2[rptHeadSize];
            fec_hdr = (FEC_HEADER *)&p2[rptHeadSize + extlen0];//shoud be change
            int esi = fec_hdr->fec_seq_no;
            if(esi < obj->k && base_seqnum < 0)
            {
                base_seqnum = this_seqnum;
                base_esi = esi;
            }
#if 0
	        //test
            {
                int rtp_extend_length = rtp_ext->rtp_extend_length;
			    rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
			    int extlen = (rtp_extend_length + 1) << 2;
			    unsigned int rtp_pkt_size = rtp_ext->rtp_pkt_size;
			    if((rawSize[esi] + headSize) != rtp_pkt_size)
			    {
			        printf("error: fec2packet: rtp_pkt_size=%d \n", rtp_pkt_size);
			        printf("error: fec2packet: rawSize[esi]=%d \n", rawSize[esi]);
			        printf("error: fec2packet: esi=%d \n", esi);
			    }
			    //
                //printf("fec2packet: extlen=%d \n", extlen);
            }
#endif
            //printf("fec2packet: fec_hdr=%x \n", fec_hdr);
            //printf("fec2packet: fec_hdr->k=%d \n", fec_hdr->k);
            //printf("fec2packet: fec_hdr->n=%d \n", fec_hdr->n);
            //printf("fec2packet: esi=%d \n", esi);
            //printf("fec2packet: total_size2=%d \n", total_size2);
        }

	    total_size2 += pktSize[j];
	    //printf("fec2packet: pktSize[j]=%d\n", pktSize[j]);
	    j++;

	}
	//printf("fec2packet: j=%d\n", j);
	offset = total_size;
	offset2 = total_size2;
	j--;
	for(i = obj->k - 1; i >= 0; i--)
	{
	    char *p0 = (char *)obj->src_symbols_tab[i];
	    //short *raw_size = (short *)&p0[obj->symbol_size - 2];
	    //int rtp_pkt_size = raw_size[0] + headSize;
	    unsigned int rtp_pkt_size = rawSize[i] + headSize;
	    offset -= rtp_pkt_size;
	    char *p1 = (char *)&outbuf[offset];
	    char *p2 = NULL;
	    //printf("fec2packet: i=%d\n", i);
	    //printf("fec2packet: raw_size[0]=%d\n", raw_size[0]);
	    //printf("fec2packet: j=%d\n", j);
	    //printf("fec2packet: offset=%d\n", offset);
	    //printf("fec2packet: offset2=%d\n", offset2);
	    //printf("fec2packet: %d, rtp_pkt_size=%d\n", raw_size[0], rtp_pkt_size - sizeof(FEC_HEADER));
	    if(j >= 0)
	    {
	        offset2 -= pktSize[j];
	        p2 = (char *)&outbuf[offset2];
	        rtp_hdr = (RTP_FIXED_HEADER *)p2;
	        int this_seqnum = rtp_hdr->seq_no;
            rtp_ext = (EXTEND_HEADER *)&p2[rptHeadSize];
            fec_hdr = (FEC_HEADER *)&p2[rptHeadSize + extlen0];//shoud be change
            int esi = fec_hdr->fec_seq_no;
            //printf("fec2packet: fec_hdr=%x \n", fec_hdr);
            //printf("fec2packet: rtp_pkt_size=%d \n", rtp_pkt_size);
            //printf("fec2packet: pktSize[j]=%d \n", pktSize[j]);
            //printf("fec2packet: fec_hdr->k=%d \n", fec_hdr->k);
            //printf("fec2packet: fec_hdr->n=%d \n", fec_hdr->n);
            //printf("fec2packet: esi=%d \n", esi);
            if(esi == i)
            {
                if(p1 != p2)
                {
                    //memcpy(p1, p2, pktSize[j]);
                    memmove(p1, p2, pktSize[j]);
                    pktSize[i] = pktSize[j];
                }

                j--;
            }
            else{
                //miss
                offset2 += pktSize[j];
                //printf("fec2packet: miss: i= %d \n", i);
                memcpy(&p1[0], rtp_head, headSize);
                //memcpy(&p1[headSize], p0, raw_size[0]);
                memcpy(&p1[headSize], p0, rawSize[i]);
                pktSize[i] = rtp_pkt_size;
                rtp_hdr = (RTP_FIXED_HEADER *)p1;
                int this_seqnum = -1;
                this_seqnum = base_seqnum + (i - base_esi);
                //if(i > base_esi)
                //{
                //    this_seqnum = base_seqnum + (i - base_esi);
                //}
                //else{
                //    this_seqnum = base_seqnum - (base_esi - i);
                //}
                if (this_seqnum >= MAX_USHORT)
			    {
				    this_seqnum = this_seqnum - MAX_USHORT;
			    }
			    if(this_seqnum < 0)
			    {
			        printf("error: fec2packet: miss: this_seqnum= %d \n", this_seqnum);
			    }

                rtp_hdr->seq_no = this_seqnum;
                //printf("fec2packet: miss: this_seqnum= %d \n", this_seqnum);
                //printf("fec2packet: miss: base_esi= %d \n", base_esi);
                //printf("fec2packet: miss: base_seqnum= %d \n", base_seqnum);
                rtp_ext = (EXTEND_HEADER *)&p1[rptHeadSize];
                rtp_ext->rtp_pkt_size = rtp_pkt_size;
                fec_hdr = (FEC_HEADER *)&p1[rptHeadSize + extlen0];
                fec_hdr->fec_seq_no = i;
            }
	    }
	    else{
	        //printf("fec2packet: miss: i= %d \n", i);
            memcpy(&p1[0], rtp_head, headSize);
            //memcpy(&p1[headSize], p0, raw_size[0]);
            memcpy(&p1[headSize], p0, rawSize[i]);
            pktSize[i] = rtp_pkt_size;
            rtp_hdr = (RTP_FIXED_HEADER *)p1;
            int this_seqnum = -1;
            this_seqnum = base_seqnum + (i - base_esi);
            //if(i > base_esi)
            //{
            //    this_seqnum = base_seqnum + (i - base_esi);
            //}
            //else{
            //    this_seqnum = base_seqnum - (base_esi - i);
            //}
            if (this_seqnum >= MAX_USHORT)
			{
			    this_seqnum = this_seqnum - MAX_USHORT;
			}
			if(this_seqnum < 0)
			{
			    printf("error: fec2packet: miss: this_seqnum= %d \n", this_seqnum);
			}
            rtp_hdr->seq_no = this_seqnum;
            //printf("fec2packet: miss: this_seqnum= %d \n", this_seqnum);
            //printf("fec2packet: miss: base_esi= %d \n", base_esi);
            //printf("fec2packet: miss: base_seqnum= %d \n", base_seqnum);
            rtp_ext = (EXTEND_HEADER *)&p1[rptHeadSize];
            rtp_ext->rtp_pkt_size = rtp_pkt_size;
            fec_hdr = (FEC_HEADER *)&p1[rptHeadSize + extlen0];
            fec_hdr->fec_seq_no = i;
	    }

    }
    ret = obj->k;
    pktSize[ret] = 0;
    free(rawSize);
	return ret;
}
int fec_decode2(FecObj *obj, char *indata, short *inSize, char *outbuf, short *pktSize)
{
    int ret = 0;
    UINT32		esi;					/* Encoding Symbol ID, used to identify each encoding symbol */
    INT32		len;
    UINT32		n_received	= 0;
    int idx = 0;
    int offset = 0;
    bool		done		= false;		/* true as soon as all source symbols have been received or recovered */
    int packet_num = 0;
    int n2 = 0;

    ret = unpacket_fec(obj, indata, inSize, outbuf, pktSize);
    //printf("fec_decode2: ret= %d \n", ret);
    //printf("fec_decode: obj->k= %d \n", obj->k);
    if(ret <= 0)
    {
        printf("gxh: fec_decode2: ret= %d \n", ret);
        printf("gxh: fec_decode2: obj->k= %d \n", obj->k);
	    printf("gxh: fec_decode2: obj->n= %d \n", obj->n);
    }
    //if(ret < 0 || ret != obj->k || (obj->recvd_symbols_tab == NULL))
    if(ret < 0 || ret < obj->k || (obj->recvd_symbols_tab == NULL))
    {
        return ret;
    }
    n2 = ret;
	//printf("gxh: fec_decode2: obj->k= %d \n", obj->k);
	//printf("gxh: fec_decode2: obj->n= %d \n", obj->n);


	/* and check the correctness of data received */
	if (obj->k > obj->n || obj->k > 40000 || obj->n > 40000)
	{
		OF_PRINT_ERROR(("Invalid FEC OTI received: k=%u or n=%u received are probably out of range\n", obj->k, obj->n))
		ret = -1;
		goto end;
	}
	if(obj->n <= 255)
	{
	    obj->codec_id = OF_CODEC_REED_SOLOMON_GF_2_M_STABLE;
	}
	else{
	    obj->codec_id = OF_CODEC_LDPC_STAIRCASE_STABLE;
	}
	//printf("gxh: fec_decode2: obj->codec_id= %d \n", obj->codec_id);

	/* now we know which codec the sender has used along with the codec parameters, we can prepar the params structure accordingly */
	switch (obj->codec_id) {
	case OF_CODEC_REED_SOLOMON_GF_2_M_STABLE: {
		/* fill in the code specific part of the of_..._parameters_t structure */
		of_rs_2_m_parameters_t	*my_params;

		//printf("\nInitialize a Reed-Solomon over GF(2^m) codec instance, (n, k)=(%u, %u)...\n", obj->n, obj->k);
		if ((my_params = (of_rs_2_m_parameters_t *)calloc(1, sizeof(* my_params))) == NULL)
		{
			OF_PRINT_ERROR(("no memory for codec %d\n", obj->codec_id))
			ret = -1;
			goto end;
		}
		my_params->m = 8;
		obj->params = (of_parameters_t *) my_params;
		break;
		}

	case OF_CODEC_LDPC_STAIRCASE_STABLE: {
		/* fill in the code specific part of the of_..._parameters_t structure */
		of_ldpc_parameters_t	*my_params;

		//printf("\nInitialize an LDPC-Staircase codec instance, (n, k)=(%u, %u)...\n", obj->n, obj->k);
		if ((my_params = (of_ldpc_parameters_t *)calloc(1, sizeof(* my_params))) == NULL)
		{
			OF_PRINT_ERROR(("no memory for codec %d\n", obj->codec_id))
			ret = -1;
			goto end;
		}
		my_params->prng_seed	= 146348057;//rand();
		my_params->N1		= 7;
		obj->params = (of_parameters_t *) my_params;
		break;
		}

	default:
		OF_PRINT_ERROR(("Invalid FEC OTI received: codec_id=%u received is not valid\n", obj->codec_id))
		ret = -1;
		goto end;
	}
	obj->params->nb_source_symbols	= obj->k;		/* fill in the generic part of the of_parameters_t structure */
	obj->params->nb_repair_symbols	= obj->n - obj->k;
	obj->params->encoding_symbol_length	= obj->symbol_size;

	/* Open and initialize the openfec decoding session now that we know the various parameters used by the sender/encoder... */
	if ((ret = of_create_codec_instance(&obj->ses, obj->codec_id, OF_DECODER, VERBOSITY)) != OF_STATUS_OK)
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

	//printf( "\nDecoding in progress. Waiting for new packets...\n" );

#ifdef USE_DECODE_WITH_NEW_SYMBOL
	/*
	 * this is the standard method: submit each fresh symbol to the library ASAP, upon reception
	 * (or later, but using the standard of_decode_with_new_symbol() function).
	 */
	 //printf("USE_DECODE_WITH_NEW_SYMBOL \n");
	///while ((ret = get_next_pkt(so, &pkt_with_fpi, &len)) == OF_STATUS_OK)
	do
	{
		/* OK, new packet received... */

		esi = idx;//ntohl(*(UINT32*)obj->pkt_with_fpi);
        //printf("fec_decode: idx= %d \n", idx);
		if (esi > obj->n)		/* a sanity check, in case... */
		{
			OF_PRINT_ERROR(("invalid esi=%u received in a packet's FPI\n", esi))
			ret = -1;
			goto end;
		}
		if(obj->recvd_symbols_tab[esi])
		{
		    n_received++;
#ifdef TEST
            char *p = obj->recvd_symbols_tab[esi];
            for(int j = 0; j < 16; j++)
            {
                unsigned char value = p[j];
                printf("%2x",value);
            }
            printf("\n");
#endif
		    if (of_decode_with_new_symbol(obj->ses, (char*)obj->recvd_symbols_tab[esi], esi) == OF_STATUS_ERROR) {
			    OF_PRINT_ERROR(("of_decode_with_new_symbol() failed\n"))
			    ret = -1;
			    goto end;
		    }
		}
		//printf("recvd_symbols_tab= ");
		//dump_buffer_32(obj->recvd_symbols_tab[esi], 1);//test

		/* check if completed in case we received k packets or more */
		if ((n_received >= obj->k) && (of_is_decoding_complete(obj->ses) == true)) {
			/* done, we recovered everything, no need to continue reception */
			done = true;
			break;
		}
		idx += 1;
	//}while(n_received < obj->k);
	}while(n_received < n2);
	//printf("gxh: fec_decode2: n_received= %d \n", n_received);
	//printf("gxh: fec_decode2: done= %d \n", done);
	ret = OF_STATUS_FAILURE;//test
#else
	/*
	 * this is the alternative method: wait to receive all the symbols, then submit them all to
	 * the library using the of_set_available_symbols() function. In that case decoding will occur
	 * during the of_finish_decoding() call.
	 */
	printf("wait to receive all the symbols \n");

	n_received = obj->k;
	/* now we received everything, submit them all to the codec if we received a sufficiently high number of symbols (i.e. >= k) */
	if (n_received >= obj->k && (of_set_available_symbols(obj->ses, obj->recvd_symbols_tab) != OF_STATUS_OK))
	{
		OF_PRINT_ERROR(("of_set_available_symbols() failed with error (%d)\n", ret))
		ret = -1;
		goto end;
	}
#endif
	if (!done && (ret == OF_STATUS_FAILURE) && (n_received >= obj->k))
	{
		/* there's no packet any more but we received at least k, and the use of of_decode_with_new_symbol() didn't succedd to decode,
		 * so try with of_finish_decoding.
		 * NB: this is useless with MDS codes (e.g. Reed-Solomon), but it is essential with LDPC-Staircase as of_decode_with_new_symbol
		 * performs ITerative decoding, whereas of_finish_decoding performs ML decoding */
		ret = of_finish_decoding(obj->ses);
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
		printf("fec_decode_close2: ret= %d \n", ret);
	}
	if (done)
	{
		/* finally, get a copy of the pointers to all the source symbols, those received (that we already know) and those decoded.
		 * In case of received symbols, the library does not change the pointers (same value). */
		if (of_get_source_symbols_tab(obj->ses, obj->src_symbols_tab) != OF_STATUS_OK)
		{
			OF_PRINT_ERROR(("of_get_source_symbols_tab() failed\n"))
			ret = -1;
			goto end;
		}
		//printf("\nDone! All source symbols rebuilt after receiving %u packets\n", n_received);
		if (VERBOSITY > 1)
		{
			for (esi = 0; esi < obj->k; esi++) {
				///printf("src[%u]= ", esi);
				char *p = obj->src_symbols_tab[esi];
				//dump_buffer_32(src_symbols_tab[esi], 1);
				///dump_buffer_32(&p[0], 4);
			}
		}
		//printf("fec2packet: start \n");
		ret = obj->k;//test
		ret = fec2packet(obj, indata, inSize, outbuf, pktSize);
		//printf("fec2packet: ret= %d \n", ret);

#ifdef TEST
        int offset2 = 0;
        for(int i = 0; i < obj->k; i++)
		{
		    for(int j = 0; j < pktSize[i]; j++)
		    {
		        unsigned char *p0 = (unsigned char *)obj->src_symbols_tab[i];
		        int value0 = p0[j];
		        int value1 = test_data[offset2];
		        if(value0 != value1)
		        {
		            printf("error: fec_decode: i= %d, j= %d \n", i, j);
		            printf("error: fec_decode: pktSize[i]= %d \n", pktSize[i]);
		        }
		        offset2 += 1;
		    }
        }
#endif
	}
	else
	{
	    printf("gxh: fec_decode2: obj->codec_id= %d \n", obj->codec_id);
        printf("gxh: fec_decode2: obj->k= %d \n", obj->k);
	    printf("gxh: fec_decode2: obj->n= %d \n", obj->n);
	    printf("gxh: fec_decode2: n_received= %d \n", n_received);
	    printf("gxh: fec_decode2: done= %d \n", done);
		printf("\ngxh: error: Failed to recover all erased source symbols even after receiving %u packets\n", n_received);
	}
end:
    //printf("fec_decode_close2: ret= %d \n", ret);
    fec_decode_close2(obj);
    return ret;
}
HCSVC_API
int api_fec_decode(char *handle, char *data, char *param, char *outbuf, char *outparam[])
{
    int ret = 0;
    //init_fec_obj(id);
    //if (id < MAX_OBJ_NUM)
    {
        long long *testp = (long long *)handle;
        CodecObj *codecObj = (CodecObj *)testp[0];
        FecObj *obj = (FecObj *)codecObj->fecDecObj;//&global_fec_objs[id];
        int id = codecObj->Obj_id;
        if (!obj)
        {
            obj = (FecObj *)calloc(1, sizeof(FecObj));
            codecObj->fecDecObj = obj;
            fec_init(obj);
            obj->Obj_id = id;
        }
        obj->json = (cJSON *)api_str2json(param);
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
        //printf("api_fec_decode: symbol_size= %d \n", symbol_size);
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
            //printf("rtp_packet2raw: nal_mem_num= %d \n", nal_mem_num);
            obj->inSize = calloc(1, sizeof(short) * (pkt_mem_num + 1));

            for( int i = 0 ; i < pkt_mem_num ; i ++ ){
                cJSON * pSub = cJSON_GetArrayItem(cjsonArr, i);
                if(NULL == pSub ){ continue ; }
                //char * ivalue = pSub->valuestring ;
                int ivalue = pSub->valueint;
                obj->inSize[i] = (short)ivalue;//可以不用傳入，通過擴展字段讀入：rtpSize[idx] = rtp_pkt_size;
                pktLen += ivalue;
            }
            obj->inSize[i] = 0;
        }
        else{
            return ret;
        }
        //printf("api_fec_decode: pkt_mem_num= %d \n", pkt_mem_num);
        short pktSize[MAX_FEC_PKT_NUM];
        //ret = fec_decode(obj, data, obj->inSize, outbuf, pktSize);
        ret = fec_decode2(obj, data, obj->inSize, outbuf, pktSize);
        //printf("api_fec_decode: ret= %d \n", ret);
        outparam[0] = obj->outparam[0];
        outparam[1] = obj->outparam[1];
        if(ret > 0)
        {
            //char text[2048] = "";//2048/4=512//512*1400=700kB

	        int sum = 0;

            for (int i = 0; i < ret; i++)
	        {
	            int size = pktSize[i];
                sum += size;
	        }

	        cJSON *json2 = NULL;
            json2 = renewJsonArray1(json2, "rtpSize", pktSize, ret);
            char *jsonStr = api_json2str(json2);//比较耗时
            api_json_free(json2);
            strcpy(obj->outparam[0], jsonStr);
            api_json2str_free(jsonStr);
            ret = sum;
            //outparam[2] = "complete";
            //printf("api_fec_decode: sum= %d \n", sum);
        }
        if(obj->inSize)
        {
            free(obj->inSize);
            obj->inSize = NULL;
        }
        api_json_free(obj->json);
        obj->json = NULL;
    }
    return ret;
}
#ifdef linux
HCSVC_API
int api_fec_client_main () //int argc, char* argv[]
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
		my_params->prng_seed	= 146348057;//rand();
		printf("gxh: api_fec_client_main: my_params->prng_seed= %d \n", my_params->prng_seed);
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
		//printf("%05d => receiving symbol esi=%u (%s)\n", n_received, esi, (esi < k) ? "src" : "repair");
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
		if(n_received >= k)
		{
		    //ret = OF_STATUS_FAILURE;
		    //break;//added by gxh
		}
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
    printf("gxh: api_fec_client_main: n_received= %d \n", n_received);
	printf("gxh: api_fec_client_main: done= %d \n", done);
	printf("gxh: api_fec_client_main: ret= %d \n", ret);
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
		printf("gxh: api_fec_client_main: done= %d \n", done);
		/* else ret == OF_STATUS_FAILURE, meaning of_finish_decoding didn't manage to recover all source symbols */
	}
	if (done)
	{
	    //if (VERBOSITY > 1)
		//{
		//	for (esi = 0; esi < k; esi++) {
		//		printf("src[%u]= ", esi);
		//		dump_buffer_32(recvd_symbols_tab[esi], 1);
		//	}
		//}
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
				printf("src[%03x]= ", (esi + 1));
				char *p = src_symbols_tab[esi];
				//dump_buffer_32(src_symbols_tab[esi], 1);
				dump_buffer_32(&p[0], 4);
			}
		}
	}
	else
	{
	    ret = -2;
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
	printf("gxh: api_fec_client_main: end \n");
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
		//if (VERBOSITY > 1)
		//	printf("%s: pkt received 0, len=%u\n", __FUNCTION__, *len);
		return OF_STATUS_OK;
	}
	/* otherwise we are in non-blocking mode... */
	*len = recvfrom(so, *pkt, saved_len, 0, NULL, NULL);
	if (*len > 0)
	{
		//if (VERBOSITY > 1)
		//	printf("%s: pkt received 1, len=%u\n", __FUNCTION__, *len);
		return OF_STATUS_OK;
	}
	else if (errno == EAGAIN || errno == EWOULDBLOCK)
	{
		/* no packet available, sleep a little bit and retry */
		SLEEP(200);	/* (in milliseconds) */
		*len = recvfrom(so, *pkt, saved_len, 0, NULL, NULL);
		if (*len > 0)
		{
			//if (VERBOSITY > 1)
			//	printf("%s: pkt received 2, len=%u\n", __FUNCTION__, *len);
			return OF_STATUS_OK;
		}
		else
		{
			/* that's the end of the test, no packet available any more, we're sure of that now... */
			//if (VERBOSITY > 1)
			//	printf("%s: end of test, no packet after the sleep\n", __FUNCTION__);
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
#endif

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

