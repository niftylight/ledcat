/*
 * ledcat - CLI tool to send greyscale values to LED devices using libniftyled
 * Copyright (C) 2006-2011 Daniel Hiepler <daniel@niftylight.de>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>
#include <niftyled.h>
#include "cache.h"


/** cache descriptor */
struct _Cache
{
        /** amount of frames in cache */
        size_t frames;
        /** first cached frame */
        CachedFrame *first;
        /** true if caching is disabled */
        bool disabled;
};



/**
 * enable or disable cache
 *
 * @param c a cache acquired by cache_new()
 * @param disabled set to true if caching should be disabled, false if enabled
 */
void cache_disable(Cache * c, bool disabled)
{
        NFT_LOG(L_DEBUG, "%s frame cache",
                disabled ? "Disabling" : "Enabling");
        c->disabled = disabled;
}


/**
 * cache a frame
 *
 * @param c a cache acquired by cache_new()
 * @param frame raw frame data (will be copied)
 * @param size size of raw frame in bytes
 * @param filename the filename of the frame (will be truncated to 255 bytes)
 * @result NFT_SUCCESS or NFT_FAILURE
 */
NftResult cache_frame_put(Cache * c, void *frame, size_t size, char *filename)
{
        if(c->disabled)
                return NFT_SUCCESS;

        /* allocate new CachedFrame */
        CachedFrame *f;
        if(!(f = calloc(1, sizeof(CachedFrame))))
                return NFT_FAILURE;

        if(!(f->frame = malloc(size)))
        {
                free(f);
                return NFT_FAILURE;
        }

        /* copy raw frame data */
        memcpy(f->frame, frame, size);

        /* store size */
        f->size = size;

        /* copy filename */
        strncpy(f->filename, filename, sizeof(f->filename));

        /* seek to last frame in cache */
        if(!c->first)
        {
                c->first = f;
        }
        else
        {
                CachedFrame *l;
                for(l = c->first; l->next; l = l->next);
                l->next = f;
        }

        /* increase counter */
        c->frames++;

        NFT_LOG(L_DEBUG, "Frame \"%s\" cached (%d frames in cache)", filename,
                c->frames);
        return NFT_SUCCESS;
}


/**
 * get a frame from cache
 *
 * @param c a cache acquired by cache_new()
 * @param filename the filname of the frame to get
 * @result pointer to raw frame data or NULL
 */
CachedFrame *cache_frame_get(Cache * c, char *filename)
{
        if(c->disabled)
                return NULL;

        for(CachedFrame * f = c->first; f; f = f->next)
        {
                if(strcmp(filename, f->filename) != 0)
                        continue;

                NFT_LOG(L_DEBUG, "Frame \"%s\" found in cache", filename);
                return f;
        }

        NFT_LOG(L_DEBUG, "Frame \"%s\" not found in cache", filename);
        return NULL;
}


/**
 * initialize a new cache
 *
 * @result new cache or NULL
 */
Cache *cache_new()
{
        NFT_LOG(L_DEBUG, "creating new frame cache");

        Cache *n = calloc(1, sizeof(Cache));

        n->disabled = false;

        return n;
}


/**
 * free a cache and all its resources
 *
 * @param c a cache acquired by cache_new()
 */
void cache_destroy(Cache * c)
{
        /* free all cached frames */
        CachedFrame *a = c->first;
        while(a)
        {
                CachedFrame *b = a->next;
                free(a->frame);
                free(a);
                a = b;
        }

        /* free cache */
        free(c);

        NFT_LOG(L_DEBUG, "destroying frame cache");
}
