/*
 * Look for correlations in random number streams.
 * Also we "optionally" save the output so you can test repeatability.
 *
 * Save is optional in the sense that if you compile with a nonempty string as
 * the value of macro symbol SAVE_FILENAME, the output will be saved to that
 * filename.  Otherwise, nothing is saved.
 *
 * Other "optional" features:  you can set the number of threads via macro
 * symbol THREAD_CT.  You can set the number of stream samples from each
 * thread via macro symbol STREAM_CT.  The output size is proportional to the
 * product of these values.  They each have defaults as you can see below.
 *
 * $Id: test_random_streams.c 21491 2017-07-20 13:19:02Z kobus $
 */
#include "l/l_sys_io.h"
#include "l/l_sys_debug.h"
#include "l/l_sys_rand.h"
#include "l/l_sys_mal.h"
#include "l/l_sys_term.h"
#include "l/l_init.h"
#include "l/l_error.h"
#include "l/l_debug.h"
#include "l/l_sort.h"
#include "l/l_global.h"
#include "l/l_int_matrix.h"
#include "l_mt/l_mt_pthread.h"
#include "l_mt/l_mt_util.h"

/* You can increase THREAD_CT or STREAM_CT.  Biggest test so far:
   THREAD_CT of 100, STREAM_CT of 1000000 (a million).
   All outputs are PAIRWISE UNIQUE, when tested on v11 and bayes01.
   That means at least the first million outputs of kjb_rand() and kjb_rand_2()
   for 100 threads contain no duplications at all.  Hopefully it's adequate!

   If you want to save the output, to demonstrate repeatability, you can put
   a filename in SAVE_FILENAME.  Results seem to be nicely repeatable.

   The above big test takes about 5 to 7 minutes on those machines.
   Testing for pairwise uniqueness is the main magic of this program.
   Doing so efficiently (via efficient sorting) is why it is SOOOOO long!
   An early, short version (like in SVN revision r15630) was quadratic time
   and would have taken six weeks to accomplish the same test.
 */

#ifndef THREAD_CT
#define THREAD_CT 10      /* number of threads to launch */
#endif

#ifndef STREAM_CT
#define STREAM_CT 1000    /* number of samples from each PRNG of each thread */
#endif

#ifndef SAVE_FILENAME
#define SAVE_FILENAME ""  /* Empty string here means "do not save." */
#endif

#define TEST_SLS 0        /* Internal test of S_list -- normally set to zero */

#define ERROR_INJECTION 0 /* Want to inject seed duplication? 1=yes, 0=no. */

#define SEED_PAIR_SIZE 6  /* do not adjust; see kjb_pthread_read_prng_seeds */



struct Seed
{
    int lag, stream_id; /* stream id LSB: 0==kjb_rand, 1==kjb_rand_2. */
                        /* thread id occupies the upper bits. */
    kjb_uint16 s[SEED_PAIR_SIZE/2]; /* seed of just one stream, not a pair */
};

#define BLOCK_BSIZE 12
#define BLOCK_SIZE (1 << BLOCK_BSIZE) /* 4096 */

struct S_block
{
    struct Seed b[BLOCK_SIZE];
    struct S_block* next;
};


struct S_list
{
    struct S_block *head, *tail, *reserve;
    int total_size, tail_size;
};



/* macro to test whether a list pointer (not object!) contains elements */
#define sls_not_empty(p) ((p) -> total_size)
#define sls_empty(p) (0 == (p) -> total_size)


static void db_print_seed(const struct Seed* s)
{
    if (NULL == s)
    {
        TEST_PSE(("null value passed to %s:%d\n", __FILE__, __LINE__));
        return;
    }
    TEST_PSE(("Seed value %u,%u,%u at lag %d, thread %d, stream %d\n",
                s -> s[0], s -> s[1], s -> s[2], s -> lag,
                s -> stream_id >> 1, s -> stream_id & 1));
}

static int seed_cmp(const void* s1, const void* s2)
{
    int i;
    const kjb_uint16 *p, *q;
    ASSERT(s1);
    ASSERT(s2);
    p = ((const struct Seed*) s1) -> s;
    q = ((const struct Seed*) s2) -> s;
    for (i = 0; i < SEED_PAIR_SIZE/2; ++i, ++p, ++q)
    {
        if (*p != *q) return *p - *q;
    }
    return 0;
}


