/* $Id: rtcp.c 2422 2009-01-20 07:39:03Z bennylp $ */
/*
 * Copyright (C) 2008-2009 Teluu Inc. (http://www.teluu.com)
 * Copyright (C) 2003-2008 Benny Prijono <benny@prijono.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <pjmedia/rtcp.h>
#include <pjmedia/errno.h>
#include <pj/assert.h>
#include <pj/log.h>
#include <pj/os.h>
#include <pj/sock.h>
#include <pj/string.h>

#define THIS_FILE "rtcp.c"
#define DEBUG_TAG   THIS_FILE


#define RTCP_SR   200
#define RTCP_RR   201
#define RTCP_XR   207

#if PJ_HAS_HIGH_RES_TIMER==0
#   error "High resolution timer needs to be enabled"
#endif



#if 0
#   define TRACE_(x)	PJ_LOG(3,x)
#else
#   define TRACE_(x)
#endif


/*
 * Get NTP time.
 */
PJ_DEF(pj_status_t) pjmedia_rtcp_get_ntp_time(const pjmedia_rtcp_session *sess,
      pjmedia_rtcp_ntp_rec *ntp)
{
   /* Seconds between 1900-01-01 to 1970-01-01 */
#define JAN_1970  (2208988800UL)
   pj_timestamp ts;
   pj_status_t status;

   status = pj_get_timestamp(&ts);

   /* Fill up the high 32bit part */
   ntp->hi = (pj_uint32_t)((ts.u64 - sess->ts_base.u64) / sess->ts_freq.u64)
             + sess->tv_base.sec + JAN_1970;

   /* Calculate seconds fractions */
   ts.u64 = (ts.u64 - sess->ts_base.u64) % sess->ts_freq.u64;
   //pj_assert(ts.u64 < sess->ts_freq.u64);
   ts.u64 = (ts.u64 << 32) / sess->ts_freq.u64;

   /* Fill up the low 32bit part */
   ntp->lo = ts.u32.lo;

#if (defined(PJ_WIN32) && PJ_WIN32!=0) || \
    (defined(PJ_WIN32_WINCE) && PJ_WIN32_WINCE!=0)

   /* On Win32, since we use QueryPerformanceCounter() as the backend
    * timestamp API, we need to protect against this bug:
    *   Performance counter value may unexpectedly leap forward
    *   http://support.microsoft.com/default.aspx?scid=KB;EN-US;Q274323
    */
   {
      /*
       * Compare elapsed time reported by timestamp with actual elapsed
       * time. If the difference is too excessive, then we use system
       * time instead.
       */

      /* MIN_DIFF needs to be large enough so that "normal" diff caused
       * by system activity or context switch doesn't trigger the time
       * correction.
       */
      enum { MIN_DIFF = 400 };

      pj_time_val ts_time, elapsed, diff;

      pj_gettimeofday(&elapsed);

      ts_time.sec = ntp->hi - sess->tv_base.sec - JAN_1970;
      ts_time.msec = (long)(ntp->lo * 1000.0 / 0xFFFFFFFF);

      PJ_TIME_VAL_SUB(elapsed, sess->tv_base);

      if (PJ_TIME_VAL_LT(ts_time, elapsed)) {
         diff = elapsed;
         PJ_TIME_VAL_SUB(diff, ts_time);
      }
      else {
         diff = ts_time;
         PJ_TIME_VAL_SUB(diff, elapsed);
      }

      if (PJ_TIME_VAL_MSEC(diff) >= MIN_DIFF) {

         TRACE_((sess->name, "RTCP NTP timestamp corrected by %d ms",
                 PJ_TIME_VAL_MSEC(diff)));


         ntp->hi = elapsed.sec + sess->tv_base.sec + JAN_1970;
         ntp->lo = (elapsed.msec * 65536 / 1000) << 16;
      }

   }
#endif

   return status;
}


