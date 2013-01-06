/* Overlay support for AMX.
 * Includes caching for block pointers.
 */

#include "amx.h"
#include "amxpool.h"
#include "ff.h"
#include <string.h>

#define AMX_ERR_FILE_CHANGED 101

typedef struct {
    int index;
    void *code;
    unsigned size;
} cache_t;

#define CACHESIZE 8
static cache_t cache[CACHESIZE];

static const char *amx_filename;
static FIL *amx_file;
static AMX_OVERLAYINFO *overlay_tbl;
static AMX_HEADER *amx_hdr;

// Inner (slow) part of overlay callback
static int __attribute__((noinline))
overlay_callback_full(AMX *amx, int index)
{
    // Check the full overlay pool
    amx->codesize = overlay_tbl[index].size;
    amx->code = amx_poolfind(index);
    
    // Finally, load from disc
    if (amx->code == NULL)
    {
        if ((amx->code = amx_poolalloc(amx->codesize, index)) == NULL)
            return AMX_ERR_FILE_CHANGED;   /* failure allocating memory for the overlay */
        
        // Unfortunately we have to clear the cache as poolalloc may 
        // have released some blocks.
        memset(cache, 0, sizeof(cache));
        
        // Verify that the file has not changed
        {
            FILINFO newfile;
            f_stat(amx_filename, &newfile);
            if (newfile.fsize != amx_file->fsize)
                return AMX_ERR_FILE_CHANGED;
        }
        
        // Read the block
        {
            AMX_HEADER *hdr = (AMX_HEADER*)amx->base;
            unsigned count;
            f_lseek(amx_file, hdr->cod + overlay_tbl[index].offset);
            f_read(amx_file, amx->code, amx->codesize, &count);
            if (count != amx->codesize)
                return AMX_ERR_FORMAT;
        }
            
        // Verify the loaded code and rewrite it.
        int ret = VerifyPcode(amx);
        if (ret != AMX_ERR_NONE)
            return ret;
    }
    
    return AMX_ERR_NONE;
}

// Outer (fast) part of overlay callback
int __attribute__((optimize("O2")))
overlay_callback(AMX *amx, int index)
{
    int cacheindex = index & (CACHESIZE - 1);
    cache_t *cacheentry = &cache[cacheindex];
    
    // First check our cache
    if (cacheentry->index == index
        && cacheentry->code != NULL)
    {
        amx->code = cacheentry->code;
        amx->codesize = cacheentry->size;
        return AMX_ERR_NONE;
    }
    
    int ret = overlay_callback_full(amx, index);
    
    // Update cache entry
    cacheentry->index = index;
    cacheentry->code = amx->code;
    cacheentry->size = amx->codesize;
    
    return ret;
}

void overlay_init(AMX *amx, const char *filename, FIL *file)
{
    amx->overlay = overlay_callback;
    memset(cache, 0, sizeof(cache));
    amx_filename = filename;
    amx_file = file;
    
    AMX_HEADER *hdr = (AMX_HEADER*)amx->base;
    overlay_tbl = (AMX_OVERLAYINFO*)(amx->base + hdr->overlays);
}