#if TEST_SLS
static void db_print_sblock(const struct S_block* b)
{
    int i;
    if (NULL == b)
    {
        TEST_PSE(("null ptr passed to %s:%d.\n", __FILE__, __LINE__));
        return;
    }
    TEST_PSE(("Sblock at pointer %p.  (Next block at %p)\n", b, b -> next));
    for (i = 0; i < BLOCK_SIZE; ++i)
    {
        TEST_PSE(("%d: ", i));
        db_print_seed(b -> b + i);
    }
}
#endif

static int ow_sort_sblock(struct S_block* b, size_t size)
{
    NRE(b);
    size = MIN_OF(size, BLOCK_SIZE); /* trim size to something valid */
    return kjb_sort(b -> b, size, sizeof(struct Seed), &seed_cmp,
                                                USE_CURRENT_ATN_HANDLING);
}

static int sls_ctor(struct S_list* d)
{
    NRE(d);
    d -> head = d -> tail = d -> reserve = NULL;
    d -> total_size = d -> tail_size = 0;
    return NO_ERROR;
}

#if TEST_SLS
static void db_print_sls(const struct S_list* d)
{
    const struct S_block *p, *q;
    if (NULL == d)
    {
        TEST_PSE(("null ptr passed to %s:%d.\n", __FILE__, __LINE__));
        return;
    }
    TEST_PSE(("Total size: %d.  Tail size: %d.  Reserve ptr: %p.\n",
                d -> total_size, d -> tail_size, d -> reserve));
    if (sls_empty(d))
    {
        TEST_PSE(("Empty list.  head pointer equals %p.\n", d -> head));
        return;
    }

    TEST_PSE(("S_list begin ==============\n"));
    for (q = NULL, p = d -> head; q != d -> tail; q = p, p = p -> next)
    {
        TEST_PSE(("S_list next block = = = = =\n"));
        if (p == d -> tail)
        {
            TEST_PSE(("NB: only %d records are valid.\n", d -> tail_size));
        }
        db_print_sblock(p);
    }
    TEST_PSE(("S_list end --------------\n"));
}
#endif

static int sls_get_reserve(struct S_list* d)
{
    NRE(d);
    if (NULL == d -> reserve)
    {
        /* must use multi-threaded version here! */
        NRE(d -> reserve
                = (struct S_block*) kjb_mt_malloc(sizeof(struct S_block)));
        d -> reserve -> next = NULL;
    }
    return NO_ERROR;
}


static int sls_push_back(struct S_list* d, const struct Seed* s)
{
    NRE(d);
    if (sls_not_empty(d) && d -> tail_size < BLOCK_SIZE)
    {
        /* common case:  space in tail block */
        ASSERT(d -> head && d -> tail);
        d -> tail -> b[ d->tail_size ++ ] = *s;
    }
    else /* empty list or append a new block */
    {
        ERE(sls_get_reserve(d));

        *(sls_not_empty(d) ? & d -> tail -> next : & d -> head) = d -> reserve;

        d -> tail = d -> reserve;
        d -> reserve = d -> reserve -> next;
        d -> tail -> next = NULL;
        d -> tail -> b[0] = *s;
        d -> tail_size = 1;
    }
    d -> total_size += 1;
    return NO_ERROR;
}


static int sls_pop_back(struct S_list* d)
{
    NRE(d);
    if (sls_empty(d))
    {
        set_error("tried to pop the rear element of an empty list.");
        NOTE_ERROR();
        return ERROR;
    }
    ASSERT(d -> tail_size >= 1);
    if (1 == d -> tail_size) /* must free a block */
    {
        ASSERT(sls_not_empty(d));
        if (d -> total_size > 1)
        {
            set_error("not implemented for multiblock lists");
            NOTE_ERROR();
            return ERROR;
        }
        d -> head -> next = d -> reserve;
        d -> reserve = d -> head;
        d -> head = d -> tail = NULL;
        d -> total_size = 0;
    }
    else
    {
        d -> tail_size -= 1;
        d -> total_size -= 1;
    }
    return NO_ERROR;
}

