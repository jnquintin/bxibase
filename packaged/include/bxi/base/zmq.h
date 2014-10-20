/* -*- coding: utf-8 -*-
 ###############################################################################
 # Authors: Sébastien Miquée <sebastien.miquee@bull.net>
 #          Pierre Vignéras <pierre.vigneras@bull.net>
 # Created on: 2013/11/21
 # Contributors:
 ###############################################################################
 # Copyright (C) 2013  Bull S. A. S.  -  All rights reserved
 # Bull, Rue Jean Jaures, B.P.68, 78340, Les Clayes-sous-Bois
 # This is not Free or Open Source software.
 # Please contact Bull S. A. S. for details about its license.
 ###############################################################################
 */

#ifndef BXIZMQ_H_
#define BXIZMQ_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <zmq.h>

#include <bxi/base/err.h>


// *********************************************************************************
// ********************************** Defines **************************************
// *********************************************************************************

#define BXIZMQ_RETRIES_MAX_ERR 1
#define BXIZMQ_FSM_ERR 2
#define BXIZMQ_MISSING_FRAME_ERR 3

// *********************************************************************************
// ********************************** Types   **************************************
// *********************************************************************************

// *********************************************************************************
// ********************************** Global Variables *****************************
// *********************************************************************************

// *********************************************************************************
// ********************************** Interface ************************************
// *********************************************************************************

/**
 * Create a zmq socket, connect to the given url and set the specified option on it.
 *
 * Any error encountered leads to a NULL pointer returned.
 *
 * @param ctx the zeromq context
 * @param type the type of zeromq socket to create
 * @param url the url to connect to or bind to
 * @param bind whether the socket should be bind or connect to the given url
 * @param set_option whether an option should be set on the created socket
 * @param option_name the name of the option
 * @param option_value the value of the option
 * @param option_len the length of the option value
 * @param result the newly created socket (NULL on error)
 * @return BXIERR_OK on success, any other on failure.
 */
bxierr_p bxizmq_zocket_new(void * const ctx, const int type,
                           const char * const url, const bool bind,
                           const bool set_option,
                           const int option_name,
                           const void * const option_value,
                           const size_t option_len, void ** result);


/**
 * Cleanup the given zeromq socket, releasing all underlying resources.
 *
 * @param zocket the socket to cleanup
 * @return BXIERR_OK on success, any other on failure.
 */
bxierr_p bxizmq_zocket_cleanup(void * const zocket);


/**
 * Initialize the given zeromq message.
 *
 * @param zmsg the zeromq message to initialize
 * @return BXIERR_OK on success, any other on failure.
 */
bxierr_p bxizmq_msg_init(zmq_msg_t * zmsg);


/**
 * Close the given msg and exit on errors.
 *
 * @param zmsg the zeromq message to close
 * @return BXIERR_OK on success, any other on failure.
 */
bxierr_p bxizmq_msg_close(zmq_msg_t * zmsg);

/*
 *
 */
/**
 * Try asynchronous sending of the given zmq message `zmsg`
 * through the given zmq socket `zocket`.
 *
 * After each failure (EAGAIN) sleep for `delay` seconds.
 * If after `retries_max` retries, it still fails (EAGAIN), retry
 * synchronously and return a bxierr with error code
 * `BXIZMQ_RETRIES_MAX_ERR` and the number of retries
 * as the error data (casted from a `size_t`, it is not a pointer).
 *
 * Note: if `retries_max == 0`, this call is equivalent to a synchronous
 * send.
 *
 * @param zmsg the zeromq message to send
 * @param zocket the zeromq socket the message must be sent from
 * @param flags a zeromq flag
 * @param retries_max the number of retries before switching to synchronous sending
 * @param delay_ns the maximum number of nanoseconds to sleep before a retry
 * @return BXIERR_OK on succes,
 *         `bxierr(code=BXIZMQ_RETRIES_MAX_ERR, data=(size_t) retries_nb)`
 *         among others.
 */
bxierr_p bxizmq_snd_msg(zmq_msg_t * zmsg,
                        void * zocket, int flags,
                        const size_t retries_max, long delay_ns);


/**
 * Copy a ZMQ message.
 * @param zmsg_src the zeromq message source
 * @param zmsg_dst the zeromq message destination
 * @return BXIERR_OK on success, any other on failure.
 */
bxierr_p bxizmq_copy_msg(zmq_msg_t * zmsg_src, zmq_msg_t * zmsg_dst);


