#include "status.h"
#include "types.h"
#include "memory/heap/heap.h"

INT heap_init(PVOID start, ULONG heap_size, PVOID free_bin_head)
{
    if ( (DWORD)start < (DWORD)HEAP_START \
            || (DWORD)start + heap_size > (DWORD)HEAP_MAX \
            || (DWORD)start + heap_size < (DWORD)HEAP_START \
            || (DWORD)start > (DWORD)HEAP_MAX )
        return -EFAULT;

    if ( heap_size < (DWORD)HEAP_PAGE_SZ )
        heap_size = HEAP_PAGE_SZ; // upsize to page size
    
    if ( heap_size % HEAP_PAGE_SZ ) // not page aligned, reject
        return -EINVAL;

    if ( (DWORD)start % HEAP_ALIGN_SZ )
        return -EINVAL;

    *(PDWORD)free_bin_head = (DWORD)start;
    PVOID top_chunk = *(PVOID*)free_bin_head;
    chunk_set_size(top_chunk, (heap_size | PREV_INUSE));
    // first chunk, fd bk points back
    chunk_set_fd(top_chunk, top_chunk);
    chunk_set_bk(top_chunk, top_chunk);
    return 0;
}

ULONG align_heap_chunks(ULONG chunk_size)
{
    return (chunk_size % CHUNK_ALIGN_SZ) ? \
    (CHUNK_ALIGN_SZ + (chunk_size) - (chunk_size % CHUNK_ALIGN_SZ)) : \
    chunk_size;
}

PVOID heap_allocate(PVOID free_bin_head, ULONG chunk_size)
{
    chunk_size += 8; // for header

    if (chunk_size < (DWORD)MIN_CHUNK_SZ)
        chunk_size = (DWORD)MIN_CHUNK_SZ;
    chunk_size = align_heap_chunks(chunk_size);
    
    PVOID chunk = heap_find_available(free_bin_head, chunk_size);
    if (chunk == NULL)
        return NULL;
    return (chunk+0x10);
}

PVOID chunk_fd(PVOID cur)
{
    return (PVOID)(*(PDWORD)(cur + 16));
}

PVOID chunk_bk(PVOID cur)
{
    return (PVOID)(*(PDWORD)(cur + 24));
}

VOID chunk_set_size(PVOID addr, ULONG chunk_size)
{
    *(PDWORD)(addr + 8) = chunk_size;
}

VOID chunk_set_fd(PVOID addr, PVOID fd)
{
    *(PDWORD)(addr + 16) = (DWORD)fd;
}

VOID chunk_set_bk(PVOID addr, PVOID bk)
{
    *(PDWORD)(addr + 24) = (DWORD)bk;
}