static int sls_append_block(struct S_list* d, struct S_block* b, size_t size)
{
    NRE(d);
    NRE(b);
    size = MIN_OF(size, BLOCK_SIZE);
    if (sls_not_empty(d) && d -> tail_size < BLOCK_SIZE)
    {
        size_t i;
        TEST_PSE(("slow append block!"));
        for (i = 0; i < size; ++i)
        {
            ERE(sls_push_back(d, b -> b + i));
        }
        b -> next = d -> reserve;
        d -> reserve = b;
    }
    else
    {
        *(sls_empty(d) ? & d -> head : & d -> tail -> next) = b;
        d -> tail = b;
        b -> next = NULL;
        d -> tail_size = size;
        d -> total_size += size;
    }
    return NO_ERROR;
}

/* this clears the data part and puts all the nodes in the reserve list. */
static int sls_clear(struct S_list* l)
{
    NRE(l);
    if (sls_not_empty(l))
    {
        ASSERT(l -> head);
        ASSERT(l -> tail);
        ASSERT(NULL == l -> tail -> next);
        l -> tail -> next = l -> reserve;
        l -> reserve = l -> head;
        l -> head = l -> tail = NULL;
        l -> total_size = l -> tail_size = 0;
    }
    return NO_ERROR;
}

static void sls_destroy(struct S_list* d)
{
    if (NULL == d) return;

    sls_clear(d);

    while (d -> reserve)
    {
        struct S_block* p = d -> reserve;
        d -> reserve = d -> reserve -> next;
        kjb_mt_free(p); /* actually we do not need the multithread version,
                           because sls_destroy() is not called by threads, but
                           let's use it anyway in an abundance of caution. */
    }
    d -> reserve = NULL;
}

static int sls_pop_front(struct S_list* l, size_t* skip)
{
    NRE(l);
    NRE(skip);
    if (sls_empty(l))
    {
        set_error("cannot pop from empty list");
        NOTE_ERROR();
        return ERROR;
    }
    if (l -> tail_size > l -> total_size)
    {
        set_error("broken invariant: list total_size %d, tail_size %d",
                       l -> total_size, l -> tail_size);
        NOTE_ERROR();
        return ERROR;
    }
    if ((int) *skip >= l -> total_size || (int) *skip >= BLOCK_SIZE)
    {
        set_error("skip size %d exceeds list total_size %d or block size %d",
                  *skip, l -> total_size, BLOCK_SIZE);
        NOTE_ERROR();
        return ERROR;
    }

    /* Pop by advancing the skip counter. */
    *skip += 1;

    /* Check to see if we need to discard a block (uncommon case). */
    if (l -> head == l -> tail)
    {
        /* list down to its last block */
        if (l -> tail_size != l -> total_size || (int) *skip > l -> tail_size)
        {
            set_error("bad pop_front request:  list total_size %d, tail_size "
               "%d, and skip size %d", l -> total_size, l -> tail_size, *skip);
            NOTE_ERROR();
            *skip -= 1;
            return ERROR;
        }
        if ((int) *skip == l -> tail_size)
        {
            l -> head -> next = l -> reserve;
            l -> reserve = l -> head;
            l -> head = l -> tail = NULL;
            l -> total_size = 0;
        }
    }
    else if (BLOCK_SIZE == (int) *skip)
    {
        /* time to discard a non-last block */
        struct S_block* b = l -> head;
        l -> head = l -> head -> next;
        l -> total_size -= BLOCK_SIZE;
        *skip = 0; /* set skip counter to indicate front of next block */
        b -> next = l -> reserve;
        l -> reserve = b;
    }
    return NO_ERROR;
}


/* If donor has any reserve blocks, he gives them to rx (recipient).
   This is idempotent:  if donor has none, nothing happens.
   The purpose is to reduce calls to malloc -- you might have foreknowledge
   that donor is shrinking and rx is growing.
   Return value is NO_ERROR unless you pass in a pointer equal to NULL. */
static int sls_xfer_reserve_from_to(struct S_list* donor, struct S_list* rx)
{
    NRE(donor);
    NRE(rx); /* recipient */

    while (donor -> reserve)
    {
        struct S_block* b = donor -> reserve;
        donor -> reserve = b -> next;
        b -> next = rx -> reserve;
        rx -> reserve = b;
    }
    return NO_ERROR;
}


