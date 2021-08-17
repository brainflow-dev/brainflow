/* pipe.h - The public pipe interface. This is the only file that must be
 *          included to begin using the pipe.
 *
 * The MIT License
 * Copyright (c) 2011 Clark Gaebel <cg.wowus.cg@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once /* If your compiler doesn't support this construct, feel free to
                use include guards. Most do, so I'm keeping it in. */

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
#define PURE                __attribute__((pure))
#define MALLOC_LIKE         __attribute__((malloc))
#define NO_NULL_POINTERS    __attribute__((nonnull))
#define WARN_UNUSED_RESULT  __attribute__((warn_unused_result))
#else
/* Feel free to fill in results for more compilers =) */
#define PURE
#define MALLOC_LIKE
#define NO_NULL_POINTERS
#define WARN_UNUSED_RESULT
#endif

/*
 * A pipe is a collection of elements, enqueued and dequeued in a FIFO pattern.
 * The beauty of it lies in that pushing and popping may be done in multiple
 * threads, with no need for external synchronization. This makes it ideal for
 * assisting in a producer/consumer concurrency model.
 *
 * pipe_t
 *
 * If there is a valid pipe_t in circulation, you may create producer_t and
 * consumer_t handles from it. It also allows you to run maintainance tasks
 * such as pipe_reserve.
 *
 * Sample code:
 *
 *   #include "pipe.h"
 *
 *   #define THREADS 4
 *
 *   pipe_t* p = pipe_new(sizeof(int), 0);
 *
 *   pipe_producer_t* pros[THREADS] = { pipe_producer_new(p) };
 *   pipe_consumer_t* cons[THREADS] = { pipe_consumer_new(p) };
 *
 *   pipe_free(p);
 *
 *   // At this point, you can freely use the producer and consumer handles.
 *
 *   // Then clean them up when you're done!
 *   for(int i = 0; i < THREADS; ++i)
 *   {
 *     pipe_producer_free(pros[i]);
 *     pipe_consumer_free(cons[i]);
 *   }
 *
 * pipe_producer_t
 *
 * pipe_producer_t is used for pushing into the pipe. It is recommended that
 * each thread has their own (as it keeps ownership semantics simple); however,
 * it is not mandatory. As long as there is at least one pipe_producer_t in
 * circulation, consumers will block until they can fill their buffers. A
 * pipe_t also counts as a pipe_producer_t, since valid pipe_producer_t handles
 * can be created from it.
 *
 * pipe_consumer_t
 *
 * pipe_consumer_t is used for popping from the pipe. It is recommended that
 * each thread has their own (as it keeps ownership semantics simple), however,
 * it is not mandatory. As long as there is at least one pipe_producer_t or
 * pipe_t in circulation, consumers will block until the buffer can be filled.
 * A pipe_t also counts as a pipe_consumer_t, since valid pipe_consumer_t
 * handles can be created from it.
 *
 * Sample code:
 *
 * #include "pipe.h"
 *
 * #define BUFSIZE 1024
 *
 * void do_stuff(pipe_consumer_t* p)
 * {
 *   int buf[BUFSIZE];
 *   size_t bytes_read;
 *
 *   while((bytes_read = pipe_pop(p, buf, BUFSIZE)))
 *     process(buf, bytes_read);
 *
 *   pipe_consumer_free(p);
 * }
 *
 * Try and keep the pipe_t allocated for as short a time as possible. This
 * means you should make all your producer and consumer handles at the start,
 * deallocate the pipe_t, then use producers and consumers for the duration
 * of your task.
 *
 * pipe_generic_t
 *
 * Generic pipe pointers can be created from pipe_t's, producer_t's, or
 * consumer_t's with the PIPE_GENERIC macro. This new pointer can then be used
 * as a parameter to any function requiring it, allowing certain operations to
 * be performed regardless of the type.
 *
 * Guarantees:
 *
 * If, in a single thread, elements are pushed in the order [a, b, c, d, ...],
 * they will be popped in that order. Note that they may not necessarily be
 * popped together.
 *
 * The underlying pipe will exist until all the handles have been freed.
 *
 * All functions are re-entrant. Synchronization is handled internally.
 */
typedef struct pipe_t          pipe_t;
typedef struct pipe_producer_t pipe_producer_t;
typedef struct pipe_consumer_t pipe_consumer_t;
typedef struct pipe_generic_t  pipe_generic_t;

#define PIPE_GENERIC(handle) ((pipe_generic_t*)(handle))

