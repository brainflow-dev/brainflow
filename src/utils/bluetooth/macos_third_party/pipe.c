/* pipe.c - The pipe implementation. This is the only file that must be linked
 *          to use the pipe.
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
#include "pipe.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Vanity bytes. As long as this isn't removed from the executable, I don't
// mind if I don't get credits in a README or any other documentation. Consider
// this your fulfillment of the MIT license.
const char _pipe_copyright[] =
    __FILE__
    " : Copyright (c) 2011 Clark Gaebel <cg.wowus.cg@gmail.com> (MIT License)";

#ifndef min
#define min(a, b) ((a) <= (b) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) ((a) >= (b) ? (a) : (b))
#endif

#ifdef __GNUC__
#define likely(cond)   __builtin_expect(!!(cond), 1)
#define unlikely(cond) __builtin_expect(  (cond), 0)
#define CONSTEXPR __attribute__((const))
#define PURE      __attribute__((pure))
#else
#define likely(cond)   (cond)
#define unlikely(cond) (cond)
#define CONSTEXPR
#define PURE
#endif

#ifdef NDEBUG
    #if defined(_MSC_VER)
        #define assertume __assume
    #else // _MSC_VER
        #define assertume assert
    #endif // _MSC_VER
#else // NDEBUG
    #define assertume assert
#endif // NDEBUG

// The number of spins to do before performing an expensive kernel-mode context
// switch. This is a nice easy value to tweak for your application's needs. Set
// it to 0 if you want the implementation to decide, a low number if you are
// copying many objects into pipes at once (or a few large objects), and a high
// number if you are coping small or few objects into pipes at once.
#define MUTEX_SPINS 8192

// Standard threading stuff. This lets us support simple synchronization
// primitives on multiple platforms painlessly.

#if defined(_WIN32) || defined(_WIN64) // use the native win32 API on windows

#include <windows.h>

// On vista+, we have native condition variables and fast locks. Yay.
#if defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0600

#define mutex_t             SRWLOCK

#define mutex_init          InitializeSRWLock
#define mutex_lock          AcquireSRWLockExclusive
#define mutex_unlock        ReleaseSRWLockExclusive
#define mutex_destroy(m)

#define cond_t              CONDITION_VARIABLE

#define cond_init           InitializeConditionVariable
#define cond_signal         WakeConditionVariable
#define cond_broadcast      WakeAllConditionVariable
#define cond_wait(c, m)     SleepConditionVariableSRW((c), (m), INFINITE, 0)
#define cond_destroy(c)

// Oh god. Microsoft has slow locks and lacks native condition variables on
// anything lower than Vista. Looks like we're rolling our own today.
#else /* vista+ */

#define mutex_t         CRITICAL_SECTION

#define mutex_init(m)   InitializeCriticalSectionAndSpinCount((m), MUTEX_SPINS)
#define mutex_lock      EnterCriticalSection
#define mutex_unlock    LeaveCriticalSection
#define mutex_destroy   DeleteCriticalSection

// This Condition variable implementation is stolen from:
// http://www.cs.wustl.edu/~schmidt/win32-cv-1.html (section 3.3)

typedef struct cond_t {
    // Count of the number of waiters, with a critical section to serialize
    // accesses to it.
    int waiters_count;
    CRITICAL_SECTION waiters_count_lock;

    // Number of threads to release via a cond_broadcast or a cond_signal.
    int release_count;

    // Keeps track of the current "generation" so that a single thread can't
    // steal all the resources from a broadcast.
    int wait_generation_count;

    // A manual-reset event that's used to block and release waiting threads.
    HANDLE event;
} cond_t;

static void cond_init(cond_t* c)
{
    c->waiters_count = 0;
    InitializeCriticalSection(&c->waiters_count_lock);
    c->release_count = 0;
    c->wait_generation_count = 0;
    c->event = CreateEvent(NULL, true, false, NULL);
}

static void cond_signal(cond_t* c)
{
    EnterCriticalSection(&c->waiters_count_lock);
    if(c->waiters_count > c->release_count)
    {
        SetEvent(c->event);
        c->release_count++;
        c->wait_generation_count++;
    }
    LeaveCriticalSection(&c->waiters_count_lock);
}

static void cond_broadcast(cond_t* c)
{
    EnterCriticalSection(&c->waiters_count_lock);
    if(c->waiters_count > 0)
    {
        SetEvent(c->event);

        // Release all the threads in this generation.
        c->release_count = c->waiters_count;

        // Start a new generation.
        c->wait_generation_count++;
    }
    LeaveCriticalSection(&c->waiters_count_lock);
}