PJ_DEF(void) pjmedia_rtcp_init(pjmedia_rtcp_session *sess,
                               char *name,
                               unsigned clock_rate,
                               unsigned samples_per_frame,
                               pj_uint32_t ssrc)
{
   pjmedia_rtcp_sr_pkt *sr_pkt = &sess->rtcp_sr_pkt;
   pj_time_val now;

   /* Memset everything */
   pj_bzero(sess, sizeof(pjmedia_rtcp_session));

   /* Last RX timestamp in RTP packet */
   sess->rtp_last_ts = (unsigned)-1;

   /* Name */
   sess->name = name ? name : (char*)THIS_FILE,

                /* Set clock rate */
                sess->clock_rate = clock_rate;
   sess->pkt_size = samples_per_frame;

   /* Init common RTCP SR header */
   sr_pkt->common.version = 2;
   sr_pkt->common.count = 1;
   sr_pkt->common.pt = RTCP_SR;
   sr_pkt->common.length = pj_htons(12);
   sr_pkt->common.ssrc = pj_htonl(ssrc);

   /* Copy to RTCP RR header */
   pj_memcpy(&sess->rtcp_rr_pkt.common, &sr_pkt->common,
             sizeof(pjmedia_rtcp_common));
   sess->rtcp_rr_pkt.common.pt = RTCP_RR;
   sess->rtcp_rr_pkt.common.length = pj_htons(7);

   /* Get time and timestamp base and frequency */
   pj_gettimeofday(&now);
   sess->tv_base = now;
   sess->stat.start = now;
   pj_get_timestamp(&sess->ts_base);
   pj_get_timestamp_freq(&sess->ts_freq);

   /* Initialize statistics states */
   pj_math_stat_init(&sess->stat.rtt);
   pj_math_stat_init(&sess->stat.rx.loss_period);
   pj_math_stat_init(&sess->stat.rx.jitter);
   pj_math_stat_init(&sess->stat.tx.loss_period);
   pj_math_stat_init(&sess->stat.tx.jitter);

   /* RR will be initialized on receipt of the first RTP packet. */
}


PJ_DEF(void) pjmedia_rtcp_fini(pjmedia_rtcp_session *sess)
{
#if defined(PJMEDIA_HAS_RTCP_XR) && (PJMEDIA_HAS_RTCP_XR != 0)
   pjmedia_rtcp_xr_fini(&sess->xr_session);
#else
   /* Nothing to do. */
   PJ_UNUSED_ARG(sess);
#endif
}

static void rtcp_init_seq(pjmedia_rtcp_session *sess)
{
   sess->received = 0;
   sess->exp_prior = 0;
   sess->rx_prior = 0;
   sess->transit = 0;
   sess->jitter = 0;
}

PJ_DEF(void) pjmedia_rtcp_rx_rtp( pjmedia_rtcp_session *sess,
                                  unsigned seq,
                                  unsigned rtp_ts,
                                  unsigned payload)
{
   pjmedia_rtcp_rx_rtp2(sess, seq, rtp_ts, payload, PJ_FALSE);
}