/* this is a DESTRUCTIVE merge:  lists i1, i2 are emptied, to fill list o. */
static int sls_merge(struct S_list* i1, struct S_list* i2, struct S_list* o)
{
    size_t skip1 = 0, skip2 = 0;
    NRE(i1);
    NRE(i2);
    NRE(o);
    if (sls_not_empty(o))
    {
        set_error("output list was supposed to be empty");
        NOTE_ERROR();
        return ERROR;
    }
    while (sls_not_empty(i1) && sls_not_empty(i2))
    {
        int r = seed_cmp(i1 -> head -> b + skip1, i2 -> head -> b + skip2);
        if (r <= 0)
        {
            if (0 == r)
            {
                TEST_PSE(("Duplicate found!\n"));
                db_print_seed(i1 -> head -> b + skip1);
                db_print_seed(i2 -> head -> b + skip2);
            }
            ERE(sls_push_back(o, i1 -> head -> b + skip1));
            ERE(sls_pop_front(i1, &skip1));
            ERE(sls_xfer_reserve_from_to(i1, o));
        }
        else
        {
            ERE(sls_push_back(o, i2 -> head -> b + skip2));
            ERE(sls_pop_front(i2, &skip2));
            ERE(sls_xfer_reserve_from_to(i2, o));
        }
    }
    while (sls_not_empty(i1))
    {
        ERE(sls_push_back(o, i1 -> head -> b + skip1));
        ERE(sls_pop_front(i1, &skip1));
        ERE(sls_xfer_reserve_from_to(i1, o));
    }
    while (sls_not_empty(i2))
    {
        ERE(sls_push_back(o, i2 -> head -> b + skip2));
        ERE(sls_pop_front(i2, &skip2));
        ERE(sls_xfer_reserve_from_to(i2, o));
    }
    return NO_ERROR;
}

/* merge sort for linked lists */
static int sls_sort(struct S_list* d)
{
    int i, result = ERROR;
    struct S_list l[2];
    NRE(d);
    /* base case of recursion:  one block */
    if (d -> total_size <= BLOCK_SIZE)
    {
        return ow_sort_sblock(d -> head, d -> total_size);
    }

    /* recursive case, step 1: divide input into two half-size lists */
    for (i = 0; i < 2; ++i)
    {
        ERE(sls_ctor(l + i));
    }
    for (i = 0; sls_not_empty(d); i = 1 - i)
    {
        size_t sz;
        struct S_block* p = d -> head;
        ASSERT(p);
        if (d -> head == d -> tail)
        {
            sz = d -> tail_size;
            ASSERT((int) sz == d -> total_size);
            ASSERT(NULL == d -> head -> next);
            d -> tail = NULL;
        }
        else
        {
            sz = BLOCK_SIZE;
        }
        d -> head = d -> head -> next;
        d -> total_size -= sz;
        EGC(sls_append_block(l + i, p, sz));
    }

    /* recursive case step 2:  sort the mini lists */
    for (i = 0; i < 2; ++i)
    {
        EGC(sls_sort(l+i));
    }
    /* recursive case step 3:  merge the mini lists */
    EGC(sls_merge(l, l+1, d));
    result = NO_ERROR;

cleanup:
    for (i = 0; i < 2; ++i)
    {
        sls_destroy(l + i);
    }
    return result;
}

static int is_interacto(void)
{
    int ii, len;
    if ((len = kjb_get_strlen_error()))
    {
        char buf[4096];
        ASSERT(len < (int) sizeof buf);
        kjb_get_error(buf, sizeof buf);
        ii = is_interactive();
        set_error("%s", buf);
    }
    else
    {
        ii = is_interactive();
        kjb_clear_error();
    }
    return ii;
}

/* return value is undefined if d does not contain an element. */
static struct Seed sls_back(struct S_list* d)
{
    struct Seed seed;
    if (d && sls_not_empty(d))
    {
        ASSERT(d -> tail_size);
        return d -> tail -> b[d -> tail_size - 1];
    }
    return seed;
}