static void cond_wait(cond_t* c, mutex_t* m)
{
    EnterCriticalSection(&c->waiters_count_lock);

    c->waiters_count++;

    int my_generation = c->wait_generation_count;

    LeaveCriticalSection(&c->waiters_count_lock);
    mutex_unlock(m);

    bool wait_done;

    do
    {
        WaitForSingleObject(c->event, INFINITE);

        EnterCriticalSection(&c->waiters_count_lock);
        int release_count = c->release_count;
        int wait_generation_count = c->wait_generation_count;
        LeaveCriticalSection(&c->waiters_count_lock);

        wait_done = release_count > 0
                 && wait_generation_count != my_generation;
    }
    while(!wait_done);

    mutex_lock(m);
    EnterCriticalSection(&c->waiters_count_lock);
    c->waiters_count--;
    int release_count = --c->release_count;
    LeaveCriticalSection(&c->waiters_count_lock);

    if(release_count == 0) // we're the last waiter
        ResetEvent(c->event);
}

static void cond_destroy(cond_t* c)
{
    DeleteCriticalSection(&c->waiters_count_lock);
    CloseHandle(c->event);
}

#endif /* vista+ */

// Fall back on pthreads if we haven't special-cased the current OS.
#else /* windows */

#include <pthread.h>

#define mutex_t pthread_mutex_t
#define cond_t  pthread_cond_t

#define mutex_init(m)  pthread_mutex_init((m), NULL)

#define mutex_lock     pthread_mutex_lock
#define mutex_unlock   pthread_mutex_unlock
#define mutex_destroy  pthread_mutex_destroy

#define cond_init(c)   pthread_cond_init((c), NULL)
#define cond_signal    pthread_cond_signal
#define cond_broadcast pthread_cond_broadcast
#define cond_wait      pthread_cond_wait
#define cond_destroy   pthread_cond_destroy

#endif /* windows */

// End threading.

/*
 * Pipe implementation overview
 * =================================
 *
 * A pipe is implemented as a circular buffer. There are two special cases for
 * this structure: nowrap and wrap.
 *
 * Nowrap:
 *
 *     buffer          begin               end                 bufend
 *       [               >==================>                    ]
 *
 * In this case, the data storage is contiguous, allowing easy access. This is
 * the simplest case.
 *
 * Wrap:
 *
 *     buffer        end                 begin                 bufend
 *       [============>                    >=====================]
 *
 * In this case, the data storage is split up, wrapping around to the beginning
 * of the buffer when it hits bufend. Hackery must be done in this case to
 * ensure the structure is maintained and data can be easily copied in/out.
 *
 * Data is 'push'ed after the end pointer and 'pop'ed from the begin pointer.
 * There is always one sentinel element in the pipe, to distinguish between an
 * empty pipe and a full pipe.
 *
 * Invariants:
 *
 * The invariants of a pipe are documented in the check_invariants function,
 * and double-checked frequently in debug builds. This helps restore sanity when
 * making modifications, but may slow down calls. It's best to disable the
 * checks in release builds.
 *
 * Thread-safety:
 *
 * pipe_t has been designed with high threading workloads foremost in my mind.
 * Its initial purpose was to serve as a task queue, with multiple threads
 * feeding data in (from disk, network, etc) and multiple threads reading it
 * and processing it in parallel. This created the need for a fully re-entrant,
 * lightweight, accommodating data structure.
 *
 * We have two locks guarding the pipe, instead of the naive solution of having
 * one. One guards writes to the begin pointer, the other guards writes to the
 * end pointer. This is due to the realization that when pushing, you don't need
 * an up-to-date value for begin, and when popping you don't need an up-to-date
 * value for end (since either can only move forward in the buffer). As long as
 * neither moves backwards, there will be no conflicts when they move
 * independently of each other. This optimization has improved benchmarks by
 * 15-20%.
 *
 * Complexity:
 *
 * Pushing and popping must run in O(n) where n is the number of elements being
 * inserted/removed. It must also run in O(1) with respect to the number of
 * elements in the pipe.
 *
 * Efficiency:
 *
 * Asserts are used liberally, and many of them, when inlined, can be turned
 * into no-ops. Therefore, it is recommended that you compile with -O1 in
 * debug builds as the pipe can easily become a bottleneck.
 */
struct pipe_t {
    size_t elem_size,  // The size of each element. This is read-only and
                       // therefore does not need to be locked to read.
           min_cap,    // The smallest sane capacity before the buffer refuses
                       // to shrink because it would just end up growing again.
                       // To modify this variable, you must lock the whole pipe.
           max_cap;    // The maximum capacity of the pipe before push requests
                       // are blocked. To read or write to this variable, you
                       // must hold 'end_lock'.

    char*  buffer,     // The internal buffer, holding the enqueued elements.
                       // to modify this variable, you must lock the whole pipe.
        *  bufend,     // One past the end of the buffer, so that the actual
                       // elements are stored in in interval [buffer, bufend).
        *  begin,      // Always points to the sentinel element. `begin + elem_size`
                       // points to the left-most element in the pipe.
                       // To modify this variable, you must lock begin_lock.
        *  end;        // Always points past the right-most element in the pipe.
                       // To modify this variable, you must lock end_lock.