/**
 * Receive a message.
 *
 * @param zocket the zeromq socket
 * @param zmsg the zeromq message
 * @param flags the zeromq flag
 * @return BXIERR_OK on success, any other on failure.
 */
bxierr_p bxizmq_rcv_msg(void * zocket, zmq_msg_t * zmsg, int flags);


/**
 * Try to receive a message asynchronously a given maximum of `retries_max` times.
 *
 * After each failure (EAGAIN), the calling thread sleep for `delay_ns` nanoseconds
 * and tries again, up to `retries_max` attempt.
 *
 * If after `retries_max` attempt, the message cannot be received without blocking,
 * the error with error code BXIERR_RETRIES_MAX_ERR is returned with the number of
 * retries as the data (casted from a `size_t`, it is not a pointer).
 *
 * @param zocket the zeromq socket
 * @param msg the zeromq message
 * @return BXIERR_OK on success,
 *         `bxierr(code=BXIZMQ_RETRIES_MAX_ERR, data=(size_t) retries_nb)`
 *         among others
 */
bxierr_p bxizmq_recv_async(void *zocket, zmq_msg_t *msg,
                           size_t retries_max, long delay_ns);

/**
 * Send a copy of the given zmsg.
 *
 * See `bxizmq_snd_msg()` for other parameters description.
 *
 * @param zmsg
 * @param zocket
 * @param flags
 * @param retries_max
 * @param delay_ns
 * @return BXIERR_OK on success
 * @see bxizmq_snd_msg()
 */
bxierr_p bxizmq_snd_msg_cpy(zmq_msg_t * zmsg,
                            void * zocket, int flags,
                            const size_t retries_max, long delay_ns);

/*
 * Try asynchronous sending of the given *data through the given zocket.
 * After each failure (EAGAIN) sleep for 'delay' seconds.
 * If after 'retries_max' retries, it still fail (EAGAIN), retry
 * synchronously.
 *
 * Return the number of retries (0 means the first send was successful).
 *
 * If retries_max == 0, this call is equivalent to a synchronous send
 * (with all the checks performed).
 *
 *
 *
 */
/**
 * Send the given data through the given zeromq socket.
 *
 * See bxizmq_snd_msg() for details on parameters and returned code.
 *
 * Usage:
 * int i = 12345;
 * // Synchronous send
 * bxierr_p err = bxizmq_snd_msg(&i, sizeof(int), socket, 0, 0, 0);
 * assert(BXIERR_OK == retries);
 * // Asynchronous send, 4 retries, sleep 1us
 * err = bxizmq_snd_msg(&i, sizeof(int), socket, ZMQ_DONTWAIT, 4, 1000);
 * assert(BXIERR_OK || 4 >= (size_t) err.data);
 *
 * @param data
 * @param size
 * @param zocket
 * @param flags
 * @param retries_max
 * @param delay_ns
 * @return
 * @see bxizmq_snd_msg()
 */
bxierr_p bxizmq_snd_data(void * data, size_t size, void * zocket,
                         int flags, size_t retries_max,
                         long delay_ns);



/**
 * Function utility to be used with bximisc_snd_data_zc()
 * and bxizmq_snd_str_zc() functions.
 *
 * @param data the data to free
 * @param hint
 */
void bxizmq_simple_free(void * data, void * hint);

/**
 * Equivalent to bxizmq_snd_msg() but with zero copy.
 *
 * This means that the given *data pointer should not be used afterwards!
 *
 * The given data will be freed using the given 'ffn' function with the data
 * and the given hint in parameter. If you need a simple free(data)
 * without the hint, use bxizmq_simple_free() as in the following:
 *
 *      bxizmq_snd_msg_zc(data, size, zocket, flags, retries_max, delay_ns,
 *                        bxizmq_simple_free, NULL);
 *
 * @param data
 * @param size
 * @param zocket
 * @param flags
 * @param retries_max
 * @param delay_ns
 * @param ffn
 * @param hint
 * @return
 * @see bxizmq_snd_msg()
 */
bxierr_p bxizmq_snd_data_zc(const void * data, size_t size, void * zocket,
                            int flags, size_t retries_max,
                            long delay_ns,
                            zmq_free_fn *ffn, void *hint);


/**
 * Send the given string through the given zocket with the given ZMQ flags.
 *
 *
 * @param str
 * @param zocket
 * @param flags
 * @param retries_max
 * @param delay_ns
 * @return
 * @see bxizmq_snd_msg()
 */
