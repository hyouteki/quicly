/*
 * Copyright (c) 2020 Fastly, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef received_cid_h
#define received_cid_h

#include "quicly/cid.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct st_quicly_received_cid_t quicly_received_cid_t;
typedef struct st_quicly_received_cid_set_t quicly_received_cid_set_t;

/**
 * records a CID given by the peer
 */
struct st_quicly_received_cid_t {
    /**
     * indicates whether this record holds an active (given by peer and not retired) CID
     */
    int is_active;
    /**
     * sequence number of the CID
     *
     * If is_active, this represents the sequence number associated with the CID.
     * If !is_active, this represents a "reserved" slot, meaning that we are expecting to receive a NEW_CONNECTION_ID frame
     * with this sequence number. This helps determine if a received frame is carrying a CID that is already retired.
     */
    uint64_t sequence;
    struct st_quicly_cid_t cid;
    uint8_t stateless_reset_token[QUICLY_STATELESS_RESET_TOKEN_LEN];
};

/**
 * structure to hold active connection IDs received from the peer
 */
struct st_quicly_received_cid_set_t {
    /**
     * we retain QUICLY_LOCAL_ACTIVE_CONNECTION_ID_LIMIT active connection IDs
     * cids[0] holds the current (in use) CID which is used when emitting packets
     */
    struct st_quicly_received_cid_t cids[QUICLY_LOCAL_ACTIVE_CONNECTION_ID_LIMIT];
    /**
     * we expect to receive CIDs with sequence number smaller than or equal to this number
     */
    uint64_t _largest_sequence_expected;
};

void quicly_received_cid_init(struct st_quicly_received_cid_set_t *set);
/**
 * registers received connection ID
 * returns 0 if successfully (registered or ignored because of duplication/stale information), transport error code otherwise
 */
int quicly_received_cid_register(struct st_quicly_received_cid_set_t *set, uint64_t sequence, const uint8_t *cid, size_t cid_len,
                                 const uint8_t srt[QUICLY_STATELESS_RESET_TOKEN_LEN]);
/**
 * unregisters specified CID from the store
 * returns 0 if success, 1 if failure
 */
int quicly_received_cid_unregister(struct st_quicly_received_cid_set_t *set, uint64_t sequence);
/**
 * unregister all CIDs with sequence number smaller than seq_unreg_prior_to
 *
 * @param unregistered_seqs sequence numbers of unregistered CIDs are returned, packed from the begining of the array
 * @return the number of unregistered CIDs
 */
size_t quicly_received_cid_unregister_prior_to(struct st_quicly_received_cid_set_t *set, uint64_t seq_unreg_prior_to,
                                               uint64_t unregistered_seqs[QUICLY_LOCAL_ACTIVE_CONNECTION_ID_LIMIT]);

#ifdef __cplusplus
}
#endif

#endif
