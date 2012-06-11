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

#include <niftyled.h>
#include <unistd.h>

/* we need this for fd_set on windows */
#if WIN32
#include <winsock.h>
#endif




/**
 * read a complete raw pixel-frame
 */
int raw_read_frame(bool *running, char *buf, int fd, size_t size)
{

        size_t bytes_to_read, bytes_read = 0;
        
        for(bytes_to_read = size; 
            bytes_to_read > 0;  
            bytes_to_read -= bytes_read)
        {		
                /* read from stdin? */
                if(fd == STDIN_FILENO)
                {
                        /* prepare stuff for select() */
                        fd_set fds;
                        struct timeval tv;
                        FD_ZERO(&fds);
                        FD_SET(fd, &fds);
                        tv.tv_sec = 0;
                        tv.tv_usec = 500;
                        
                        /* wait for incoming data */
                        while(*running && (select(1, &fds, NULL, NULL, &tv) == 0));
                }
                                                
                /* break loop if we're not running anymore */
                if(!*running)
                        break;

                /* read data into buffer */
                if((bytes_read = read(fd, buf, bytes_to_read)) < 0)
                {
                        NFT_LOG_PERROR("read()");
                        return 0;
                }

                /* end of file? */
                if(fd != STDIN_FILENO && bytes_read == 0)
                {
                        /* end of file? */
                        if(bytes_read == 0)
                                break;
                }
                
                buf += bytes_read;
        }

        return bytes_read;
}
