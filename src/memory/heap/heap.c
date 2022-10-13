#include "status.h"
#include "types.h"
#include "memory/heap/heap.h"

/*
    * Doubly-Linked free bins list to store freed heap chunks
    * Merge with forward and backward free chunks if possible
    * Heap block structure:
    *       Chunk+0x00: prev_size(if prev chunk is free) or empty(if prev chunk inuse)
    *       Chunk+0x08: cur chunk size, last bit for previous chunk inuse if set
    *       Chunk+0x10: cur chunk user data start, forward pointer of free bins list(if free)
    *       CHunk+0x18: cur chunk data, backward pointer of free bins list(if free)
*/

MIRRORSTATUS heap_init(PVOID start, ULONG heap_size, PVOID free_bin_head)
{
    PVOID top_chunk = NULL;

    // check against default heap pool range
    if ( (ULONG)start < (ULONG)HEAP_START \
            || (ULONG)start + heap_size > (ULONG)HEAP_MAX \
            || (ULONG)start + heap_size < (ULONG)HEAP_START \
            || (ULONG)start > (ULONG)HEAP_MAX )
        return STATUS_EINVAL;

    if ( heap_size % HEAP_PAGE_SZ ) // upsize to page alignment
        heap_size += (HEAP_PAGE_SZ - (heap_size % HEAP_PAGE_SZ));

    if ( (ULONG)start % HEAP_ALIGN_SZ ) // start of heap must be page aligned
        return STATUS_EINVAL;

    // set head of doubly linked free bins list
    *(PULONG)free_bin_head = (ULONG)start;

    // set "top chunk", the big initial chunk to split chunks from
    top_chunk = (PVOID)(*(PULONG)free_bin_head);

    // set top chunk size, previous chunk is always in use
    chunk_set_size(top_chunk, (heap_size | PREV_INUSE));
    // first chunk, fd bk points back to itself
    chunk_set_fd(top_chunk, top_chunk);
    chunk_set_bk(top_chunk, top_chunk);

    return 0;
}

ULONG heap_align_heap_chunk(ULONG chunk_size)
{
    return (chunk_size % HEAP_CHUNK_ALIGN_SZ) ? \
    (HEAP_CHUNK_ALIGN_SZ + (chunk_size) - (chunk_size % HEAP_CHUNK_ALIGN_SZ)) : \
    chunk_size;
}

ULONG heap_calculate_chunksize(ULONG init_chunk_size)
{
    init_chunk_size += 0x8; // for size header

    if (init_chunk_size < (ULONG)HEAP_MIN_CHUNK_SZ)
        init_chunk_size = (ULONG)HEAP_MIN_CHUNK_SZ;
    
    return heap_align_heap_chunk(init_chunk_size);
}

PVOID heap_allocate(PVOID free_bin_head, ULONG chunk_size)
{
    PVOID chunk = NULL;

    chunk_size = heap_calculate_chunksize(chunk_size);
    
    // perform search to find required chunk size
    chunk = heap_find_available(free_bin_head, chunk_size);
    if (!chunk)
        return NULL;

    return (chunk+0x10); // user data
}

PVOID chunk_fd(PVOID cur)
{
    return (PVOID)(*(PULONG)(cur + 0x10));
}

PVOID chunk_bk(PVOID cur)
{
    return (PVOID)(*(PULONG)(cur + 0x18));
}

VOID chunk_set_size(PVOID addr, ULONG chunk_size)
{
    *(PULONG)(addr + 0x8) = chunk_size;
}

VOID chunk_set_fd(PVOID addr, PVOID fd)
{
    *(PULONG)(addr + 0x10) = (ULONG)fd;
}

VOID chunk_set_bk(PVOID addr, PVOID bk)
{
    *(PULONG)(addr + 0x18) = (ULONG)bk;
}

