#include "status.h"
#include "memory/heap/heap.h"

static int merged_back = 0;
static int merged_front = 0;

int heap_init(void * start, size_t heap_size, void * free_bin_head)
{
    if ( (unsigned int)start < (unsigned int)HEAP_START \
            || (unsigned int)start + heap_size > (unsigned int)HEAP_MAX \
            || (unsigned int)start + heap_size < (unsigned int)HEAP_START \
            || (unsigned int)start > (unsigned int)HEAP_MAX )
        return -EFAULT;

    if ( heap_size < (unsigned int)HEAP_PAGE_SZ )
        heap_size = HEAP_PAGE_SZ; // upsize to page size
    
    if ( heap_size % HEAP_PAGE_SZ ) // not page aligned, reject
        return -EINVAL;

    if ( (unsigned int)start % HEAP_ALIGN_SZ )
        return -EINVAL;

    *(unsigned int *)free_bin_head = (unsigned int)start;
    void * top_chunk = *(void **)free_bin_head;
    chunk_set_size(top_chunk, (heap_size | PREV_INUSE));
    // first chunk, fd bk points back
    chunk_set_fd(top_chunk, top_chunk);
    chunk_set_bk(top_chunk, top_chunk);
    return 0;

}

size_t align_heap_chunks(size_t chunk_size)
{
    return (chunk_size % CHUNK_ALIGN_SZ) ? \
    (CHUNK_ALIGN_SZ + (chunk_size) - (chunk_size % CHUNK_ALIGN_SZ)) : \
    chunk_size;
}

void * heap_allocate(void * free_bin_head, size_t chunk_size)
{
    chunk_size += 8; // for header

    if (chunk_size < (unsigned int)MIN_CHUNK_SZ)
        chunk_size = (unsigned int)MIN_CHUNK_SZ;
    chunk_size = align_heap_chunks(chunk_size);
    
    void * chunk = heap_find_available(free_bin_head, chunk_size);
    if (chunk == NULL)
        return NULL;
    return (chunk+0x10);
}

void * chunk_fd(void * cur)
{
    return (void *)(*(unsigned int *)(cur + 16));
}

void * chunk_bk(void * cur)
{
    return (void *)(*(unsigned int *)(cur + 24));
}

void chunk_set_size(void * addr, size_t chunk_size)
{
    *(unsigned int *)(addr + 8) = chunk_size;
}

void chunk_set_fd(void * addr, void * fd)
{
    *(unsigned int *)(addr + 16) = (unsigned int)fd;
}

void chunk_set_bk(void * addr, void * bk)
{
    *(unsigned int *)(addr + 24) = (unsigned int)bk;
}

void * heap_find_available(void * free_bin_head, size_t chunk_size)
{
    void * cur = *((void **)free_bin_head);
    size_t cur_chunksize;

    while (chunk_fd(cur) != *((void **)free_bin_head))
    {
        cur_chunksize = chunksize_nomask(chunksize_at_mem(cur));
        if ( cur_chunksize < chunk_size )
        {
            cur = chunk_fd(cur);
            continue;
        }

        // found suitable chunk, unlink if exhausted
        void * ret = cur;
        void * B = chunk_bk(cur);
        void * F = chunk_fd(cur);

        cur_chunksize -= chunk_size;

        if ( cur_chunksize < MIN_CHUNK_SZ )
        {
            // return all if can't be allocated again;
            chunk_size += cur_chunksize;
            cur_chunksize = 0;

            if ( cur == *((void **)free_bin_head) ) // first chunk
            {
                *((void **)free_bin_head) = chunk_fd(cur);
                unlink(cur);
            }
            else
                unlink(cur);

            // now set size of returning chunk and set prev_inuse of next
            chunk_set_size(cur, (chunk_size | PREV_INUSE));
            chunk_set_size(cur + chunk_size, (chunksize_at_mem(\
                cur + chunk_size) | PREV_INUSE));

            return ret;
        }

        // if no unlink
        chunk_set_fd(B, cur + chunk_size);
        chunk_set_bk(F, cur + chunk_size);

        chunk_set_size(cur, (chunk_size | PREV_INUSE));

        void * old_cur = cur;
        cur += chunk_size;
        chunk_set_size(cur, (cur_chunksize | PREV_INUSE));

        // write new fd and bk
        chunk_set_fd(cur, F);
        chunk_set_bk(cur, B);

        // update PREV_SIZE field
        *(unsigned int *)(cur + chunk_size) = chunk_size;

        // if bin head, update bin head
        if (  old_cur == *((void **)free_bin_head) )
            *((void **)free_bin_head) = cur;

        return ret;
    }

    cur_chunksize = chunksize_nomask(chunksize_at_mem(cur));
    if ( chunk_fd(cur) == *((void **)free_bin_head) )
    {
        // we are the last chunk left (top chunk)
        if ( cur_chunksize < chunk_size )
            return NULL; // no more memory
        void * ret = cur;
        void * B = chunk_bk(cur);
        void * F = chunk_fd(cur);
        chunk_set_size(cur, (chunk_size | PREV_INUSE));
        cur += chunk_size;

        chunk_set_fd(B, cur);
        chunk_set_bk(F, cur);
        
        cur_chunksize -= chunk_size; // don't unlink because last chunk
        chunk_set_size(cur, (cur_chunksize | PREV_INUSE));

        // if top chunk is only chunk, shift bin pointer also
        if ( cur-chunk_size == *((void **)free_bin_head) )
        {
            *((void **)free_bin_head) = cur;
            chunk_set_fd(cur, cur);
            chunk_set_bk(cur, cur);
        }
        else
        {
            chunk_set_fd(cur, F);
            chunk_set_bk(cur, B);
        }

        return ret;
    }
    return NULL; // won't reach here but just for safety
}