PVOID heap_find_available(PVOID free_bin_head, ULONG chunk_size)
{
    PVOID cur = *((PVOID*)free_bin_head);
    ULONG cur_chunksize;

    while (chunk_fd(cur) != *((PVOID*)free_bin_head))
    {
        cur_chunksize = chunksize_nomask(chunksize_at_mem(cur));
        if ( cur_chunksize < chunk_size )
        {
            cur = chunk_fd(cur);
            continue;
        }

        // found suitable chunk, unlink if exhausted
        PVOID ret = cur;
        PVOID B = chunk_bk(cur);
        PVOID F = chunk_fd(cur);

        cur_chunksize -= chunk_size;

        if ( cur_chunksize < MIN_CHUNK_SZ )
        {
            // return all if can't be allocated again;
            chunk_size += cur_chunksize;
            cur_chunksize = 0;

            if ( cur == *((PVOID*)free_bin_head) ) // first chunk
            {
                *((PVOID*)free_bin_head) = chunk_fd(cur);
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

        PVOID old_cur = cur;
        cur += chunk_size;
        chunk_set_size(cur, (cur_chunksize | PREV_INUSE));

        // write new fd and bk
        chunk_set_fd(cur, F);
        chunk_set_bk(cur, B);

        // update PREV_SIZE field
        *(PDWORD)(cur + chunk_size) = chunk_size;

        // if bin head, update bin head
        if (  old_cur == *((PVOID*)free_bin_head) )
            *((PVOID*)free_bin_head) = cur;

        return ret;
    }

    cur_chunksize = chunksize_nomask(chunksize_at_mem(cur));
    if ( chunk_fd(cur) == *((PVOID*)free_bin_head) )
    {
        // we are the last chunk left (top chunk)
        if ( cur_chunksize < chunk_size )
            return NULL; // no more memory
        PVOID ret = cur;
        PVOID B = chunk_bk(cur);
        PVOID F = chunk_fd(cur);
        chunk_set_size(cur, (chunk_size | PREV_INUSE));
        cur += chunk_size;

        chunk_set_fd(B, cur);
        chunk_set_bk(F, cur);
        
        cur_chunksize -= chunk_size; // don't unlink because last chunk
        chunk_set_size(cur, (cur_chunksize | PREV_INUSE));

        // if top chunk is only chunk, shift bin pointer also
        if ( cur-chunk_size == *((PVOID*)free_bin_head) )
        {
            *((PVOID*)free_bin_head) = cur;
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
    INT merged_back = 0, merged_front = 0;
    chunk_addr -= 0x10;

    // only allowed to free memory in the heap region
    if ( (DWORD)chunk_addr < HEAP_START \
        || (DWORD)chunk_addr > HEAP_MAX )
        return;
    
    ULONG chunk_size_mask = chunksize_at_mem(chunk_addr);
    ULONG chunk_sz_nomask = chunksize_nomask(chunk_size_mask);
    PVOID topchunk_addr = chunk_bk(*((PVOID*)free_bin_head));

    // check backward consolidation
    if ( !prev_inuse(chunk_size_mask) )
    {
        // get to prev chunk by PREV_SIZE field
        PVOID target = chunk_addr - *(PDWORD)(chunk_addr);
        ULONG old_sz = chunksize_nomask(chunksize_at_mem(target));
        ULONG new_sz = old_sz + chunk_sz_nomask;
        chunk_set_size(target, (new_sz | PREV_INUSE));

        // now check this big chunk
        chunk_addr = target;
        chunk_size_mask = (new_sz | PREV_INUSE);
        chunk_sz_nomask = new_sz;

        merged_back = 1;
    }

    // if next chunk is top chunk, consolidate
    if ( (DWORD)chunk_addr + chunk_sz_nomask == (DWORD)topchunk_addr )
    {
        PVOID top_bk = chunk_bk(topchunk_addr);
        ULONG old_top_sz = chunksize_at_mem(topchunk_addr);
        ULONG new_top_sz = old_top_sz + chunk_sz_nomask;
        PVOID new_top = topchunk_addr - chunk_sz_nomask;

        if (top_bk == *((PVOID*)free_bin_head))
        // if only top chunk in free bin
        {
            *((PVOID*)free_bin_head) = new_top;
            chunk_set_fd(new_top, new_top);
            chunk_set_bk(new_top, new_top);
        }
        else
        {
            chunk_set_fd(top_bk, new_top);
            chunk_set_bk(*((PVOID*)free_bin_head), new_top);
        }

        chunk_set_size(new_top, new_top_sz);
        topchunk_addr = new_top;

        if (merged_back)
        // if merged back and merge with top, unlink back chunk
        {
            PVOID cur = *((PVOID*)free_bin_head);
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
    PVOID next_chunk_addr = chunk_addr + chunk_sz_nomask;
    chunk_set_size(next_chunk_addr, chunksize_nomask(\
        chunksize_at_mem(next_chunk_addr)));

    // set PREV_SIZE field for next
    *(PDWORD)next_chunk_addr = chunk_sz_nomask;

    // check forward consolidation
    PVOID next_next_chunk_addr = next_chunk_addr + \
        chunksize_nomask(chunksize_at_mem(next_chunk_addr));
    
    if ( (DWORD)next_next_chunk_addr < (DWORD)topchunk_addr )
    {
        if ( !prev_inuse(chunksize_at_mem(next_next_chunk_addr)) )
        {
            merged_front = 1;
            PVOID new_next_chunk = next_chunk_addr - *(PDWORD)next_chunk_addr;
            ULONG new_next_chunk_sz = chunksize_nomask(chunksize_at_mem(next_chunk_addr)) +\
                 *(PDWORD)next_chunk_addr;

            if (merged_back)
            // needs unlinking
            {
                PVOID cur = *((PVOID*)free_bin_head);
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

            PVOID next_chunk_bk = chunk_bk(next_chunk_addr);
            PVOID next_chunk_fd = chunk_fd(next_chunk_addr);

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

    PVOID head = *((PVOID*)free_bin_head);

    PVOID head_bk = chunk_bk(head);
    chunk_set_fd(chunk_addr, head);
    chunk_set_bk(chunk_addr, head_bk);
    chunk_set_fd(head_bk, chunk_addr);
    chunk_set_bk(head, chunk_addr);

    *((PVOID*)free_bin_head) = chunk_addr;

    return;
}

ULONG chunksize_at_mem(PVOID addr)
{
    return *(PDWORD)(addr+8);
}

VOID unlink(PVOID cur)
{
    PVOID B = chunk_bk(cur);
    PVOID F = chunk_fd(cur);
    chunk_set_fd(B, chunk_fd(cur));
    chunk_set_bk(F, chunk_bk(cur));
}