PJ_DEF(void) pjmedia_rtcp_rx_rtp2(pjmedia_rtcp_session *sess,
                                  unsigned seq,
                                  unsigned rtp_ts,
                                  unsigned payload,
                                  pj_bool_t discarded)
{
   pj_timestamp ts;
   pj_uint32_t arrival;
   pj_int32_t transit;
   pjmedia_rtp_status seq_st;
   //unsigned last_seq;

#if !defined(PJMEDIA_HAS_RTCP_XR) || (PJMEDIA_HAS_RTCP_XR == 0)
   PJ_UNUSED_ARG(discarded);
#endif

   if (sess->stat.rx.pkt == 0) {
      /* Init sequence for the first time. */
      pjmedia_rtp_seq_init(&sess->seq_ctrl, (pj_uint16_t)seq);
   }

   sess->stat.rx.pkt++;
   sess->stat.rx.bytes += payload;

   /* Process the RTP packet. */
   //last_seq = sess->seq_ctrl.max_seq;
   pjmedia_rtp_seq_update(&sess->seq_ctrl, (pj_uint16_t)seq, &seq_st);

   if (seq_st.status.flag.restart) {
      rtcp_init_seq(sess);
   }

   if (seq_st.status.flag.dup) {
      sess->stat.rx.dup++;
      TRACE_((sess->name, "Duplicate packet detected"));
   }

   if (seq_st.status.flag.outorder && !seq_st.status.flag.probation) {
      sess->stat.rx.reorder++;
      TRACE_((sess->name, "Out-of-order packet detected"));
   }

   if (seq_st.status.flag.bad) {
      sess->stat.rx.discard++;

#if defined(PJMEDIA_HAS_RTCP_XR) && (PJMEDIA_HAS_RTCP_XR != 0)
      pjmedia_rtcp_xr_rx_rtp(&sess->xr_session, seq,
                             -1,				 /* lost    */
                             (seq_st.status.flag.dup? 1:0),	 /* dup     */
                             (!seq_st.status.flag.dup? 1:-1),  /* discard */
                             -1,				 /* jitter  */
                             -1, 0);				 /* toh	    */
#endif

      TRACE_((sess->name, "Bad packet discarded"));
      return;
   }

   /* Only mark "good" packets */
   ++sess->received;

   /* Calculate loss periods. */
   if (seq_st.diff > 1) {
      unsigned count = seq_st.diff - 1;
      unsigned period;

      period = count * sess->pkt_size * 1000 / sess->clock_rate;
      period *= 1000;

      /* Update packet lost.
       * The packet lost number will also be updated when we're sending
       * outbound RTCP RR.
       */
      sess->stat.rx.loss += (seq_st.diff - 1);
      TRACE_((sess->name, "%d packet(s) lost", seq_st.diff - 1));

      /* Update loss period stat */
      pj_math_stat_update(&sess->stat.rx.loss_period, period);
   }


   /*
    * Calculate jitter only when sequence is good (see RFC 3550 section A.8),
    * AND only when the timestamp is different than the last packet
    * (see RTP FAQ).
    */
   if (seq_st.diff == 1 && rtp_ts != sess->rtp_last_ts) {
      /* Get arrival time and convert timestamp to samples */
      pj_get_timestamp(&ts);
      ts.u64 = ts.u64 * sess->clock_rate / sess->ts_freq.u64;
      arrival = ts.u32.lo;

      transit = arrival - rtp_ts;

      /* Ignore the first N packets as they normally have bad jitter
       * due to other threads working to establish the call
       */
      if (sess->transit == 0 ||
            sess->received < PJMEDIA_RTCP_IGNORE_FIRST_PACKETS)
      {
         sess->transit = transit;
         sess->stat.rx.jitter.min = (unsigned)-1;
      }
      else {
         pj_int32_t d;
         pj_uint32_t jitter;

         d = transit - sess->transit;
         sess->transit = transit;
         if (d < 0)
            d = -d;

         sess->jitter += d - ((sess->jitter + 8) >> 4);

         /* Get jitter in usec */
         if (d < 4294)
            jitter = d * 1000000 / sess->clock_rate;
         else {
            jitter = d * 1000 / sess->clock_rate;
            jitter *= 1000;
         }

         /* Update jitter stat */
         pj_math_stat_update(&sess->stat.rx.jitter, jitter);

#if defined(PJMEDIA_HAS_RTCP_XR) && (PJMEDIA_HAS_RTCP_XR != 0)
         pjmedia_rtcp_xr_rx_rtp(&sess->xr_session, seq,
                                0,			    /* lost    */
                                0,			    /* dup     */
                                discarded,		    /* discard */
                                (sess->jitter >> 4),	    /* jitter  */
                                -1, 0);		    /* toh     */
#endif
      }
#if defined(PJMEDIA_HAS_RTCP_XR) && (PJMEDIA_HAS_RTCP_XR != 0)
   }
   else if (seq_st.diff > 1) {
      int i;

      /* Report RTCP XR about packet losses */
      for (i=seq_st.diff-1; i>0; --i) {
         pjmedia_rtcp_xr_rx_rtp(&sess->xr_session, seq - i,
                                1,			    /* lost    */
                                0,			    /* dup     */
                                0,			    /* discard */
                                -1,			    /* jitter  */
                                -1, 0);		    /* toh     */
      }

      /* Report RTCP XR this packet */
      pjmedia_rtcp_xr_rx_rtp(&sess->xr_session, seq,
                             0,			    /* lost    */
                             0,			    /* dup     */
                             discarded,		    /* discard */
                             -1,			    /* jitter  */
                             -1, 0);			    /* toh     */
#endif
   }

   /* Update timestamp of last RX RTP packet */
   sess->rtp_last_ts = rtp_ts;
}

PJ_DEF(void) pjmedia_rtcp_tx_rtp(pjmedia_rtcp_session *sess,
                                 unsigned bytes_payload_size)
{
   /* Update statistics */
   sess->stat.tx.pkt++;
   sess->stat.tx.bytes += bytes_payload_size;
}