    // The number of producers/consumers in the pipe.
    size_t producer_refcount, // Guarded by begin_lock.
           consumer_refcount; // Guarded by end_lock.

    // Our lovely mutexes. To lock the pipe, call lock_pipe. Depending on what
    // you modify, you may be able to get away with only locking one of them.
    mutex_t begin_lock,
            end_lock;

    cond_t just_pushed, // Signaled immediately after a push.
           just_popped; // Signaled immediately after a pop.
};

// Converts a pointer to either a producer or consumer into a suitable pipe_t*.
#define PIPIFY(handle) ((pipe_t*)(handle))

// We wrap elem_size in a function so we can annotate it with PURE, allowing
// the compiler's CSE to eliminate extraneous memory accesses.
static inline PURE size_t __pipe_elem_size(pipe_t* p)
{
    return p->elem_size;
}

size_t pipe_elem_size(pipe_generic_t* p)
{
    return __pipe_elem_size(PIPIFY(p));
}


// Represents a snapshot of a pipe. We often don't need all our values
// up-to-date (usually only one of begin or end). By passing this around, we
// avoid constantly wrecking our cache by accessing the real pipe_t.
typedef struct {
    char*   buffer,
        *   bufend,
        *   begin,
        *   end;
    size_t elem_size;
} snapshot_t;

static inline snapshot_t make_snapshot(pipe_t* p)
{
    return (snapshot_t) {
        .buffer = p->buffer,
        .bufend = p->bufend,
        .begin  = p->begin,
        .end    = p->end,
        .elem_size = __pipe_elem_size(p),
    };
}

// The initial minimum capacity of the pipe. This can be overridden dynamically
// with pipe_reserve.
#ifdef PIPE_DEBUG
#define DEFAULT_MINCAP  2
#else
#define DEFAULT_MINCAP  32
#endif

// Returns the maximum number of bytes the buffer can hold, excluding the
// sentinel element.
static inline size_t capacity(snapshot_t s)
{
    return s.bufend - s.buffer - s.elem_size;
}

// Does the buffer wrap around?
//   true  -> wrap
//   false -> nowrap
static inline bool wraps_around(snapshot_t s)
{
    return unlikely(s.begin >= s.end);
}

// Returns the number of bytes currently in use in the buffer, excluding the
// sentinel element.
static inline size_t bytes_in_use(snapshot_t s)
{
    return (wraps_around(s)
    //         v   right half   v   v     left half    v
            ? ((s.end - s.buffer) + (s.bufend - s.begin))
            : (s.end - s.begin))
        // exclude the sentinel element.
        - s.elem_size;
}

static inline char* wrap_ptr_if_necessary(char* buffer,char* p, char* bufend)
{
    return p == bufend ? buffer : p;
}

// Runs a memcpy, then returns the end of the range copied.
// Has identical functionality as mempcpy, but is portable.
static inline void* offset_memcpy(void* __restrict dest,const void* __restrict src,size_t n)
{
    memcpy(dest, src, n);
    return (char*)dest + n;
}

static size_t CONSTEXPR next_pow2(size_t n)
{
    // I don't see why we would even try. Maybe a stacktrace will help.
    assertume(n != 0);

    // In binary, top is equal to 10000...0:  A 1 right-padded by as many zeros
    // as needed to fill up a size_t.
    size_t top = (~(size_t)0 >> 1) + 1;

    // If when we round up we will overflow our size_t, avoid rounding up and
    // exit early.
    if(unlikely(n >= top))
        return n;

    // Since we don't have to worry about overflow anymore, we can just use
    // the algorithm documented at:
    //   http://bits.stephan-brumme.com/roundUpToNextPowerOfTwo.html
    // It's my favorite due to being branch-free (the loop will be unrolled),
    // and portable. However, on x86, it will be faster to use the BSR (bit-scan
    // reverse) instruction. Since this isn't straight C, it has been omitted,
    // but may be best for your platform.
    //
    // clang 3.0 is smart enough to turn this code into a bsr. gcc 4.6 isn't. I
    // haven't tested higher versions.
    n--;

    for(size_t shift = 1; shift < (sizeof n)*8; shift <<= 1)
        n |= n >> shift;

    n++;

    return n;
}

#define in_bounds(left, x, right) ((x) >= (left) && (x) <= (right))