bxierr_p bxizmq_snd_str(char * const str, void * zocket, int flags,
                        size_t retries_max, long delay_ns);


/**
 * Send the given string through the given zocket with the given ZMQ flags without
 * performing a copy. The given string should not be modified after this call.
 *
 * @param str
 * @param zocket
 * @param flags
 * @param retries_max
 * @param delay_ns
 * @return
 * @see bxizmq_snd_msg()
 */
bxierr_p bxizmq_snd_str_zc(const char * const str, void * zocket, int flags,
                           size_t retries_max, long delay_ns);

/**
 * Receive a zmq message through the given zocket with the given ZMQ flags,
 * fills the given `*result` pointer with the received content and return the number of
 * bytes received in the `received_size` pointer.
 *
 * If `*result` is given NULL, then it is set to a new allocated area filled with
 * the message content. This area will have to be freed using BXIFREE(). The size of
 * the allocated region is of the specified expected size unless it is 0. In this case
 * the allocated region is of the received size.
 *
 * If `*result` is not NULL, it is filled with the message content unless the
 * received message size is strictly greater than the given `expected_size`.
 * In this case the program will exit with an error message (since it is
 * dangerous to fill `*result` with more bytes than expected).
 *
 * Note that if the received message size is less than `expected_size` the
 * `*result` will be filled up to the received message size and no more.
 * Therefore, `*result` might holds inconsistent data: the beginning of the region
 * contains a copy of the message content up to the received size, but the rest
 * (expected_size - received_size) will still contains its previous content.
 *
 * If this is a problem, check that `*received_size == expected_size`.
 *
 * if `check_more` is `true`, a check is made to know if another zeromq frame is expected
 * from the given zocket (using zmq_getsockopt()) that is related to a
 * zeromq multipart message. If it is not the case, a bxierr(code=BXIZMQ_MISSING_FRAME_ERR)
 * is returned.
 *
 * If `flags` contains ZMQ_DONTWAIT and EAGAIN was returned while receiving (meaning
 * no message was received at that time), then `*result` will be NULL and
 * `*received_size` will be 0.
 *
 * If `expected_size` is 0 and `*received_size` is 0, the given `*result` pointer is left
 * untouched and BXIERR_OK is returned. This can be used for signaling purpose
 * (if you send something with a zero size), you will receive a zero sized message.
 *
 * @param result a pointer on the storage location where the received data must be stored
 * @param expected_size the expected size of the received data
 * @param zocket the zeromq socket the data should be received from
 * @param flags a zeromq flag
 * @param check_more tells if the next data must be part of a zeromq message or not
 * @param received_size the actual size of the received data
 * @return BXIERR_OK on success, bxierr(code=BXIZMQ_MISSING_FRAME_ERR) among others.
 */
bxierr_p bxizmq_rcv_data(void ** result, size_t expected_size,
                         void * zocket, int flags,
                         bool check_more, size_t *received_size);

/**
 * Receive a string through the given zocket with the given ZMQ flags.
 * Note that the returned string has been allocated. Therefore, it should
 * be freed using BXIFREE().
 *
 * if ZMQ_DONTWAIT has been set in flags, this function can return NULL,
 * meaning that nothing was (fully) received at the time this call was made,
 * and suggesting that retrying might work.
 *
 * if `check_more` is `true`, a check is made to know if another zeromq frame is expected
 * from the given zocket (using zmq_getsockopt()) that is related to a
 * zeromq multipart message. If it is not the case, the program exit with an
 * error message.
 *
 * Receiving a multipart message looks like the following sample code:
 *
 *      char * frame0, frame1, frame2;
 *      bxierr_p err0, err1, err2;
 *      err0 = bxizmq_rcv_str(zocket, 0, false, &frame0);
 *      err1 = bxizmq_rcv_str(zocket, 0, true, &frame1);
 *      err2 = bxizmq_rcv_str(zocket, 0, true, &frame2);
 *      ...
 *      BXIFREE(frame0);
 *      BXIFREE(frame1);
 *      BXIFREE(frame2);
 *
 * @param zocket
 * @param flags
 * @param check_more
 * @return BXIERR_OK on success, any other bxierr otherwise
 */
bxierr_p bxizmq_rcv_str(void * zocket, int flags, bool check_more, char ** result);


/**
 * Return true if the next frame the given zocket will received comes
 * from a multipart message.
 *
 * @param zocket
 * @return
 */
bxierr_p bxizmq_has_more_msg(void *zocket, bool* result);

#endif /* BXIZMQ_H_ */