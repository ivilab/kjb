/*
 * This program exercises a sort function (like kjb_sort).
 * It does not really test it in the conventional sense.
 * However, if you run this program using valgrind, it has detected
 * erroneous behavior in kjb_sort -- specifically, calling memcpy with
 * overlapping source and destination regions.
 *
 * It's a snipped from a longer test program that is or was in lib/l_mt/test
 * and ended up here only because, as a byproduct, it also found an error
 * in the (old) sort routine.
 *
 * $Id: break_sort.c 15637 2013-10-10 20:37:32Z predoehl $
 */
#include <l/l_sys_io.h>
#include <l/l_sys_debug.h>
#include <l/l_sys_mal.h>
#include <l/l_sys_term.h>
#include <l/l_error.h>
#include <l/l_debug.h>
#include <l/l_sort.h>
#include <l/l_global.h>


#define SEED_PAIR_SIZE 6


struct Seed
{
    int lag, stream_id; /* stream id LSB: 0==kjb_rand, 1==kjb_rand_2. */
						/* thread id occupies the upper bits. */
    kjb_uint16 s[SEED_PAIR_SIZE/2]; /* seed of just one stream, not a pair */
};

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

#define BLOCK_BSIZE 3 /* 12 */
#define BLOCK_SIZE (1 << BLOCK_BSIZE) /* 4096 */

struct S_block
{
    struct Seed b[BLOCK_SIZE];
    struct S_block* next;
};

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

static int ow_sort_sblock(struct S_block* b, size_t size)
{
    NRE(b);
    size = MIN_OF(size, BLOCK_SIZE); /* trim size to something valid */
#if 1
    return kjb_sort(b->b, size, sizeof(struct Seed), &seed_cmp, 
                                                USE_CURRENT_ATN_HANDLING);
#else
    qsort(b->b, size, sizeof(struct Seed), &seed_cmp);
    return NO_ERROR;
#endif
}

struct S_list
{
    struct S_block *head, *tail;
    int total_size, tail_size;
};

static void db_print_sls(const struct S_list* d)
{
    const struct S_block *p, *q;
    if (NULL == d)
    {
        TEST_PSE(("null ptr passed to %s:%d.\n", __FILE__, __LINE__));
        return;
    }
    TEST_PSE(("Total size: %d.  Tail size: %d.\n",
                d -> total_size, d -> tail_size));
    if (0 == d -> total_size)
    {
        TEST_PSE(("Empty list.  head pointer equals %p.\n", d -> head));
        return;
    }

    TEST_PSE(("S_list begin ==============\n"));
    for (q = NULL, p = d -> head; q != d -> tail; q = p, p = p -> next)
    {
        TEST_PSE(("S_list next block = = = = =\n"));
        db_print_sblock(p);
    }
    TEST_PSE(("S_list end --------------\n"));
}

static int sls_push_back(struct S_list* d, const struct Seed* s)
{
    NRE(d);
    if (0 == d -> total_size) /* empty list */
    {
        NRE(d -> head = d -> tail = TYPE_MALLOC(struct S_block));
        d -> tail -> next = NULL;
        d -> tail -> b[0] = *s;
        d -> tail_size = 1;
    }
    else if (d -> tail_size < BLOCK_SIZE) /* space in tail block */
    {
        ASSERT(d -> head && d -> tail);
        d -> tail -> b[ d->tail_size ++ ] = *s;
    }
    else /* append a new block */
    {
        NRE(d -> tail -> next = TYPE_MALLOC(struct S_block));
        d -> tail = d -> tail -> next;
        d -> tail -> next = NULL;
        d -> tail -> b[0] = *s;
        d -> tail_size = 1;
    }
    d -> total_size += 1;
    return NO_ERROR;
}

static int sls_append_block(struct S_list* d, struct S_block* b, size_t size)
{
    NRE(d);
    NRE(b);
    size = MIN_OF(size, BLOCK_SIZE);
    if (d -> total_size > 0 && d -> tail_size < BLOCK_SIZE)
    {
        size_t i;
        TEST_PSE(("slow append block!"));
        for (i = 0; i < size; ++i)
        {
            ERE(sls_push_back(d, b -> b + i));
        }
        kjb_free(b);
    }
    else
    {
        *(0 == d -> total_size ? & d -> head : & d -> tail -> next) = b;
        d -> tail = b;
        b -> next = NULL;
        d -> tail_size = size;
        d -> total_size += size;
    }
    return NO_ERROR;
}