// You know all those assumptions we make about our data structure whenever we
// use it? This function checks them, and is called liberally through the
// codebase. It would be best to read this function over, as it also acts as
// documentation. Code AND documentation? What is this witchcraft?
static inline void check_invariants(pipe_t* p)
{
    if(p == NULL) return;

    // p->buffer may be NULL. When it is, we must have no issued consumers.
    // It's just a way to save memory when we've deallocated all consumers
    // and people are still trying to push like idiots.
    if(p->buffer == NULL)
    {
        assertume(p->consumer_refcount == 0);
        return;
    }
    else
    {
        assertume(p->consumer_refcount != 0);
    }

    snapshot_t s = make_snapshot(p);

    assertume(s.begin);
    assertume(s.end);
    assertume(s.bufend);

    assertume(p->elem_size != 0);

    assertume(bytes_in_use(s) <= capacity(s)
            && "There are more elements in the buffer than its capacity.");

    assertume(in_bounds(s.buffer, s.begin, s.bufend));
    assertume(in_bounds(s.buffer, s.end, s.bufend));

    if(s.begin == s.end)
        assertume(bytes_in_use(s) == capacity(s));

    assertume(in_bounds(DEFAULT_MINCAP*p->elem_size, p->min_cap, p->max_cap));
    assertume(in_bounds(p->min_cap, capacity(s), p->max_cap));
}

static inline void lock_pipe(pipe_t* p)
{
    // watch the locking order VERY carefully. end_lock must ALWAYS be locked
    // before begin_lock when dealing with both at once.
    mutex_lock(&p->end_lock);
    mutex_lock(&p->begin_lock);
    check_invariants(p);
}

static inline void unlock_pipe(pipe_t* p)
{
    check_invariants(p);
    mutex_unlock(&p->begin_lock);
    mutex_unlock(&p->end_lock);
}

// runs some code while automatically locking and unlocking the pipe. If `break'
// is used, the pipe will be unlocked before control returns from the macro.
#define WHILE_LOCKED(stuff) do { \
    lock_pipe(p);                \
    do { stuff; } while(0);      \
    unlock_pipe(p);              \
 } while(0)

pipe_t* pipe_new(size_t elem_size, size_t limit)
{
    assertume(elem_size != 0);

    if(elem_size == 0)
        return NULL;

    pipe_t* p = malloc(sizeof *p);

    size_t cap = DEFAULT_MINCAP * elem_size;
    char*  buf = malloc(elem_size * cap);

    // Change the limit from being in "elements" to being in "bytes".
    limit *= elem_size;

    if((p == NULL) || (buf == NULL)) {
        free(p);
        free(buf);
        return NULL;
    }

    *p = (pipe_t) {
        .elem_size  = elem_size,
        .min_cap = cap,
        .max_cap = limit ? next_pow2(max(limit, cap)) : ~(size_t)0,

        .buffer = buf,
        .bufend = buf + cap + elem_size,
        .begin  = buf,
        .end    = buf + elem_size,

        // Since we're issuing a pipe_t, it counts as both a producer and a
        // consumer since it can issue new instances of both. Therefore, the
        // refcounts both start at 1; not the intuitive 0.
        .producer_refcount = 1,
        .consumer_refcount = 1,
    };

    mutex_init(&p->begin_lock);
    mutex_init(&p->end_lock);

    cond_init(&p->just_pushed);
    cond_init(&p->just_popped);

    check_invariants(p);

    return p;
}

// Instead of allocating a special handle, the pipe_*_new() functions just
// return the original pipe, cast into a user-friendly form. This saves needless
// malloc calls. Also, since we have to refcount anyways, it's free.
pipe_producer_t* pipe_producer_new(pipe_t* p)
{
    mutex_lock(&p->begin_lock);
        p->producer_refcount++;
    mutex_unlock(&p->begin_lock);

    return (pipe_producer_t*)p;
}

pipe_consumer_t* pipe_consumer_new(pipe_t* p)
{
    mutex_lock(&p->end_lock);
        p->consumer_refcount++;
    mutex_unlock(&p->end_lock);

    return (pipe_consumer_t*)p;
}

static void deallocate(pipe_t* p)
{
    assertume(p->producer_refcount == 0);
    assertume(p->consumer_refcount == 0);

    mutex_destroy(&p->begin_lock);
    mutex_destroy(&p->end_lock);

    cond_destroy(&p->just_pushed);
    cond_destroy(&p->just_popped);

    free(p->buffer);
    free(p);
}

void pipe_free(pipe_t* p)
{
    size_t new_producer_refcount,
           new_consumer_refcount;

    mutex_lock(&p->begin_lock);
        assertume(p->producer_refcount > 0);
        new_producer_refcount = --p->producer_refcount;
    mutex_unlock(&p->begin_lock);

    mutex_lock(&p->end_lock);
        assertume(p->consumer_refcount > 0);
        new_consumer_refcount = --p->consumer_refcount;
    mutex_unlock(&p->end_lock);

    if(unlikely(new_consumer_refcount == 0))
    {
        p->buffer = (free(p->buffer), NULL);

        if(likely(new_producer_refcount > 0))
            cond_broadcast(&p->just_popped);
        else
            deallocate(p);
    }
    else if(unlikely(new_producer_refcount == 0))
        cond_broadcast(&p->just_pushed);
}

