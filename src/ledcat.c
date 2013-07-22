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

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include <getopt.h>
#include <niftyled.h>
#include "ledcat.h"
#include "cache.h"
#include "version.h"
#include "raw.h"
#include "magick.h"




/** main structure to hold global info */
static struct Ledcat _c;


/******************************************************************************/
/**************************** STATIC FUNCTIONS ********************************/
/******************************************************************************/

/** signal handler for exiting */
void _exit_signal_handler(int signal)
{
        NFT_LOG(L_INFO, "Exiting...");
        _c.running = false;
}

#if ! WIN32
/** signal handler that prints current fps */
void _alarm_signal_handler(int signal)
{
        NFT_LOG(L_INFO, "FPS: %d", led_fps_get());
}
#endif



/** print commandline help */
static void _print_help(char *name)
{
        printf("Send image to LED hardware - %s\n"
               "Usage: %s [options] <file(s)>\n\n"
               "Choose \"-\" as <file> to read from stdin\n\n"
               "Valid options:\n"
               "\t--help\t\t\t-h\t\tThis help text\n"
               "\t--plugin-help\t\t-p\t\tList of installed plugins + information\n"
               "\t--config <file>\t\t-c <file>\tLoad this prefs file [~/.ledcat.xml]\n"
               "\t--no-cache\t\t-n\t\tDon't use frame cache [off]\n"
               "\t--dimensions <w>x<h>\t-d <w>x<h>\tDefine width and height of input frames. [auto]\n"
               "\t--big-endian\t\t-b\t\tRAW data is big-endian ordered [off]\n"
               "\t--loop\t\t\t-L\t\tDon't exit after last file but start over with first [off]\n"
               "\t--fps <n>\t\t-F <n>\t\tFramerate to play multiple frames at. (Ignored when --signal is used) [25]\n"
#if HAVE_IMAGEMAGICK == 1
               "\t--raw\t\t\t-r\t\tTreat input files as raw-files (false)\n"
#endif
               "\t--format <format>\t-f <format>\tPixelformat of raw frame - doesn't have effect without --raw. (s. http://gegl.org/babl/ for supported formats)\n"
               "\t--loglevel <level>\t-l <level>\tOnly show messages with loglevel <level> (info)\n\n",
               PACKAGE_URL, name);

        /* print loglevels */
        printf("\nValid loglevels:\n\t");
		nft_log_print_loglevels();
        printf("\n\n");
}


/** print list of installed plugins + information they provide */
static void _print_plugin_help()
{

        /* save current loglevel */
        NftLoglevel ll_current = nft_log_level_get();
        nft_log_level_set(L_INFO);

        int i;
        for(i = 0; i < led_hardware_plugin_total_count(); i++)
        {
                const char *name;
                if(!(name = led_hardware_plugin_get_family_by_n(i)))
                        continue;

                printf("======================================\n\n");

                LedHardware *h;
                if(!(h = led_hardware_new("tmp01", name)))
                        continue;

                printf("\tID Example: %s\n",
                       led_hardware_plugin_get_id_example(h));


                led_hardware_destroy(h);

        }

        /* restore logolevel */
        nft_log_level_set(ll_current);
}