PJ_DEF(void) pjmedia_rtcp_rx_rtcp( pjmedia_rtcp_session *sess,
                                   const void *pkt,
                                   pj_size_t size)
{
   pjmedia_rtcp_common *common = (pjmedia_rtcp_common*) pkt;
   const pjmedia_rtcp_rr *rr = NULL;
   const pjmedia_rtcp_sr *sr = NULL;
   pj_uint32_t last_loss, jitter_samp, jitter;

   /* Parse RTCP */
   if (common->pt == RTCP_SR) {
      sr = (pjmedia_rtcp_sr*) (((char*)pkt) + sizeof(pjmedia_rtcp_common));
      if (common->count > 0 && size >= (sizeof(pjmedia_rtcp_sr_pkt))) {
         rr = (pjmedia_rtcp_rr*)(((char*)pkt) + (sizeof(pjmedia_rtcp_common)
                                                 + sizeof(pjmedia_rtcp_sr)));
      }
   }
   else if (common->pt == RTCP_RR && common->count > 0) {
      rr = (pjmedia_rtcp_rr*)(((char*)pkt) + sizeof(pjmedia_rtcp_common));
#if defined(PJMEDIA_HAS_RTCP_XR) && (PJMEDIA_HAS_RTCP_XR != 0)
   }
   else if (common->pt == RTCP_XR) {
      if (sess->xr_enabled)
         pjmedia_rtcp_xr_rx_rtcp_xr(&sess->xr_session, pkt, size);

      return;
#endif
   }


   if (sr) {
      /* Save LSR from NTP timestamp of RTCP packet */
      sess->rx_lsr = ((pj_ntohl(sr->ntp_sec) & 0x0000FFFF) << 16) |
                     ((pj_ntohl(sr->ntp_frac) >> 16) & 0xFFFF);

      /* Calculate SR arrival time for DLSR */
      pj_get_timestamp(&sess->rx_lsr_time);

      TRACE_((sess->name, "Rx RTCP SR: ntp_ts=%p",
              sess->rx_lsr,
              (pj_uint32_t)(sess->rx_lsr_time.u64*65536/sess->ts_freq.u64)));
   }


   /* Nothing more to do if there's no RR packet */
   if (rr == NULL)
      return;


   last_loss = sess->stat.tx.loss;

   /* Get packet loss */
   sess->stat.tx.loss = (rr->total_lost_2 << 16) +
                        (rr->total_lost_1 << 8) +
                        rr->total_lost_0;

   TRACE_((sess->name, "Rx RTCP RR: total_lost_2=%x, 1=%x, 0=%x, lost=%d",
           (int)rr->total_lost_2,
           (int)rr->total_lost_1,
           (int)rr->total_lost_0,
           sess->stat.tx.loss));

   /* We can't calculate the exact loss period for TX, so just give the
    * best estimation.
    */
   if (sess->stat.tx.loss > last_loss) {
      unsigned period;

      /* Loss period in msec */
      period = (sess->stat.tx.loss - last_loss) * sess->pkt_size *
               1000 / sess->clock_rate;

      /* Loss period in usec */
      period *= 1000;

      /* Update loss period stat */
      pj_math_stat_update(&sess->stat.tx.loss_period, period);
   }

   /* Get jitter value in usec */
   jitter_samp = pj_ntohl(rr->jitter);
   /* Calculate jitter in usec, avoiding overflows */
   if (jitter_samp <= 4294)
      jitter = jitter_samp * 1000000 / sess->clock_rate;
   else {
      jitter = jitter_samp * 1000 / sess->clock_rate;
      jitter *= 1000;
   }

   /* Update jitter statistics */
   pj_math_stat_update(&sess->stat.tx.jitter, jitter);

   /* Can only calculate if LSR and DLSR is present in RR */
   if (rr->lsr && rr->dlsr) {
      pj_uint32_t lsr, now, dlsr;
      pj_uint64_t eedelay;
      pjmedia_rtcp_ntp_rec ntp;

      /* LSR is the middle 32bit of NTP. It has 1/65536 second
       * resolution
       */
      lsr = pj_ntohl(rr->lsr);

      /* DLSR is delay since LSR, also in 1/65536 resolution */
      dlsr = pj_ntohl(rr->dlsr);

      /* Get current time, and convert to 1/65536 resolution */
      pjmedia_rtcp_get_ntp_time(sess, &ntp);
      now = ((ntp.hi & 0xFFFF) << 16) + (ntp.lo >> 16);

      /* End-to-end delay is (now-lsr-dlsr) */
      eedelay = now - lsr - dlsr;

      /* Convert end to end delay to usec (keeping the calculation in
            * 64bit space)::
       *   sess->ee_delay = (eedelay * 1000) / 65536;
       */
      if (eedelay < 4294) {
         eedelay = (eedelay * 1000000) >> 16;
      }
      else {
         eedelay = (eedelay * 1000) >> 16;
         eedelay *= 1000;
      }

      TRACE_((sess->name, "Rx RTCP RR: lsr=%p, dlsr=%p (%d:%03dms), "
              "now=%p, rtt=%p",
              lsr, dlsr, dlsr/65536, (dlsr%65536)*1000/65536,
              now, (pj_uint32_t)eedelay));

      /* Only save calculation if "now" is greater than lsr, or
       * otherwise rtt will be invalid
       */
      if (now-dlsr >= lsr) {
         unsigned rtt = (pj_uint32_t)eedelay;

         /* Check that eedelay value really makes sense.
          * We allow up to 30 seconds RTT!
          */
         if (eedelay > 30 * 1000 * 1000UL) {

            TRACE_((sess->name, "RTT not making any sense, ignored.."));
            goto end_rtt_calc;
         }

         /* "Normalize" rtt value that is exceptionally high.
          * For such values, "normalize" the rtt to be three times
          * the average value.
          */
         if (rtt > ((unsigned)sess->stat.rtt.mean*3) && sess->stat.rtt.n!=0)
         {
            //unsigned orig_rtt = rtt;
            rtt = sess->stat.rtt.mean*3;
            //PJ_LOG(5,(sess->name,
            //          "RTT value %d usec is normalized to %d usec",
            //          orig_rtt, rtt));
         }

         TRACE_((sess->name, "RTCP RTT is set to %d usec", rtt));

         /* Update RTT stat */
         pj_math_stat_update(&sess->stat.rtt, rtt);

      }
      else {
         PJ_LOG(5, (sess->name, "Internal RTCP NTP clock skew detected: "
                    "lsr=%p, now=%p, dlsr=%p (%d:%03dms), "
                    "diff=%d",
                    lsr, now, dlsr, dlsr/65536,
                    (dlsr%65536)*1000/65536,
                    dlsr-(now-lsr)));
      }
   }

end_rtt_calc:

   pj_gettimeofday(&sess->stat.tx.update);
   sess->stat.tx.update_cnt++;
}