void pipe_producer_free(pipe_producer_t* handle)
{
    pipe_t* p = PIPIFY(handle);
    size_t new_producer_refcount;

    mutex_lock(&p->begin_lock);
        assertume(p->producer_refcount > 0);
        new_producer_refcount = --p->producer_refcount;
    mutex_unlock(&p->begin_lock);

    if(unlikely(new_producer_refcount == 0))
    {
        size_t consumer_refcount;

        mutex_lock(&p->end_lock);
            consumer_refcount = p->consumer_refcount;
        mutex_unlock(&p->end_lock);

        // If there are still consumers, wake them up if they're waiting on
        // input from a producer. Otherwise, since we're the last handle
        // altogether, we can free the pipe.
        if(likely(consumer_refcount > 0))
            cond_broadcast(&p->just_pushed);
        else
            deallocate(p);
    }
}

void pipe_consumer_free(pipe_consumer_t* handle)
{
    pipe_t* p = PIPIFY(handle);
    size_t new_consumer_refcount;

    mutex_lock(&p->end_lock);
        new_consumer_refcount = --p->consumer_refcount;
    mutex_unlock(&p->end_lock);

    if(unlikely(new_consumer_refcount == 0))
    {
        size_t producer_refcount;

        mutex_lock(&p->begin_lock);
            producer_refcount = p->producer_refcount;
        mutex_unlock(&p->begin_lock);

        // If there are still producers, wake them up if they're waiting on
        // room to free up from a consumer. Otherwise, since we're the last
        // handle altogether, we can free the pipe.
        if(likely(producer_refcount > 0))
            cond_broadcast(&p->just_popped);
        else
            deallocate(p);
    }
}

// Returns the end of the buffer (buf + number_of_bytes_copied).
static inline char* copy_pipe_into_new_buf(snapshot_t s,
                                           char* restrict buf)
{
    if(wraps_around(s))
    {
        buf = offset_memcpy(buf, s.begin, s.bufend - s.begin);
        buf = offset_memcpy(buf, s.buffer, s.end - s.buffer);
    }
    else
    {
        buf = offset_memcpy(buf, s.begin, s.end - s.begin);
    }

    return buf;
}

// Resizes the buffer to make room for at least 'new_size' elements, returning
// an updated snapshot of the pipe state.
//
// The new size MUST be bigger than the number of elements currently in the
// pipe.
//
// The pipe must be fully locked on entrance to this function.
static snapshot_t resize_buffer(pipe_t* p, size_t new_size)
{
    check_invariants(p);

    const size_t max_cap = p->max_cap,
                 min_cap = p->min_cap,
                 elem_size = __pipe_elem_size(p);

    assertume(new_size >= bytes_in_use(make_snapshot(p)));

    if(unlikely(new_size >= max_cap))
        new_size = max_cap;

    if(new_size <= min_cap)
        return make_snapshot(p);

    char* new_buf = malloc(new_size + elem_size);
    p->end = copy_pipe_into_new_buf(make_snapshot(p), new_buf);

    p->begin  =
    p->buffer = (free(p->buffer), new_buf);

    p->bufend = new_buf + new_size + elem_size;

    check_invariants(p);

    return make_snapshot(p);
}

static inline snapshot_t validate_size_impl(pipe_t* p,
                                            snapshot_t s,
                                            size_t count,
                                            bool should_unlock)
{
    size_t elem_size    = __pipe_elem_size(p),
           new_bytes    = count*elem_size,
           cap          = capacity(s),
           bytes_needed = bytes_in_use(s) + new_bytes;

    // We add 1 to ensure p->begin != p->end unless the pipe is empty.
    if(unlikely(bytes_needed > cap))
    {
        // upgrade our lock, then re-check. By taking both locks (end and begin)
        // in order, we have an equivalent operation to lock_pipe().
        { mutex_lock(&p->begin_lock);

            s            = make_snapshot(p);
            bytes_needed = bytes_in_use(s) + new_bytes;

            if(likely(bytes_needed > cap))
                s = resize_buffer(p, next_pow2(bytes_needed));
        }

        // Unlock the pipe if requested.
        if(should_unlock)
            mutex_unlock(&p->begin_lock);
    }
    else
    {
        // If we're not unlocking, this function assures that the whole pipe is
        // locked on exit. With this condition, the pipe is locked in either
        // case.
        if(!should_unlock)
            mutex_lock(&p->begin_lock);
    }

    return s;
}

// Ensures the buffer has enough room for `count' more bytes. This function
// assumes p->end_lock is locked, and will have p->end_lock and p->begin_lock
// locked on exit - equivalent to a lock_pipe call. Returns a valid snapshot
// of the pipe.
static inline snapshot_t validate_size_locked(pipe_t* p, snapshot_t s, size_t count)
{
    return validate_size_impl(p, s, count, false);
}

