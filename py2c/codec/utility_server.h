/*****************************************************************************
 * utility_server.h:
 *****************************************************************************
 * Copyright (C) 2020-2020 hcsc project
 *
 * Authors: Xiaohui Gao <waky_7635@126.com>
  *****************************************************************************/

#ifndef UTILITY_SERVER_H
#define UTILITY_SERVER_H

#include "hcsvc.h"

typedef struct
{
    int status;
    int64_t seqnum;
    int64_t timestamp;
}RtpPacketInfo;
typedef struct {
    int loss_rate;
    int max_loss_rate;
    int last_max_loss_rate;
    int sum_loss_rate;
    int cnt_loss_rate;
    int cnt_max;
    //
    int64_t now_time;
    int time_len;
}LossRateInfo;
typedef struct
{
    FILE *logfp;
    int layerId;
    unsigned int ssrc;
    int64_t time_stamp;
    int64_t min_seqnum;
    int64_t max_seqnum;
    int64_t old_seqnum;
    int status;
    int max_loss_series_num;
    int max_fec_n;
    LossRateInfo loss_rate_info[NETN];
    RtpPacketInfo info[MAX_PKT_BUF_SIZE];
}PacketManager;

#endif