/* function called by many threads -- must be reentrant */
static void* work_fun(void* arg)
{
    int i;
    struct S_list* l = (struct S_list*) arg;
    kjb_uint16 b[SEED_PAIR_SIZE];
    struct Seed seed1, seed2;
    NRN(l);
    ASSERT(1 == l -> total_size);
    seed1 = seed2 = sls_back(l);
    ERN(sls_pop_back(l));
    seed2.stream_id |= 1; /* low bit indicates kjb_rand() or kjb_rand_2() */

    /* Verify correctness of stream_id.  The off-by-one term is because the
       creation numbering starts from zero; whereas the wrapper considers the
       main thread to be thread zero, and numbers the other starting with 1.
       =================
       NOTE: if this error message IS ever seen in practice or in regression
       testing, that will be strong evidence that the order of calls to
       kjb_pthread_create can occur in a different order than the internals
       of the implementation finish.  In other words, that test_thread_id
       (another test in this directory) is NOT a waste of time.  If that
       occurs, you should only trust get_kjb_pthread_number() to help you tell
       which random stream is which, because it will be repeatable even if
       the creation numbering is not.  Also in that case you should revise
       this test not to bother receiving a thread number from the S_list, but
       instead just get it from get_kjb_pthread_number().

       If this error message is NEVER seen, then it does not matter where you
       get the pthread numbering from.  The weird stuff I saw earlier must
       have been due to some other unexplained bug.
     */
    if (1 + (seed1.stream_id >> 1) != get_kjb_pthread_number())
    {
        set_error("Thread ID is number %d from creation, %d to wrapper.",
                    seed1.stream_id >> 1, get_kjb_pthread_number());
        NOTE_ERROR();
        return NULL;
    }

    /* Main loop: read each random stream a lot, and store in a queue. */
    for (i = 0; i < STREAM_CT; ++i, kjb_rand(), kjb_rand_2())
    {
        int k;
        ERN(kjb_pthread_read_prng_seeds(b, SEED_PAIR_SIZE));
        seed1.lag = seed2.lag = i;
        for (k = 0; k < SEED_PAIR_SIZE/2; ++k)
        {
            seed1.s[k] = b[k];
            seed2.s[k] = b[k+SEED_PAIR_SIZE/2];
        }
        sls_push_back(l, &seed1);
        sls_push_back(l, &seed2);
    }
    return arg;
}



static int fp_write_sblock(const struct S_block* b, size_t size, FILE* fp)
{
    size_t j;
    NRE(b);
    NRE(fp);
    for (j = 0; j < size; ++j)
    {
        long ct;
        NRE(b -> b);
        ct = kjb_fwrite(fp, b -> b[j].s, sizeof b -> b[j].s);
        if (ERROR == ct || ct != (int) sizeof b -> b[j].s)
        {
            int tid = b -> b[0].stream_id >> 1;
            add_error("bad write %ld, for thread %d, output %u", ct, tid, j);
            return ERROR;
        }
    }
    return NO_ERROR;
}


static int maybe_save(struct S_list stream[])
{
    int i, result = ERROR;
    FILE* fp = NULL;
    const char* fn = SAVE_FILENAME;

    if (NULL == fn || 0 == strlen(fn)) return NO_ERROR; /* trivial case */

    NRE(stream);
    NRE(fp = kjb_fopen(fn, "w"));
    for (i = 0; i < THREAD_CT; ++i)
    {
        const struct S_block* b;
        if (sls_empty(stream + i))
        {
            set_error("Why is list %d empty?", i);
            NOTE_ERROR();
            goto cleanup;
        }
        NGC(stream[i].head);
        ASSERT(stream[i].head -> b[0].stream_id >> 1 == i);
        for (b = stream[i].head; b != stream[i].tail; b = b -> next)
        {
            EGC(fp_write_sblock(b, BLOCK_SIZE, fp));
        }
        ASSERT(b == stream[i].tail);
        EGC(fp_write_sblock(b, stream[i].tail_size, fp));
    }
    result = NO_ERROR;

cleanup:
    kjb_fclose(fp);
    return result;
}