PJ_DEF(void) pjmedia_rtcp_build_rtcp(pjmedia_rtcp_session *sess,
                                     void **ret_p_pkt, int *len)
{
   pj_uint32_t expected, expected_interval, received_interval, lost_interval;
   //pjmedia_rtcp_common *common;
   pjmedia_rtcp_sr *sr;
   pjmedia_rtcp_rr *rr;
   pj_timestamp ts_now;
   pjmedia_rtcp_ntp_rec ntp;

   /* Get current NTP time. */
   pj_get_timestamp(&ts_now);
   pjmedia_rtcp_get_ntp_time(sess, &ntp);

   /* See if we have transmitted RTP packets since last time we
    * sent RTCP SR.
    */
   if (sess->stat.tx.pkt != pj_ntohl(sess->rtcp_sr_pkt.sr.sender_pcount))
   {
      /* So we should send RTCP SR */
      *ret_p_pkt = (void*) &sess->rtcp_sr_pkt;
      *len = sizeof(pjmedia_rtcp_sr_pkt);
      //common = &sess->rtcp_sr_pkt.common;
      rr = &sess->rtcp_sr_pkt.rr;
      sr = &sess->rtcp_sr_pkt.sr;

      /* Update packet count */
      sr->sender_pcount = pj_htonl(sess->stat.tx.pkt);

      /* Update octets count */
      sr->sender_bcount = pj_htonl(sess->stat.tx.bytes);

      /* Fill in NTP timestamp in SR. */
      sr->ntp_sec = pj_htonl(ntp.hi);
      sr->ntp_frac = pj_htonl(ntp.lo);

      TRACE_((sess->name, "TX RTCP SR: ntp_ts=%p",
              ((ntp.hi & 0xFFFF) << 16) + ((ntp.lo & 0xFFFF0000)
                                           >> 16)));
   }
   else
   {
      /* We should send RTCP RR then */
      *ret_p_pkt = (void*) &sess->rtcp_rr_pkt;
      *len = sizeof(pjmedia_rtcp_rr_pkt);
      //common = &sess->rtcp_rr_pkt.common;
      rr = &sess->rtcp_rr_pkt.rr;
      sr = NULL;

      TRACE_((sess->name, "TX RTCP RR: ntp_ts=%p",
              ((ntp.hi & 0xFFFF) << 16) + ((ntp.lo & 0xFFFF0000)
                                           >> 16)));
  }

   /* SSRC and last_seq */
   rr->ssrc = pj_htonl(sess->peer_ssrc);
   rr->last_seq = (sess->seq_ctrl.cycles & 0xFFFF0000L);
   /* Since this is an "+=" operation, make sure we update last_seq on
    * both RR and SR.
    */
   sess->rtcp_sr_pkt.rr.last_seq += sess->seq_ctrl.max_seq;
   sess->rtcp_rr_pkt.rr.last_seq += sess->seq_ctrl.max_seq;
   rr->last_seq = pj_htonl(rr->last_seq);


   /* Jitter */
   rr->jitter = pj_htonl(sess->jitter >> 4);


   /* Total lost. */
   expected = pj_ntohl(rr->last_seq) - sess->seq_ctrl.base_seq;

   /* This is bug: total lost already calculated on each incoming RTP!
   if (expected >= sess->received)
   	sess->stat.rx.loss = expected - sess->received;
   else
   	sess->stat.rx.loss = 0;
   */

   rr->total_lost_2 = (sess->stat.rx.loss >> 16) & 0xFF;
   rr->total_lost_1 = (sess->stat.rx.loss >> 8) & 0xFF;
   rr->total_lost_0 = (sess->stat.rx.loss & 0xFF);

   /* Fraction lost calculation */
   expected_interval = expected - sess->exp_prior;
   sess->exp_prior = expected;

   received_interval = sess->received - sess->rx_prior;
   sess->rx_prior = sess->received;

   if (expected_interval >= received_interval)
      lost_interval = expected_interval - received_interval;
   else
      lost_interval = 0;

   if (expected_interval==0 || lost_interval == 0) {
      rr->fract_lost = 0;
   }
   else {
      rr->fract_lost = (lost_interval << 8) / expected_interval;
   }

   if (sess->rx_lsr_time.u64 == 0 || sess->rx_lsr == 0) {
      rr->lsr = 0;
      rr->dlsr = 0;
   }
   else {
      pj_timestamp ts;
      pj_uint32_t lsr = sess->rx_lsr;
      pj_uint64_t lsr_time = sess->rx_lsr_time.u64;
      pj_uint32_t dlsr;

      /* Convert LSR time to 1/65536 seconds resolution */
      lsr_time = (lsr_time << 16) / sess->ts_freq.u64;

      /* Fill in LSR.
         LSR is the middle 32bit of the last SR NTP time received.
       */
      rr->lsr = pj_htonl(lsr);

      /* Fill in DLSR.
         DLSR is Delay since Last SR, in 1/65536 seconds.
       */
      ts.u64 = ts_now.u64;

      /* Convert interval to 1/65536 seconds value */
      ts.u64 = (ts.u64 << 16) / sess->ts_freq.u64;

      /* Get DLSR */
      dlsr = (pj_uint32_t)(ts.u64 - lsr_time);
      rr->dlsr = pj_htonl(dlsr);

      TRACE_((sess->name,"Tx RTCP RR: lsr=%p, lsr_time=%p, now=%p, dlsr=%p"
              "(%ds:%03dms)",
              lsr,
              (pj_uint32_t)lsr_time,
              (pj_uint32_t)ts.u64,
              dlsr,
              dlsr/65536,
              (dlsr%65536)*1000/65536 ));
   }

   /* Update counter */
   pj_gettimeofday(&sess->stat.rx.update);
   sess->stat.rx.update_cnt++;
}

PJ_DEF(pj_status_t) pjmedia_rtcp_enable_xr( pjmedia_rtcp_session *sess,
      pj_bool_t enable)
{
#if defined(PJMEDIA_HAS_RTCP_XR) && (PJMEDIA_HAS_RTCP_XR != 0)

   /* Check if request won't change anything */
   if (!(enable ^ sess->xr_enabled))
      return PJ_SUCCESS;

   if (!enable) {
      sess->xr_enabled = PJ_FALSE;
      return PJ_SUCCESS;
   }

   pjmedia_rtcp_xr_init(&sess->xr_session, sess, 0, 1);
   sess->xr_enabled = PJ_TRUE;

   return PJ_SUCCESS;

#else

   PJ_UNUSED_ARG(sess);
   PJ_UNUSED_ARG(enable);
   return PJ_ENOTSUP;

#endif
}