/** parse commandline arguments */
static NftResult _parse_args(int argc, char *argv[])
{
        int index, argument;

        static struct option loptions[] = {
                {"help", 0, 0, 'h'},
                {"plugin-help", 0, 0, 'p'},
                {"loglevel", required_argument, 0, 'l'},
                {"config", required_argument, 0, 'c'},
                {"dimensions", required_argument, 0, 'd'},
                {"fps", required_argument, 0, 'F'},
                {"format", required_argument, 0, 'f'},
                {"big-endian", no_argument, 0, 'b'},
                {"loop", no_argument, 0, 'L'},
                {"no-cache", no_argument, 0, 'n'},
#if HAVE_IMAGEMAGICK == 1
                {"raw", no_argument, 0, 'r'},
#endif
                {0, 0, 0, 0}
        };

#if HAVE_IMAGEMAGICK == 1
        const char arglist[] = "hpl:c:d:F:f:bLnr";
#else
        const char arglist[] = "hpl:c:d:F:f:bLn";
#endif
        while((argument =
               getopt_long(argc, argv, arglist, loptions, &index)) >= 0)
        {

                switch (argument)
                {
                                /* --help */
                        case 'h':
                        {
                                _print_help(argv[0]);
                                return NFT_FAILURE;
                        }

                                /* --plugin-help */
                        case 'p':
                        {
                                _print_plugin_help();
                                return NFT_FAILURE;
                        }

                                /* --config */
                        case 'c':
                        {
                                /* save filename for later */
                                strncpy(_c.prefsfile, optarg,
                                        sizeof(_c.prefsfile));
                                break;
                        }

                        /** --loop */
                        case 'L':
                        {
                                _c.do_loop = true;
                                break;
                        }

                        /** --dimensions */
                        case 'd':
                        {
                                if(sscanf
                                   (optarg, "%32dx%32d", (int *) &_c.width,
                                    (int *) &_c.height) != 2)
                                {
                                        NFT_LOG(L_ERROR,
                                                "Invalid dimension \"%s\" (Use something like 320x400)",
                                                optarg);
                                        return NFT_FAILURE;
                                }
                                break;
                        }

                        /** --fps */
                        case 'F':
                        {
                                if(sscanf(optarg, "%32d", (int *) &_c.fps) !=
                                   1)
                                {
                                        NFT_LOG(L_ERROR,
                                                "Invalid framerate \"%s\" (Use an integer)",
                                                optarg);
                                        return NFT_FAILURE;
                                }
                                break;
                        }

                        /** --loglevel */
                        case 'l':
                        {
                                if(!nft_log_level_set
                                   (nft_log_level_from_string(optarg)))
                                {
                                        _print_loglevels();
                                        return NFT_FAILURE;
                                }

                                break;
                        }

                        /** --format */
                        case 'f':
                        {
                                strncpy(_c.pixelformat, optarg,
                                        sizeof(_c.pixelformat));
                                break;
                        }

#if HAVE_IMAGEMAGICK == 1
                        /** --raw */
                        case 'r':
                        {
                                _c.raw = true;
                                break;
                        }
#endif

                        /** --big-endian */
                        case 'b':
                        {
                                _c.is_big_endian = true;
                                break;
                        }


                        /** --no-cache */
                        case 'n':
                        {
                                _c.no_caching = true;
                                break;
                        }

                                /* invalid argument */
                        case '?':
                        {
                                NFT_LOG(L_ERROR, "argument %d is invalid",
                                        index);
                                _print_help(argv[0]);
                                return NFT_FAILURE;
                        }


                                /* unhandled arguments */
                        default:
                        {
                                NFT_LOG(L_ERROR, "argument %d is invalid",
                                        index);
                                break;
                        }
                }
        }


        _c.files = &argv[optind];


        return NFT_SUCCESS;
}





/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