void heap_free(void * free_bin_head, void * chunk_addr)
{
    merged_back = 0;
    merged_front = 0;
    chunk_addr -= 0x10;

    // only allowed to free memory in the heap region
    if ( (unsigned int)chunk_addr < HEAP_START \
        || (unsigned int)chunk_addr > HEAP_MAX )
        return;
    
    size_t chunk_size_mask = chunksize_at_mem(chunk_addr);
    size_t chunk_sz_nomask = chunksize_nomask(chunk_size_mask);
    void * topchunk_addr = chunk_bk(*((void **)free_bin_head));

    // check backward consolidation
    if ( !prev_inuse(chunk_size_mask) )
    {
        // get to prev chunk by PREV_SIZE field
        void * target = chunk_addr - *(unsigned int *)(chunk_addr);
        size_t old_sz = chunksize_nomask(chunksize_at_mem(target));
        size_t new_sz = old_sz + chunk_sz_nomask;
        chunk_set_size(target, (new_sz | PREV_INUSE));

        // now check this big chunk
        chunk_addr = target;
        chunk_size_mask = (new_sz | PREV_INUSE);
        chunk_sz_nomask = new_sz;

        merged_back = 1;
    }

    // if next chunk is top chunk, consolidate
    if ( (unsigned int)chunk_addr + chunk_sz_nomask == (unsigned int)topchunk_addr )
    {
        void * top_bk = chunk_bk(topchunk_addr);
        size_t old_top_sz = chunksize_at_mem(topchunk_addr);
        size_t new_top_sz = old_top_sz + chunk_sz_nomask;
        void * new_top = topchunk_addr - chunk_sz_nomask;

        if (top_bk == *((void **)free_bin_head))
        // if only top chunk in free bin
        {
            *((void **)free_bin_head) = new_top;
            chunk_set_fd(new_top, new_top);
            chunk_set_bk(new_top, new_top);
        }
        else
        {
            chunk_set_fd(top_bk, new_top);
            chunk_set_bk(*((void **)free_bin_head), new_top);
        }

        chunk_set_size(new_top, new_top_sz);
        topchunk_addr = new_top;

        if (merged_back)
        // if merged back and merge with top, unlink back chunk
        {
            void * cur = *((void **)free_bin_head);
            while (chunk_fd(cur) != free_bin_head)
            {
                if (cur == topchunk_addr)
                {
                    unlink(cur);
                    break;
                }
                cur = chunk_fd(cur);
            }
        }

        // we are done here, no forward consolidation, no bit unset
        return;
    }

    // unset inuse bit for next
    void * next_chunk_addr = chunk_addr + chunk_sz_nomask;
    chunk_set_size(next_chunk_addr, chunksize_nomask(\
        chunksize_at_mem(next_chunk_addr)));

    // set PREV_SIZE field for next
    *(unsigned int *)next_chunk_addr = chunk_sz_nomask;

    // check forward consolidation
    void * next_next_chunk_addr = next_chunk_addr + \
        chunksize_nomask(chunksize_at_mem(next_chunk_addr));
    
    if ( (unsigned int)next_next_chunk_addr < (unsigned int)topchunk_addr )
    {
        if ( !prev_inuse(chunksize_at_mem(next_next_chunk_addr)) )
        {
            merged_front = 1;
            void * new_next_chunk = next_chunk_addr - *(unsigned int *)next_chunk_addr;
            size_t new_next_chunk_sz = chunksize_nomask(chunksize_at_mem(next_chunk_addr)) +\
                 *(unsigned int *)next_chunk_addr;

            if (merged_back)
            // needs unlinking
            {
                void * cur = *((void **)free_bin_head);
                while (chunk_fd(cur) != free_bin_head)
                {
                    if (cur == new_next_chunk)
                    {
                        unlink(cur);
                        break;
                    }
                    cur = chunk_fd(cur);
                }
            }

            void * next_chunk_bk = chunk_bk(next_chunk_addr);
            void * next_chunk_fd = chunk_fd(next_chunk_addr);

            chunk_set_fd(next_chunk_bk, new_next_chunk);
            chunk_set_bk(next_chunk_fd, new_next_chunk);
            chunk_set_fd(new_next_chunk, next_chunk_fd);
            chunk_set_bk(new_next_chunk, next_chunk_fd);

            chunk_set_size(new_next_chunk, (new_next_chunk_sz | PREV_INUSE));
            
            // now this is new cur chunk
            chunk_addr = new_next_chunk;
            chunk_size_mask = chunksize_at_mem(new_next_chunk);
            chunk_sz_nomask = chunksize_nomask(chunk_size_mask);
        }
    }

    // all checks done, check if need to link into bin head
    if (merged_back || merged_front)
        return;

    void * head = *((void **)free_bin_head);

    void * head_bk = chunk_bk(head);
    chunk_set_fd(chunk_addr, head);
    chunk_set_bk(chunk_addr, head_bk);
    chunk_set_fd(head_bk, chunk_addr);
    chunk_set_bk(head, chunk_addr);

    *((void **)free_bin_head) = chunk_addr;

    return;
}

size_t chunksize_at_mem(void * addr)
{
    return *(unsigned int *)(addr+8);
}

void unlink(void * cur)
{
    void * B = chunk_bk(cur);
    void * F = chunk_fd(cur);
    chunk_set_fd(B, chunk_fd(cur));
    chunk_set_bk(F, chunk_bk(cur));
}