#if TEST_SLS
static int test_sls(void)
{
    int i;
    struct S_list l;
    struct Seed *p, tab[] = {
        {       0,      0,      {   20, 37, 40  } },
        {       0,      1,      {   20, 30, 40  } },
        {       0,      0,      {   20, 32, 40  } },
        {       0,      1,      {   20, 36, 40  } },
        {       0,      0,      {   20, 33, 40  } },
        {       0,      1,      {   20, 38, 40  } },
        {       0,      0,      {   20, 31, 40  } },
        {       0,      1,      {   20, 35, 40  } },
        {       0,      0,      {   20, 34, 40  } },
        {       0,      1,      {   20, 39, 40  } },
        {       -1,     0,      {   0,  0,  0   } } /* sentinel */
    };
    l.total_size = 0;
    l.head = l.tail = l.reserve = NULL;

#if 0 /* mini, trivial test */
    for (p = tab; p -> lag >= 0; ++p)
    {
        EPETE(sls_push_back(&l, p));
    }
#else /* more serious test */
    for (i = 0; i < 10000; ++i)
    {
        struct Seed s;
        s.lag = kjb_rand() * 100;
        s.stream_id = kjb_rand() * 100;
        s.s[0] = kjb_rand() * 100;
        s.s[1] = kjb_rand() * 100;
        s.s[2] = kjb_rand() * 100;
        EPETE(sls_push_back(&l, &s));
    }
#endif
    kjb_disable_paging();
    db_print_sls(&l);
    ERE(sls_sort(&l));
    TEST_PSE(("................. sorted? .................\n"));
    db_print_sls(&l);
    sls_destroy(&l);
    return EXIT_SUCCESS;
}
#endif


static int sls_swap(struct S_list* p, struct S_list* q)
{
    struct S_list temp;
    NRE(p);
    NRE(q);
    temp = *p;
    *p = *q;
    *q = temp;
    return NO_ERROR;
}

/* precondition: the input array must contain 'ct' lists, with 'ct' > 0.
   precondition: each list of the input array must be sorted and nonempty.
   precondition: the output list must be in a valid state.

   postcondition: previous contents of output list are lost.
   postcondition: all records of input array are copied to output list.
   postcondition: output list is in sorted order.
   postcondition: each list of input array is empty.
 */
static int multiway_destructive_merge(
    struct S_list in[],
    size_t ct,
    struct S_list* out
)
{
    size_t iix, oix; /* input and output indices */
    NRE(in);
    NRE(out);
    if (0 == ct)
    {
        set_error("zero length input");
        NOTE_ERROR();
        return ERROR;
    }
    if (1 == ct)
    {
        return sls_swap(in, out);
    }

    /* 2 or more inputs */
    ERE(sls_clear(out));
    ERE(sls_merge(in, in+1, out));
    if (2 == ct) return NO_ERROR; /* 2 inputs is also a base case */

    /* 3 or more inputs:  get recursive */
    ERE(sls_swap(in, out));
    ASSERT(sls_empty(out));
    for (oix = 1, iix = 3; iix < ct; oix += 1, iix += 2)
    {
        ERE(sls_merge(in + iix - 1, in + iix, in + oix));
    }
    /* if odd size, last input list just gets shifted */
    if (ct & 1)
    {
        ERE(sls_swap(in + ct - 1, in + ct/2));
    }
    /* tail recursion: list is now about half the size, so keep merging. */
    return multiway_destructive_merge(in, (1+ct)/2, out);
}

static int scan_seeds_for_duplicates(
    const struct Seed* r,
    const struct Seed* s
)
{
    int x;
    NRE(r);
    NRE(s);

    x = seed_cmp(r, s);

    if (x > 0)
    {
        set_error("Seeds are not sorted:\n");
        db_print_seed(r);
        db_print_seed(s);
        return ERROR;
    }

    if (0 == x)
    {
        kjb_printf("Seed duplication detected.\n");
        db_print_seed(r);
        db_print_seed(s);
    }
    return NO_ERROR;
}


/* look for duplicates within a sorted block, and if found, print them. */
static int scan_block_for_duplicates(const struct S_block* b, size_t size)
{
    size_t i;
    NRE(b);
    for (i = 1; i < size; ++i)
    {
        ERE(scan_seeds_for_duplicates(b -> b + i - 1, b -> b + i));
    }
    return NO_ERROR;
}