int main(int argc, char *argv[])
{
        /* current configuration */
        LedPrefs *p = NULL;
        /* current setup */
        LedSetup *s = NULL;
        /* list of LED hardware adapters */
        LedHardware *hw = NULL;
        /* input pixel-frame buffer */
        LedFrame *frame = NULL;
        /* width of map */
        LedFrameCord width;
        /* height of map */
        LedFrameCord height;
        /* frame cache */
        Cache *cache = NULL;



        /* check libniftyled binary version compatibility */
        if(!NFT_LED_CHECK_VERSION)
                return EXIT_FAILURE;

        /* set default loglevel to INFO */
        if(!nft_log_level_set(L_INFO))
        {
                fprintf(stderr, "nft_log_level_set() error");
                return EXIT_FAILURE;
        }


        /* initialize exit handlers */
#if WIN32
        int signals[] = { SIGINT, SIGABRT };
#else
        int signals[] = { SIGHUP, SIGINT, SIGQUIT, SIGABRT };
#endif
        unsigned int i;
        for(i = 0; i < sizeof(signals) / sizeof(int); i++)
        {
                if(signal(signals[i], _exit_signal_handler) == SIG_ERR)
                {
                        NFT_LOG_PERROR("signal()");
                        return EXIT_FAILURE;
                }
        }



        /* default fps */
        _c.fps = 25;

        /* default endianess */
        _c.is_big_endian = false;

#if HAVE_IMAGEMAGICK == 1
        /* default handle-input-as-raw */
        _c.raw = false;
#endif

        /* default looping */
        _c.do_loop = false;

        /* set "running" flag */
        _c.running = true;

        /* use caching by default */
        _c.no_caching = false;

        /* default pixel-format */
        strncpy(_c.pixelformat, "RGB u8", sizeof(_c.pixelformat));

        /* default prefs-filename */
        if(!led_prefs_default_filename
           (_c.prefsfile, sizeof(_c.prefsfile), ".ledcat.xml"))
                return EXIT_FAILURE;


        /* parse commandline arguments */
        if(!_parse_args(argc, argv))
                return EXIT_FAILURE;



        /* print welcome msg */
        NFT_LOG(L_INFO, "%s %s (c) D.Hiepler 2006-2013", PACKAGE_NAME,
                ledcat_version_long());
        NFT_LOG(L_VERBOSE, "Loglevel: %s",
                nft_log_level_to_string(nft_log_level_get()));


#if HAVE_IMAGEMAGICK == 1
        /* initialize imagemagick */
        if(!_c.raw)
        {
                if(!im_init(&_c))
                {
                        NFT_LOG(L_ERROR, "Failed to initialize ImageMagick");
                        return EXIT_FAILURE;
                }
        }
#endif

        /* default result of main() function */
        int res = EXIT_FAILURE;

        /* initialize preferences context */
        if(!(p = led_prefs_init()))
                goto m_deinit;


        /* parse prefs-file */
        LedPrefsNode *pnode;
        if(!(pnode = led_prefs_node_from_file(_c.prefsfile)))
        {
                NFT_LOG(L_ERROR, "Failed to open configfile \"%s\"",
                        _c.prefsfile);
                goto m_deinit;
        }

        /* create setup from prefs-node */
        if(!(s = led_prefs_setup_from_node(p, pnode)))
        {
                NFT_LOG(L_ERROR, "No valid setup found in preferences file.");
                led_prefs_node_free(pnode);
                goto m_deinit;
        }

        /* free preferences node */
        led_prefs_node_free(pnode);

        /* determine width of input-frames */
        if(!_c.width)
                /* width of mapped chain */
                width = led_setup_get_width(s);
        else
                /* use value from cmdline arguments */
                width = _c.width;


        /* determine height of input-frames */
        if(!_c.height)
                /* height of mapped chain */
                height = led_setup_get_height(s);
        else
                height = _c.height;


        /* validate dimensions */
        if(width <= 0 || height <= 0)
        {
                NFT_LOG(L_ERROR,
                        "Dimensions %dx%d not possible in this universe. Exiting.",
                        width, height);
                goto m_deinit;
        }


	/* get first toplevel hardware */
        if(!(hw = led_setup_get_hardware(s)))
                goto m_deinit;

        /* initialize pixel->led mapping */
        if(!led_hardware_list_refresh_mapping(hw))
                goto m_deinit;
	
        /* allocate frame (where our pixelbuffer resides) */
        NFT_LOG(L_INFO, "Allocating frame: %dx%d (%s)",
                width, height, _c.pixelformat);
        LedPixelFormat *format = led_pixel_format_from_string(_c.pixelformat);
        if(!(frame = led_frame_new(width, height, format)))
                goto m_deinit;


        /* precalc memory offsets for actual mapping */
        LedHardware *ch;
        for(ch = hw; ch; ch = led_hardware_list_get_next(ch))
        {
                if(!led_chain_map_from_frame
                   (led_hardware_get_chain(ch), frame))
                        goto m_deinit;
        }

        /* set correct gain to hardware */
        if(!led_hardware_list_refresh_gain(hw))
                goto m_deinit;

#if HAVE_IMAGEMAGICK == 1
        /* determine format that ImageMagick should provide */
        if(!_c.raw)
        {
                if(!(im_format(&_c, format)))
                {
                        NFT_LOG(L_ERROR,
                                "Failed to determine valid ImageMagick format");
                        goto m_deinit;
                }
        }
#endif

        /* do we have at least one filename? */
        if(!_c.files[0])
        {
                NFT_LOG(L_ERROR, "No input file(s) given");
                goto m_deinit;
        }


        /* initially sample time for frame-timing */
        if(!led_fps_sample())
                goto m_deinit;


#if ! WIN32
        /* initialize alarm-signal handler for periodical fps output */
        {
                struct sigaction sa;
                struct itimerval timer;

                memset(&sa, 0, sizeof(sa));
                sa.sa_handler = &_alarm_signal_handler;
                sigaction(SIGALRM, &sa, NULL);
                timer.it_value.tv_sec = 1;
                timer.it_value.tv_usec = 0;
                timer.it_interval.tv_sec = 1;
                timer.it_interval.tv_usec = 0;
                setitimer(ITIMER_REAL, &timer, NULL);
        }
#endif


        /* get data-buffer of frame to write our pixels to */
        char *buf;
        if(!(buf = led_frame_get_buffer(frame)))
        {
                NFT_LOG(L_ERROR, "Frame has NULL buffer");
                goto m_deinit;
        }


        /* initialize frame cache */
        if(!(cache = cache_new()))
        {
                NFT_LOG(L_ERROR, "Failed to initialize frame cache");
                goto m_deinit;
        }

        /* cache disabled by commandline */
        if(_c.no_caching)
                cache_disable(cache, true);





        /* walk all files (supplied as commandline arguments) and output them */
        int filecount;
        for(filecount = 0; _c.files[filecount]; filecount++)
        {

                NFT_LOG(L_VERBOSE, "Getting pixels from \"%s\"",
                        _c.files[filecount]);


                /* check if file is already cached */
                CachedFrame *f;
                bool cached_frame_found;

                if((!_c.no_caching) && 
                   (f = cache_frame_get(cache, _c.files[filecount])))
                {
                        /* copy frame to buffer */
                        memcpy(buf, f->frame, f->size);

                        /* mark current frame as cached */
                        cached_frame_found = true;
                }
                else
                {
                        /* current frame not found in cache */
                        cached_frame_found = false;

                        /* open file */
                        if(_c.files[filecount][0] == '-' &&
                           strlen(_c.files[filecount]) == 1)
                        {
                                _c.fd = STDIN_FILENO;
                        }
                        else
                        {
                                if((_c.fd =
                                    open(_c.files[filecount], O_RDONLY)) < 0)
                                {
                                        NFT_LOG(L_ERROR,
                                                "Failed to open \"%s\": %s",
                                                _c.files[filecount],
                                                strerror(errno));
                                        continue;
                                }
                        }

#if HAVE_IMAGEMAGICK == 1
                        /** initialize stream for ImageMagick */
                        if(!_c.raw)
                        {
                                if(!(im_open_stream(&_c)))
                                        continue;
                        }
#endif
                }



                /* output file frame-by-frame */
                while(_c.running)
                {

                        if(!cached_frame_found)
                        {
#if HAVE_IMAGEMAGICK == 1
                                /* use imagemagick to load file if we're not in 
                                 * "raw-mode" */
                                if(!_c.raw)
                                {
                                        /* load frame to buffer using
                                         * ImageMagick */
                                        if(!im_read_frame
                                           (&_c, width, height, buf))
                                                break;

                                }
                                else
                                {
#endif
                                        /* read raw frame */
                                        if(raw_read_frame
                                           (&_c.running, buf, _c.fd,
                                            led_pixel_format_get_buffer_size
                                            (led_frame_get_format(frame),
                                             led_frame_get_width(frame) *
                                             led_frame_get_height(frame))) ==
                                           0)
                                                continue;

#if HAVE_IMAGEMAGICK == 1
                                }
#endif

                                /* cache frame */
                                if(!
                                   (cache_frame_put
                                    (cache, buf,
                                     led_frame_get_buffersize(frame),
                                     _c.files[filecount])))
                                {
                                        NFT_LOG(L_ERROR,
                                                "Failed to cache frame \"%s\"",
                                                _c.files[filecount]);
                                        break;
                                }

                        }


                        /* set endianess (flag will be changed when conversion
                         * occurs) */
                        led_frame_set_big_endian(frame, _c.is_big_endian);

                        /* fill chain of every hardware from frame */
                        LedHardware *h;
                        for(h = hw; h; h = led_hardware_list_get_next(h))
                        {
                                if(!led_chain_fill_from_frame
                                   (led_hardware_get_chain(h), frame))
                                {
                                        NFT_LOG(L_ERROR,
                                                "Error while mapping frame");
                                        break;
                                }
                        }

                        /* send frame to hardware(s) */
                        NFT_LOG(L_DEBUG, "Sending frame");
                        led_hardware_list_send(hw);

                        /* delay in respect to fps */
                        if(!led_fps_delay(_c.fps))
                                break;

                        /* latch hardware */
                        NFT_LOG(L_DEBUG, "Showing frame");
                        led_hardware_list_show(hw);


                        /* save time when frame is displayed */
                        if(!led_fps_sample())
                                break;

                        /* if frame is from cache, there are no more frames
                         * since this can't be a stream */
                        if(cached_frame_found)
                                break;

                }

                /* close file if not from cache */
                if(!cached_frame_found)
                {
#if HAVE_IMAGEMAGICK == 1
                        if(!_c.raw)
                                im_close_stream(&_c);
#else
                        close(_c.fd);
#endif
                }

                /* reset flag */
                cached_frame_found = false;

                /* loop endlessly? */
                if((_c.running) && (!_c.files[filecount + 1]) && _c.do_loop)
                {
                        /* start over by resetting the for-loop */
                        filecount = -1;
                }
        }


        /* all ok */
        res = EXIT_SUCCESS;


m_deinit:
        /* free frame cache */
		if(!_c.no_caching)
        		cache_destroy(cache);

        /* free setup */
        led_setup_destroy(s);

        /* free frame */
        led_frame_destroy(frame);

        /* destroy config */
        led_prefs_deinit(p);

#if HAVE_IMAGEMAGICK == 1
        /* deinitialize ImageMagick */
        im_deinit(&_c);
#endif

        return res;
}