// Ensures the buffer has enough room for `count' more bytes. This function
// assumes p->end_lock is locked. Returns a valid snapshot of the pipe.
static inline snapshot_t validate_size(pipe_t* p, snapshot_t s, size_t count)
{
    return validate_size_impl(p, s, count, true);
}

// Runs the actual push, assuming there is enough room in the buffer.
//
// Returns the new 'end' pointer.
static inline char* process_push(snapshot_t s,
                                const void* restrict elems,
                                size_t bytes_to_copy
                               )
{
    assertume(bytes_to_copy != 0);

    // This shouldn't be necessary.
    //s.end = wrap_ptr_if_necessary(s.buffer, s.end, s.bufend);
    assertume(s.end != s.bufend);

    // If we currently have a nowrap buffer, we may have to wrap the new
    // elements. Copy as many as we can at the end, then start copying into the
    // beginning. This basically reduces the problem to only deal with wrapped
    // buffers, which can be dealt with using a single offset_memcpy.
    if(!wraps_around(s))
    {
        size_t at_end = min(bytes_to_copy, (size_t)(s.bufend - s.end));

        s.end = offset_memcpy(s.end, elems, at_end);

        elems = (const char*)elems + at_end;
        bytes_to_copy -= at_end;
    }

    // Now copy any remaining data...
    if(unlikely(bytes_to_copy))
    {
        s.end = wrap_ptr_if_necessary(s.buffer, s.end, s.bufend);
        s.end = offset_memcpy(s.end, elems, bytes_to_copy);
    }

    s.end = wrap_ptr_if_necessary(s.buffer, s.end, s.bufend);

    // ...and update the end pointer!
    return s.end;
}

// Will spin until there is enough room in the buffer to push any elements.
// Returns the number of elements currently in the buffer. `end_lock` should be
// locked on entrance to this function.
static inline snapshot_t wait_for_room(pipe_t* p, size_t* max_cap)
{
    snapshot_t s = make_snapshot(p);

    size_t bytes_used = bytes_in_use(s);

    size_t consumer_refcount = p->consumer_refcount;

    *max_cap = p->max_cap;

    for(; unlikely(bytes_used == *max_cap) && likely(consumer_refcount > 0);
          s                 = make_snapshot(p),
          bytes_used        = bytes_in_use(s),
          consumer_refcount = p->consumer_refcount,
          *max_cap          = p->max_cap)
        cond_wait(&p->just_popped, &p->end_lock);

    return s;
}

// Peforms the actual pipe_push, but `count' is in "bytes" as opposed to
// "elements" to simplify processing.
static inline void __pipe_push(pipe_t* p,
                               const void* restrict elems,
                               size_t count)
{
    if(unlikely(count == 0))
        return;

    size_t pushed = 0;

    { mutex_lock(&p->end_lock);
        size_t max_cap;
        snapshot_t s = wait_for_room(p, &max_cap);

        // if no more consumers...
        if(unlikely(p->consumer_refcount == 0))
        {
            mutex_unlock(&p->end_lock);
            return;
        }

        s = validate_size(p, s, count);

        // Finally, we can now begin with pushing as many elements into the
        // queue as possible.
        p->end = process_push(s, elems,
                     pushed = min(count, max_cap - bytes_in_use(s)));
    } mutex_unlock(&p->end_lock);

    assertume(pushed > 0);

    // Signal if we've only pushed one element, broadcast if we've pushed more.
    if(unlikely(pushed == __pipe_elem_size(p)))
        cond_signal(&p->just_pushed);
    else
        cond_broadcast(&p->just_pushed);

    // We might not be done pushing. If the max_cap was reached, we'll need to
    // recurse.
    size_t bytes_remaining = count - pushed;

    if(unlikely(bytes_remaining))
        __pipe_push(p, (const char*)elems + pushed, bytes_remaining);
}

void pipe_push(pipe_producer_t* p, const void* restrict elems, size_t count)
{
    count *= __pipe_elem_size(PIPIFY(p));
    __pipe_push(PIPIFY(p), elems, count);
}