PVOID heap_find_available(PVOID free_bin_head, ULONG chunk_size)
{
    PVOID   cur, ret, B, F, old_cur;
    ULONG   cur_chunksize;

    cur = *(PVOID *)free_bin_head;

    // traverse linked list until we reach the last block or find desired size
    while ((ULONG)chunk_fd(cur) != *(PULONG)free_bin_head)
    {
        cur_chunksize = chunksize_nomask(chunksize_at_mem(cur));
        if ( cur_chunksize < chunk_size )
        {
            cur = chunk_fd(cur);
            continue;
        }

        // found suitable chunk, unlink if exhausted(less than min size)
        ret = cur;
        B = chunk_bk(cur);
        F = chunk_fd(cur);

        cur_chunksize -= chunk_size;

        if ( cur_chunksize < HEAP_MIN_CHUNK_SZ )
        {
            // return all if can't be allocated again
            chunk_size += cur_chunksize;
            cur_chunksize = 0;

            if ( cur == *(PVOID *)free_bin_head ) // if first chunk(current free bin head), update to next
            {
                *(PULONG)(free_bin_head) = (ULONG)chunk_fd(cur);
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

        // if no unlink, update fd and bk of chunk, because we "shrank" it
        chunk_set_fd(B, cur + chunk_size);
        chunk_set_bk(F, cur + chunk_size);

        // set size for our newly carved out chunk
        chunk_set_size(cur, (chunk_size | PREV_INUSE));

        old_cur = cur;
        cur += chunk_size; // the remaining "shrank" chunk
        chunk_set_size(cur, (cur_chunksize | PREV_INUSE));

        // write new fd and bk
        chunk_set_fd(cur, F);
        chunk_set_bk(cur, B);

        // if bin head, update bin head
        if (  old_cur == *(PVOID *)free_bin_head )
            *(PULONG)free_bin_head = (ULONG)cur;

        return ret;
    }

    // search exhausted, no free linked chunks found
    if ( (ULONG)chunk_fd(cur) == *(PULONG)free_bin_head )
    {
        // we are the last chunk left (top chunk)
        cur_chunksize = chunksize_nomask(chunksize_at_mem(cur));

        if ( cur_chunksize < chunk_size )
            return NULL; // no more memory :(

        ret = cur;
        B = chunk_bk(cur);
        F = chunk_fd(cur);
        chunk_set_size(cur, (chunk_size | PREV_INUSE));

        // fix the pointers of back and forward chunk
        cur += chunk_size;
        chunk_set_fd(B, cur);
        chunk_set_bk(F, cur);
        
        // fix new top chunk size
        cur_chunksize -= chunk_size;
        chunk_set_size(cur, (cur_chunksize | PREV_INUSE));

        // if top chunk is only chunk, shift bin pointer also
        if ( (ULONG)((ULONG)cur-(ULONG)chunk_size) == *(PULONG)free_bin_head )
        {
            *(PULONG)free_bin_head = (ULONG)cur;
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


VOID heap_free(PVOID free_bin_head, PVOID chunk_addr)
{
    BYTE        merged_back = 0, merged_front = 0;
    ULONG       chunk_sz_mask, chunk_sz_nomask, prev_sz;
    ULONG       new_sz, old_top_sz, new_top_sz, new_next_chunk_sz;
    PVOID       cur, topchunk_addr, target, top_bk;
    PVOID       new_top, next_chunk_addr, next_next_chunk_addr;
    PVOID       new_next_chunk, head, head_bk;
    PVOID       next_chunk_bk, next_chunk_fd;

    chunk_addr -= 0x10; // reach actual heap metadata

    // only allowed to free memory in the heap region
    if ( (ULONG)chunk_addr < HEAP_START \
        || (ULONG)chunk_addr > HEAP_MAX )
        return;
    
    chunk_sz_mask = chunksize_at_mem(chunk_addr);
    chunk_sz_nomask = chunksize_nomask(chunk_sz_mask);
    topchunk_addr = chunk_bk(*(PVOID *)free_bin_head);

    // check backward consolidation
    if ( !prev_inuse(chunk_sz_mask) )
    {
        // get to prev chunk by PREV_SIZE field
        target = chunk_addr - *(PULONG)(chunk_addr);
        prev_sz = chunksize_nomask(chunksize_at_mem(target));
        new_sz = prev_sz + chunk_sz_nomask;
        chunk_set_size(target, (new_sz | PREV_INUSE));

        // now check this big chunk
        chunk_addr = target;
        chunk_sz_mask = (new_sz | PREV_INUSE);
        chunk_sz_nomask = new_sz;

        merged_back = 1;
    }

    // if next chunk is top chunk, consolidate
    if ( (ULONG)chunk_addr + chunk_sz_nomask == (ULONG)topchunk_addr )
    {
        top_bk = chunk_bk(topchunk_addr);
        old_top_sz = chunksize_at_mem(topchunk_addr);
        new_top_sz = old_top_sz + chunk_sz_nomask;
        new_top = topchunk_addr - chunk_sz_nomask; // move top chunk backwards

        if (top_bk == *(PVOID *)free_bin_head)
        // if only top chunk in free bin
        {
            *(PULONG)free_bin_head = (ULONG)new_top;
            chunk_set_fd(new_top, new_top);
            chunk_set_bk(new_top, new_top);
        }
        else
        {
            chunk_set_fd(top_bk, new_top);
            chunk_set_bk(*(PVOID *)free_bin_head, new_top);
        }

        chunk_set_size(new_top, new_top_sz);
        topchunk_addr = new_top;

        if (merged_back)
        // if merged back and merge with top, unlink back chunk
        {
            cur = *(PVOID *)free_bin_head;
            while (chunk_fd(cur) != free_bin_head)
            {
                if (cur == topchunk_addr) // topchunk_addr is now addr of merged chunk
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
    next_chunk_addr = chunk_addr + chunk_sz_nomask;
    chunk_set_size(next_chunk_addr, chunksize_nomask(\
        chunksize_at_mem(next_chunk_addr)));

    // set prev_size field for next
    *(PULONG)next_chunk_addr = chunk_sz_nomask;

    // check forward consolidation
    next_next_chunk_addr = next_chunk_addr + \
        chunksize_nomask(chunksize_at_mem(next_chunk_addr));
    
    if ( (ULONG)next_next_chunk_addr < (ULONG)topchunk_addr )
    {
        if ( !prev_inuse(chunksize_at_mem(next_next_chunk_addr)) )
        {
            merged_front = 1;
            new_next_chunk = next_chunk_addr - *(PULONG)next_chunk_addr; // prev_size
            new_next_chunk_sz = chunksize_nomask(chunksize_at_mem(next_chunk_addr)) +\
                 *(PULONG)next_chunk_addr;

            // update prev_size
            *(PULONG)next_next_chunk_addr = new_next_chunk_sz;

            if (merged_back)
            // needs unlinking
            {
                cur = *(PVOID *)free_bin_head;
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

            next_chunk_bk = chunk_bk(next_chunk_addr);
            next_chunk_fd = chunk_fd(next_chunk_addr);

            // update the fd and bk pointers to point to our enlarged free chunk
            chunk_set_fd(next_chunk_bk, new_next_chunk);
            chunk_set_bk(next_chunk_fd, new_next_chunk);
            chunk_set_fd(new_next_chunk, next_chunk_fd);
            chunk_set_bk(new_next_chunk, next_chunk_fd);

            chunk_set_size(new_next_chunk, (new_next_chunk_sz | PREV_INUSE));
            
            // now this is new cur chunk
            chunk_addr = new_next_chunk;
            chunk_sz_mask = chunksize_at_mem(new_next_chunk);
            chunk_sz_nomask = chunksize_nomask(chunk_sz_mask);
        }
    }
    else
        ; // corruption detected, maybe do something in the future

    // all checks done, check if need to link into bin head
    if (merged_back || merged_front)
        return;

    head = *(PVOID *)free_bin_head;
    head_bk = chunk_bk(head);
    
    // link our new free chunk into free chunk list
    chunk_set_fd(chunk_addr, head);
    chunk_set_bk(chunk_addr, head_bk);
    chunk_set_fd(head_bk, chunk_addr);
    chunk_set_bk(head, chunk_addr);

    *(PULONG)free_bin_head = (ULONG)chunk_addr;

    return;
}

ULONG chunksize_at_mem(PVOID addr)
{
    return *(PULONG)(addr+0x8);
}

VOID unlink(PVOID cur)
{
    PVOID B = chunk_bk(cur);
    PVOID F = chunk_fd(cur);
    chunk_set_fd(B, chunk_fd(cur));
    chunk_set_bk(F, chunk_bk(cur));
}