static int validate(struct S_list stream[])
{
    int i;
    struct S_list biglist;
    const struct S_block *b0, *b1;
    int result = ERROR;

    if (is_interacto()) TEST_PSE(("Sorting threads' outputs (1 of 2).\n"));

    /* sort each stream, then merge sorted streams */
    ERE(sls_ctor(&biglist));
    for (i = 0; i < THREAD_CT; ++i)
    {
        ERE(sls_sort(stream + i));
    }
    if (is_interacto()) TEST_PSE(("Sorting threads' outputs (2 of 2).\n"));
    ERE(multiway_destructive_merge(stream, THREAD_CT, &biglist));

    /* sanity checking -- factor 2 is because there are 2 prngs per thread */
    if (biglist.total_size != 2 * THREAD_CT * STREAM_CT)
    {
        set_error("big list has size %d, expecting size %d.",
                    biglist.total_size, 2 * THREAD_CT * STREAM_CT);
        NOTE_ERROR();
        goto cleanup;
    }
    NGC(biglist.head);

    /* scan the big list for duplicates (which will be sequential). */
    if (is_interacto()) TEST_PSE(("Scan for duplicates.\n"));
    for (b0=NULL, b1=biglist.head; b1 != biglist.tail; b0=b1, b1 = b1 -> next)
    {
        EGC(scan_block_for_duplicates(b1, BLOCK_SIZE));
        /* don't forget to check for a match between blocks. */
        if (b0) EGC(scan_seeds_for_duplicates(b0->b + BLOCK_SIZE-1, b1->b));
    }

    /* scan the final block */
    ASSERT(biglist.tail == b1);
    EGC(scan_block_for_duplicates(biglist.tail, biglist.tail_size));
    ASSERT(NULL == b0 || b0 -> next == b1);
    if (b0) EGC(scan_seeds_for_duplicates(b0->b + BLOCK_SIZE-1, b1->b));
    result = NO_ERROR;

cleanup:
    sls_destroy(&biglist);
    return result;
}


static int test_thread_prngs(void)
{
    int i;
    int status = EXIT_FAILURE;
    struct S_list thread_out_arr[THREAD_CT];
    kjb_pthread_t tid[THREAD_CT];

    EPETE(kjb_init()); /* no EGC allowed yet */

    if (is_interacto()) TEST_PSE(("Starting threads and sampling PRNGs.\n"));

    /* generate empty seed lists */
    for (i = 0; i < THREAD_CT; ++i)
    {
        EPETE(sls_ctor(thread_out_arr + i)); /* no EGC allowed yet */
    }

    /* push on a thread-id.  This is a bit clunky but not a bottleneck. */
    for (i = 0; i < THREAD_CT; ++i)
    {
        struct Seed seed;
        seed.stream_id = i << 1;
        EGC(sls_push_back(thread_out_arr + i, &seed));
    }

    /* generate threads, each to fill one list */
    for (i = 0; i < THREAD_CT; ++i)
    {
        EGC(kjb_pthread_create(tid+i, NULL, &work_fun, thread_out_arr + i));
    }

    /* join all threads and verify that each died happy */
    for (i = 0; i < THREAD_CT; ++i)
    {
        void* p;
        EGC(kjb_pthread_join(tid[i], &p));
        NGC(p);
    }

#if ERROR_INJECTION
    do
    {
        struct Seed *r = thread_out_arr[ 0].head -> b + 0,
                    *s = thread_out_arr[39].head -> b + 17;

        /* lag/stream relationship to b offset:  b[ 2*lag | stream ].
           e.g., 17 = 2*8 | 1.
         */
        TEST_PSE(("NOTE: DELIBERATE ERROR INJECTION HAS BEEN ENABLED.\n"
                  "Expect duplication between thread 0, lag 0, stream 0 "
                  "and thread 39, lag 8, stream 1.\n"));
        r -> s[0] = s -> s[0];
        r -> s[1] = s -> s[1];
        r -> s[2] = s -> s[2];
    }
    while(0);
#endif


    /* maybe save results, and validate */
    EGC(maybe_save(thread_out_arr));
    EGC(validate(thread_out_arr)); /* this destroys the contents of array! */
    status = EXIT_SUCCESS;

cleanup:
    for (i = 0; i < THREAD_CT; ++i)
    {
        sls_destroy(thread_out_arr + i);
    }

    if (kjb_get_strlen_error() && is_interacto()) kjb_print_error();
    kjb_cleanup();

    return status;
}



int main(void)
{

    /* Kobus; 17/07/04 
     * We used to in general exit with EXIT_FAILURE if something was wrong, but
     * now we want to exit with EXIT_BUG. However, it seems possible that the
     * previous routine could have existed with ERROR which is wrong. For now,
     * convert all ERROR returns to bug exit, but this program could use some
     * work!
    */
#if TEST_SLS
    return test_sls();

#else
    return test_thread_prngs();

#endif
}

