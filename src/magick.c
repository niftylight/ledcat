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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_IMAGEMAGICK == 1
#include <wand/MagickWand.h>
#endif

#include <unistd.h>
#include <niftyled.h>
#include "ledcat.h"
#include "magick.h"




/** 
 * ImageMagick error-handler 
 */
void im_error(MagickWand *wand)
{
#if HAVE_IMAGEMAGICK == 1
        char *description;
        ExceptionType  severity;
        
        description = MagickGetException(wand, &severity);
        NFT_LOG(L_ERROR, "%s %s %lu %s\n", GetMagickModule(), description);
        description = (char *) MagickRelinquishMemory(description);
#endif
}


/**
 * initialize ImageMagick
 */
NftResult im_init(struct Ledcat *c)
{
#if HAVE_IMAGEMAGICK == 1
        /* initialize magickWand */
        if(!c->raw)
        {
                MagickWandGenesis();
                if(!(c->mw = NewMagickWand()))
                        return FALSE;

              
        }
#endif
        return TRUE;
}


/**
 * deinitialize ImageMagick
 */
void im_deinit(struct Ledcat *c)
{
#if HAVE_MAGICKWAND == 1       
        if(!c->raw)
        {
                DestroyMagickWand(c->mw);
                MagickWandTerminus();
        }
#endif
}


/** 
 * initialize stream for ImageMagick 
 */
NftResult im_open_stream(struct Ledcat *c)
{
#if HAVE_IMAGEMAGICK == 1
        /* open a stream (for ImageMagick)? */
        if(!c->raw)
        {
                /* open stream from file-descriptor */
                if(!(c->file = fdopen(c->fd, "r")))
                {
                        NFT_LOG_PERROR("fdopen()");
                        return FALSE;
                }
        }
#endif
        return TRUE;
}


/**
 * deinitialize stream
 */
void im_close_stream(struct Ledcat *c)
{
        if(c->file)
        {
                /* close stream */
                fclose(c->file);
                c->file = NULL;
        }

        /* close file */
        close(c->fd);
}


/**
 * determine format that ImageMagick should provide 
 */
NftResult im_format(struct Ledcat *c, LedPixelFormat *format)
{
#if HAVE_IMAGEMAGICK == 1

        /* determine map format for MagickGetImagePixels */
        strncpy(c->map, led_pixel_format_colorspace_to_string(format), sizeof(c->map));

        /* determine storage format for MagickGetImagePixels */
        char type[16];
        strncpy(type, led_pixel_format_type_to_string(format, 0), sizeof(type));
        if(strncmp(type, "u8", sizeof(type)) == 0)
                c->storage = CharPixel;
        else if(strncmp(type, "u16", sizeof(type)) == 0)
                c->storage = ShortPixel;
        else if(strncmp(type, "u32", sizeof(type)) == 0)
                c->storage = IntegerPixel;
        else if(strncmp(type, "u64", sizeof(type)) == 0)
                c->storage = LongPixel;
        else if(strncmp(type, "double", sizeof(type)) == 0)
                c->storage = DoublePixel;
        else if(strncmp(type, "float", sizeof(type)) == 0)
                c->storage = FloatPixel;
        else
        {
                NFT_LOG(L_ERROR, "Unhandled pixel-format: \"%s\"", type);
                return FALSE;
        }
#endif
        
        return TRUE;
}


/**
 * read frame using ImageMagick
 */
NftResult im_read_frame(struct Ledcat *c, size_t width, size_t height, char *buf)
{
#if HAVE_IMAGEMAGICK == 1

        /* is there an image from a previous read? */
        if(MagickHasNextImage(c->mw))
        {
                MagickNextImage(c->mw);
        }
        else
        {
                /* end-of-stream? */
                if(feof(c->file))
                        return FALSE;
                
                /* read file */
                if(!MagickReadImageFile(c->mw, c->file))
                {
                        im_error(c->mw);
                        return FALSE;
                }

                /* reset iterator in case we read more than one file */
                //MagickResetIterator(c->mw);
        }

        
        /* turn possible alpha-channel black */
        /*PixelWand *pw;
        if(!(pw = NewPixelWand()))
                return FALSE;
        
        PixelSetColor(pw, "black");
        MagickSetImageBackgroundColor(c->mw, pw);
        DestroyPixelWand(pw);*/
                
        /* get raw-buffer from imagemagick */
        if(!(MagickExportImagePixels(c->mw, 0, 0, width, height, c->map, c->storage, buf)))
        {
                im_error(c->mw);
                return FALSE;
        }
        
        /* free resources */
        if(!MagickHasNextImage(c->mw))
                ClearMagickWand(c->mw);

#endif
        
        return TRUE;
}