static void sls_destroy(struct S_list* d)
{
    if (NULL == d || 0 == d -> total_size) return;
    ASSERT(d -> head && d -> tail);
    while (d -> head != d -> tail)
    {
        struct S_block* p = d -> head;
        d -> head = d -> head -> next;
        kjb_free(p);
    }
    kjb_free(d -> head);
    d -> head = d -> tail = NULL;
    d -> total_size = 0;
}

static int sls_pop_front(struct S_list* l, size_t* skip)
{
    NRE(l);
    NRE(skip);
    if (l -> head == l -> tail) /* list is down to its last block */
    {
        ASSERT((int) *skip <= l -> tail_size);
        if ((int) *skip == l -> tail_size)
        {
            kjb_free(l -> head);
            l -> head = l -> tail = NULL;
            l -> total_size = 0;
        }
    }
    else
    {
        ASSERT((int) *skip <= BLOCK_SIZE);
        if (BLOCK_SIZE == (int) *skip)
        {
            struct S_block* b = l -> head;
            l -> head = l -> head -> next;
            l -> total_size -= BLOCK_SIZE;
            *skip = 0;
            kjb_free(b);
        }
    }
    return NO_ERROR;
}

static int sls_merge(struct S_list* i1, struct S_list* i2, struct S_list* o)
{
    size_t skip1 = 0, skip2 = 0;
    NRE(i1);
    NRE(i2);
    NRE(o);
    if (o -> total_size)
    {
        set_error("output list was supposed to be empty");
        NOTE_ERROR();
        return ERROR;
    }
    while (i1 -> total_size && i2 -> total_size)
    {
        int r = seed_cmp(i1 -> head -> b + skip1, i2 -> head -> b + skip2);
        if (r <= 0)
        {
            if (0 == r)
            {
                TEST_PSE(("Duplicate found!"));
                db_print_seed(i1 -> head -> b + skip1);
                db_print_seed(i2 -> head -> b + skip2);
            }
            ERE(sls_push_back(o, i1 -> head -> b + skip1++));
            ERE(sls_pop_front(i1, &skip1));
        }
        else
        {
            ERE(sls_push_back(o, i2 -> head -> b + skip2++));
            ERE(sls_pop_front(i2, &skip2));
        }
    }
    while (i1 -> total_size)
    {
        ERE(sls_push_back(o, i1 -> head -> b + skip1++));
        ERE(sls_pop_front(i1, &skip1));
    }
    while (i2 -> total_size)
    {
        ERE(sls_push_back(o, i2 -> head -> b + skip2++));
        ERE(sls_pop_front(i2, &skip2));
    }
    return NO_ERROR;
}

static int sls_sort(struct S_list* d)
{
    int i;
    struct S_list l[2];
    NRE(d);
    if (d -> total_size <= BLOCK_SIZE) /* base case: one block */
    {
        return ow_sort_sblock(d -> head, d -> total_size);
    }
    /* merge sort for linked lists */
    for (i = 0; i < 2; ++i)
    {
        l[i].total_size = 0;
        l[i].head = l[i].tail = NULL;
    }
    for (i = 0; d -> total_size; i = 1 - i)
    {
        size_t sz;
        struct S_block* p = d -> head;
        ASSERT(p);
        if (d -> head == d -> tail)
        {
            sz = d -> tail_size;
            ASSERT((int) sz == d -> total_size);
        }
        else
        {
            sz = BLOCK_SIZE;
            d -> head = d -> head -> next;
        }
        d -> total_size -= sz;
        ERE(sls_append_block(l + i, p, sz));
    }
    d -> head = d -> tail = NULL;
    ERE(sls_sort(l));
    ERE(sls_sort(l+1));
    ERE(sls_merge(l, l+1, d));
    return NO_ERROR;
}




int main(int argc, char** argv)
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
    l.head = l.tail = NULL;
    
    for (p = tab; p -> lag >= 0; ++p)
    {
        EPETE(sls_push_back(&l, p));
    }
    db_print_sls(&l);
    ERE(sls_sort(&l));
    TEST_PSE(("................. sorted? .................\n"));
    db_print_sls(&l);
    sls_destroy(&l);
    return EXIT_SUCCESS;
}