/*
#ifdef PIPE_DEBUG
// For testing/debugging only, and is only available in debug mode. Assuming a
// pipe of ints, prints them out. This function is not included or documented
// in the header file!

#include <stdio.h>

void pipe_debug(pipe_t* p, const char* id)
{
    printf("%s: [ ", id);
    for(int* ptr = (int*)p->buffer; ptr != (int*)p->bufend; ++ptr)
        printf("%i ", *ptr);
    printf("]\n");
    printf("begin: %lu    end: %lu\n", p->begin - p->buffer, p->end - p->buffer);
}
#endif

// This works in three phases:                  '+' = new element
//                                              '=' = old element
//                                              ' ' = empty
//
//          WRAP                        NOWRAP
//  [===>        >S========]     [>S======>            ]
//
// 1) Copy until the buffer is full, stop if we every run out of new elements.
//
//  [====+++++++>>S========]     [>S=======+++++++++++>]
//
// 2) Copy to the range [start, bufend), again, stopping when out of elements.
//    Also, move start and end appropriately. Note the NOWRAP case runs out of
//    elements before this causes issues.
//
//  [S===++++++++++++++++>>]     [++++++++>>S++++++++++]
//
// 3) Copy from bufstart to bufend, stopping early (which will definitely
//    happen).
//
//  [+++>>S++++++++++++++++]     [++++++++>>S++++++++++]
void pipe_push_clobber(pipe_producer_t* pp,
                       const void* restrict elems, size_t count)
{
    pipe_t* p = PIPIFY(pp);
    size_t elem_size = __pipe_elem_size(p);

    // We signal if pushing one element, and broadcast otherwise.
    bool should_signal = count == 1;

    count *= elem_size;

    { mutex_lock(&p->end_lock);
        snapshot_t s = make_snapshot(p);

        // if no more consumers...
        if(unlikely(p->consumer_refcount == 0))
        {
            mutex_unlock(&p->end_lock);
            return;
        }

        size_t max_cap = p->max_cap;

        const char* restrict data_left = elems;
        size_t amount_left = count;

        // If the number of elements to push is smaller than our maximum
        // capacity, only copy the last set. They'd just be overwritten anyhow.
        if(max_cap < count)
        {
            data_left += count - max_cap;
            amount_left = max_cap;
        }

        s = validate_size_locked(p, s, amount_left);
        // The pipe is now fully locked.

        // PHASE 1 - Push to capacity.

        size_t pushed;
        s.end = process_push(s, data_left,
                     pushed = min(amount_left, max_cap - bytes_in_use(s)));

        // PHASE 2 - Overwrite the beginning of the buffer with new data, until
        //           we hit bufend.

        data_left   += pushed;
        amount_left -= pushed;

        s.begin = offset_memcpy(s.begin,
                                data_left,
                                pushed = min(amount_left, (size_t)(s.bufend - s.begin)));
        s.end += pushed;

        s.begin = wrap_ptr_if_necessary(s.buffer, s.begin, s.bufend);
        s.end   = wrap_ptr_if_necessary(s.buffer, s.end,   s.bufend);

        // PHASE 3 - Overwrite from the start of the buffer to the end of the
        //           input data.
        //
        // We ensured (in validate_size_locked and the if statement above its
        // call) that we were only copying, at most, max_cap bytes and that the
        // remaining buffer is appropriately sized. Therefore, we just copy any
        // remaining data.

        data_left   += pushed;
        amount_left -= pushed;

        s.begin = offset_memcpy(s.begin, data_left, pushed = amount_left);
        s.end += pushed;

        s.begin = wrap_ptr_if_necessary(s.buffer, s.begin, s.bufend);
        s.end   = wrap_ptr_if_necessary(s.buffer, s.end,   s.bufend);

        // Reflect our snapshot back onto the pipe. We should not have modified
        // p->buffer nor p->bufend since validate_size_locked, where we got our
        // snapshot.
        p->begin = s.begin;
        p->end   = s.end;

    } unlock_pipe(p);

    if(unlikely(should_signal))
        cond_signal(&p->just_pushed);
    else
        cond_broadcast(&p->just_pushed);
}
*/

// Waits for at least one element to be in the pipe. p->begin_lock must be
// locked when entering this function, and a new, valid snapshot is returned.
static inline snapshot_t wait_for_elements(pipe_t* p)
{
    snapshot_t s = make_snapshot(p);

    size_t bytes_used = bytes_in_use(s);

    for(; unlikely(bytes_used == 0) && likely(p->producer_refcount > 0);
          s = make_snapshot(p),
          bytes_used = bytes_in_use(s))
        cond_wait(&p->just_pushed, &p->begin_lock);

    return s;
}

// wow, I didn't even intend for the name to work like that...
// returns a new snapshot, with the updated changes also reflected onto the
// pipe.
static inline snapshot_t pop_without_locking(snapshot_t s,
                                             void* restrict target,
                                             size_t bytes_to_copy,
                                             char** begin // [out]
                                            )
{
    // This wrapping shouldn't be necessary.
    //s.begin = wrap_ptr_if_necessary(s.buffer, s.begin, s.bufend);
    assertume(s.begin != s.bufend);

    size_t elem_size = s.elem_size;

    // Copy either as many bytes as requested, or the available bytes in the RHS
    // of a wrapped buffer - whichever is smaller.
    {
        size_t first_bytes_to_copy = min(bytes_to_copy, (size_t)(s.bufend - s.begin - elem_size));

        target = offset_memcpy(target, s.begin + elem_size, first_bytes_to_copy);

        bytes_to_copy -= first_bytes_to_copy;
        s.begin       += first_bytes_to_copy;

        s.begin = wrap_ptr_if_necessary(s.buffer, s.begin, s.bufend);
    }

    if(unlikely(bytes_to_copy > 0))
    {
        s.begin += elem_size;
        s.begin = wrap_ptr_if_necessary(s.buffer, s.begin, s.bufend);

        memcpy(target, s.begin, bytes_to_copy);
        s.begin += bytes_to_copy;

        s.begin -= elem_size;

        s.begin = wrap_ptr_if_necessary(s.buffer, s.begin, s.bufend);
    }

    // Since we cached begin on the stack, we need to reflect our changes back
    // on the pipe.
    *begin = s.begin;

    return s;
}