/*
 * Initializes a new pipe storing elements of size `elem_size'. A pusher handle
 * is returned, from which you may push elements into the pipe.
 *
 * If `limit' is 0, the pipe has no maximum size. If it is nonzero, the pipe
 * will never have more than `limit' elements in it at any time. In most cases,
 * you want this to be 0. However, limits help prevent an explosion of memory
 * usage in cases where production is significantly faster than consumption.
 */
pipe_t* MALLOC_LIKE WARN_UNUSED_RESULT pipe_new(size_t elem_size, size_t limit);

/*
 * Makes a production handle to the pipe, allowing push operations. This
 * function is extremely cheap; it doesn't allocate memory.
 */
pipe_producer_t* NO_NULL_POINTERS WARN_UNUSED_RESULT pipe_producer_new(pipe_t*);

/*
 * Makes a consumption handle to the pipe, allowing pop operations. This
 * function is extremely cheap; it doesn't allocate memory.
 */
pipe_consumer_t* NO_NULL_POINTERS WARN_UNUSED_RESULT pipe_consumer_new(pipe_t*);

/*
 * If you call *_new, you must call the corresponding *_free. Failure to do so
 * may result in resource leaks, undefined behavior, and spontaneous combustion.
 */

void pipe_free(pipe_t*);
void pipe_producer_free(pipe_producer_t*);
void pipe_consumer_free(pipe_consumer_t*);

/* Copies `count' elements from `elems' into the pipe. */
void NO_NULL_POINTERS pipe_push(pipe_producer_t*, const void* elems, size_t count);

/*
 * Copies `count' elements from `elems' into the pipe.
 *
 * WARNING: You probably want pipe_push.
 *
 * If the pipe is full, this version of pipe_push will automatically pop enough
 * elements from the front of the queue to make room for the new elements in
 * the pipe.
 *
 * Disabled unless someone tells me they want it. Enabling this function voids
 * your warrant.
 */
/*
void NO_NULL_POINTERS pipe_push_clobber(pipe_producer_t*,
                                        const void* elems, size_t count);
*/

/*
 * Tries to pop `count' elements out of the pipe and into `target', returning
 * the number of elements successfully copied. This function will block until:
 *
 * a) `target' has been filled with `count' elements, or
 * b) all producer_t handles have been freed (including the parent pipe_t).
 *
 * If this function returns 0, there will be no more elements coming in. Every
 * subsequent call will return 0.
 *
 * The difference between this function and pipe_pop_eager is that this one will
 * do its best to fill `target' before returning whereas pipe_pop_eagar will
 * return as soon as any elements are available.
 */
size_t NO_NULL_POINTERS WARN_UNUSED_RESULT pipe_pop(pipe_consumer_t*,
                                                    void* target,
                                                    size_t count);

/*
 * Tries to pop `count' elements out of the pipe and into `target', returning
 * the number of elements successfully copied. This function will block until:
 *
 * a) there is at least one element in the pipe.
 * b) all producer_t handles have been freed (including the parent pipe_t).
 *
 * If this function returns 0, there will be no more elements coming in. Every
 * subsequent call will return 0.
 *
 * The difference between this function and pipe_pop is that this one will
 * return as soon as any elements are available, whereas pipe_pop will do its
 * best to fill `target' first.
 */
size_t NO_NULL_POINTERS WARN_UNUSED_RESULT pipe_pop_eager(pipe_consumer_t*,
                                                          void* target,
                                                          size_t count);

/*
 * Modifies the pipe to have room for at least `count' elements. If more room
 * is already allocated, the call does nothing. This can be useful if requests
 * tend to come in bursts.
 *
 * The default minimum is 32 elements. To reset the reservation size to the
 * default, set count to 0.
 */
void NO_NULL_POINTERS pipe_reserve(pipe_generic_t*, size_t count);

/*
 * Determines the size of a pipe's elements. This can be used for generic
 * pipe-processing algorithms to reserve appropriately-sized buffers.
 */
size_t PURE NO_NULL_POINTERS pipe_elem_size(pipe_generic_t*);

/*
 * Use this to run the pipe self-test. It will call abort() if anything is
 * wrong. This is usually unnecessary. If this is never called, pipe_test.c
 * does not need to be linked.
 */
void pipe_run_test_suite(void);

#undef WARN_UNUSED_RESULT
#undef NO_NULL_POINTERS
#undef MALLOC_LIKE
#undef PURE

#ifdef __cplusplus
}
#endif

/* vim: set et ts=4 sw=4 softtabstop=4 textwidth=80: */