// If the buffer is shrunk to something a lot smaller than our current
// capacity, resize it to something sane. This function must be entered with
// only p->begin_lock locked, and will automatically unlock p->begin_lock on
// exit.
static inline void trim_buffer(pipe_t* p, snapshot_t s)
{
    size_t cap = capacity(s);

    // We have a sane size. We're done here.
    if(likely(bytes_in_use(s) > cap / 4))
    {
        mutex_unlock(&p->begin_lock);
        return;
    }

    // Okay, we need to resize now. Upgrade our lock so we can check again. The
    // weird lock/unlock order is to make sure we always acquire the end_lock
    // before begin_lock. Deadlock can arise otherwise.
    mutex_unlock(&p->begin_lock);
    mutex_lock(&p->end_lock);
    mutex_lock(&p->begin_lock);

    s   = make_snapshot(p);
    cap = capacity(s);

    // To conserve space like the good computizens we are, we'll shrink
    // our buffer if our memory usage efficiency drops below 25%. However,
    // since shrinking/growing the buffer is the most expensive part of a push
    // or pop, we only shrink it to bring us up to a 50% efficiency. A common
    // pipe usage pattern is sudden bursts of pushes and pops. This ensures it
    // doesn't get too time-inefficient.
    if(likely(bytes_in_use(s) <= cap / 4))
        resize_buffer(p, cap / 2);

    // All done. Unlock the pipe. The reason we don't let the calling function
    // unlock begin_lock is so that we can do it BEFORE end_lock. This prevents
    // the lock order from being fudged.
    mutex_unlock(&p->begin_lock);
    mutex_unlock(&p->end_lock);
}

// Performs the actual pop, except `requested' is now in bytes as opposed to
// elements.
//
// This will behave eagerly, returning as many elements that it can into
// `target' as it can fill right now.
static inline size_t __pipe_pop(pipe_t* p,
                                void* restrict target,
                                size_t requested)
{
    if(unlikely(requested == 0))
        return 0;

    size_t popped = 0;

    { mutex_lock(&p->begin_lock);
        snapshot_t s      = wait_for_elements(p);
        size_t bytes_used = bytes_in_use(s);

        if(unlikely(bytes_used == 0))
        {
            mutex_unlock(&p->begin_lock);
            return 0;
        }

        s = pop_without_locking(s, target,
                                popped = min(requested, bytes_used),
                                &p->begin
        );

        trim_buffer(p, s);
    } // p->begin_lock was unlocked by trim_buffer.

    assertume(popped);

    if(unlikely(popped == __pipe_elem_size(p)))
        cond_signal(&p->just_popped);
    else
        cond_broadcast(&p->just_popped);

    return popped;
}

size_t pipe_pop(pipe_consumer_t* p, void* target, size_t count)
{
    size_t elem_size = __pipe_elem_size(PIPIFY(p));

    size_t bytes_left  = count*elem_size;
    size_t bytes_popped = 0;
    size_t ret = -1;

    do {
        ret = __pipe_pop(PIPIFY(p), target, bytes_left);
        target = (void*)((char*)target + ret);
        bytes_popped += ret;
        bytes_left   -= ret;
    } while(ret != 0 && bytes_left);

    return bytes_popped / elem_size;
}

size_t pipe_pop_eager(pipe_consumer_t* p, void* target, size_t count)
{
    size_t elem_size = __pipe_elem_size(PIPIFY(p));
    return __pipe_pop(PIPIFY(p), target, count*elem_size) / elem_size;
}

void pipe_reserve(pipe_generic_t* gen, size_t count)
{
    pipe_t* p = PIPIFY(gen);

    count *= __pipe_elem_size(p); // now `count' is in "bytes" instead of "elements".

    if(count == 0)
        count = DEFAULT_MINCAP;

    size_t max_cap = p->max_cap;

    WHILE_LOCKED(
        if(unlikely(count <= bytes_in_use(make_snapshot(p))))
            break;

        p->min_cap = min(count, max_cap);
        resize_buffer(p, count);
    );
}

/* vim: set et ts=4 sw=4 softtabstop=4 textwidth=80: */
