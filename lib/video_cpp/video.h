/* $Id: video.h 21067 2017-01-14 21:16:43Z kobus $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
   |
   |  Personal and educational use of this code is granted, provided that this
   |  header is kept intact, and that the authorship is not misrepresented, that
   |  its use is acknowledged in publications, and relevant papers are cited.
   |
   |  For other use contact the author (kobus AT cs DOT arizona DOT edu).
   |
   |  Please note that the code in this file has not necessarily been adequately
   |  tested. Naturally, there is no guarantee of performance, support, or fitness
   |  for any particular task. Nonetheless, I am interested in hearing about
   |  problems that you encounter.
   |
   |  Author:  Kyle Simek
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker

#ifndef KJB_CPP_I_VIDEO_H
#define KJB_CPP_I_VIDEO_H

/**
 * @file Classes for videos and video frames to ease the reading of videos using ffmpeg.  Frames are
 * stored in a slim format optimized for fast passing to opengl.
 */


#ifdef KJB_HAVE_FFMPEG

// this is necessary to ensure stdint.h defines the UINT64_C macro,
// which is needed in libavutil/common.h :
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
}

#define AVFORMAT_IS_RECENT (defined(LIBAVFORMAT_VERSION_MAJOR ) && LIBAVFORMAT_VERSION_MAJOR >= 53 && LIBAVFORMAT_VERSION_MINOR >= 4)

/* Kobus: Jan 13, 2017 --- I don't gaurentee these patches to translate between
 * versions, but I don't have the time to figure out the details. 
*/
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55,28,1)
    #define PIX_FMT_RGB24 AV_PIX_FMT_RGB24
    #define PixelFormat  AVPixelFormat 
    #define avcodec_alloc_frame av_frame_alloc  
#endif


#endif /* KJB_HAVE_FFMPEG */

#include <boost/shared_array.hpp>
#include <i_cpp/i_image.h>

namespace kjb
{

class Video_frame
{
public:
    Video_frame(boost::shared_array<unsigned char> data, size_t width, size_t height) :
        data_(data),
        width_(width),
        height_(height)
    { }

public:
    Image to_image() const;

private:
    boost::shared_array<unsigned char> data_;
    size_t width_;
    size_t height_;
};

// (eventually would like to have a lazy video class in addition to the video class.
//  The lazy video would load frames only when needed.  Hence the abstract base class here.
class Abstract_video
{
public:
    virtual size_t size() const = 0;
    virtual Video_frame operator[](size_t i) const = 0;

    virtual size_t get_width() const = 0;
    virtual size_t get_height() const = 0;
    virtual float get_frame_rate() const = 0;
};

class Video : public Abstract_video
{
public:
    Video() {}

    Video(const std::vector<std::string>& fnames, float frame_rate = 30.0)
    {
        load_images(fnames, frame_rate);
    }

    Video(const std::string& fnames)
    {
        decode_video(fnames);
    }

    void decode_video(const std::string& fname)
    {
#if defined(KJB_HAVE_FFMPEG) && AVFORMAT_IS_RECENT
        AVFormatContext *pFormatCtx = NULL;
        int             i, videoStream;
        AVCodecContext  *pCodecCtx = NULL;
        AVCodec         *pCodec = NULL;
        AVFrame         *pFrame = NULL; 
        AVFrame         *pFrameRGB = NULL;
        AVPacket        packet;
        int             frameFinished;
        int             numBytes;
        uint8_t         *buffer = NULL;

        /* Register all formats and codecs */
        if(!ffmpeg_registered_)
        {
            av_register_all();
        }



        /* Open video file */
        if(avformat_open_input(&pFormatCtx, fname.c_str(), NULL, NULL)!=0)
            KJB_THROW_3(kjb::IO_error, "Couldn't open file %s", (fname.c_str()));

        /* Retrieve stream information */
        if(avformat_find_stream_info(pFormatCtx, NULL)<0)
            KJB_THROW_2(kjb::Runtime_error, "Failed to find stream info");

    //    /* Dump information about file onto standard error */
    //    dump_format(pFormatCtx, 0, argv[1], false);

        /* Find the first video stream */
        videoStream=-1;
        for(i=0; i< (int) pFormatCtx->nb_streams; i++)
        {
            if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
            {
                videoStream=i;
                break;
            }
        }

        if(videoStream==-1)
            KJB_THROW_2(kjb::Runtime_error, "Didn't find a video stream");

        /* Get a pointer to the codec context for the video stream */
        pCodecCtx=pFormatCtx->streams[videoStream]->codec;
        size_t num_frames = pFormatCtx->streams[videoStream]->nb_frames;

        frames_.resize(0);
        frames_.reserve(num_frames);
        AVRational fps = pFormatCtx->streams[videoStream]->r_frame_rate;
        frame_rate_ = (float) fps.num / fps.den;

        if(frame_rate_ == 0)
        {
            fps = pFormatCtx->streams[videoStream]->avg_frame_rate;
            frame_rate_ = (float) fps.num / fps.den;

        }

        assert(frame_rate_ > 0);

        /* Find the decoder for the video stream */
        pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
        if(pCodec==NULL)
            KJB_THROW_2(kjb::Runtime_error, "Codec not found");

        /* Open codec */
        if(avcodec_open2(pCodecCtx, pCodec, NULL)<0)
            KJB_THROW_2(kjb::Runtime_error, "Codec not found");

        /* Hack to correct wrong frame rates that seem to be generated by some codecs */
        if(pCodecCtx->time_base.num>1000 && pCodecCtx->time_base.den==1)
            pCodecCtx->time_base.den=1000;
            
        /* Allocate video frame */
        pFrame=avcodec_alloc_frame();

        /* Allocate an AVFrame structure */
        pFrameRGB=avcodec_alloc_frame();
        if(pFrameRGB==NULL)
            KJB_THROW_2(kjb::Runtime_error, "Failed to allocate AVFrame");

        /* Determine required buffer size and allocate buffer */
        numBytes=avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width,
            pCodecCtx->height);

        width_ = pCodecCtx->width;
        height_ = pCodecCtx->height;

        buffer=(uint8_t*)malloc(numBytes);

        /* Assign appropriate parts of buffer to image planes in pFrameRGB */
        avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24,
            pCodecCtx->width, pCodecCtx->height);

        /* Read all frames */

        while(av_read_frame(pFormatCtx, &packet)>=0)
        {
            /* Is this a packet from the video stream? */
            if(packet.stream_index==videoStream)
            {
                /* Decode video frame */
                avcodec_decode_video2(pCodecCtx,
                              pFrame, &frameFinished, &packet); 

                /* Did we get a video frame? */
                if(frameFinished)
                {
                    static struct SwsContext *img_convert_ctx;

                    /* Convert the image into YUV format that SDL uses */
                    if(img_convert_ctx == NULL) {
                        int w = pCodecCtx->width;
                        int h = pCodecCtx->height;
                        
                        img_convert_ctx = sws_getContext(w, h, 
                                        pCodecCtx->pix_fmt, 
                                        w, h, PIX_FMT_RGB24, SWS_BICUBIC,
                                        NULL, NULL, NULL);
                        if(img_convert_ctx == NULL) {
                            fprintf(stderr, "Cannot initialize the conversion context!\n");
                            exit(1);
                        }
                    }
                    sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0, 
                              pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);

                    /* Save the frame to disk */
                    const size_t& width = pCodecCtx->width;
                    const size_t& height = pCodecCtx->height;

                    boost::shared_array<unsigned char> buffer(new unsigned char[width * height * 3]);

                    for(size_t row = 0; row < height; row++)
                    {
                        const uint8_t* source_begin = pFrameRGB->data[0] + row * pFrameRGB->linesize[0];
                        unsigned char* dest_begin = buffer.get() + (height - row - 1) * width * 3;
                        size_t row_length = width * 3;

                        std::copy(source_begin,
                                source_begin + row_length,
                                dest_begin);
                    }

                    frames_.push_back(buffer);
                }
            }

            /* Free the packet that was allocated by av_read_frame */
            av_free_packet(&packet);
        }

//        assert(num_frames == frames_.size());

        /* Free the RGB image */
        free(buffer);
        av_free(pFrameRGB);

        /* Free the YUV frame */
        av_free(pFrame);

        /* Close the codec */
        avcodec_close(pCodecCtx);

        /* Close the video file */
        avformat_close_input(&pFormatCtx);

#else
        KJB_THROW_2(Missing_dependency, "ffmpeg");
#endif
    }

    void load_images(const std::vector<Image>& images, float /* frame_rate = 30.0 */)
    {
        frames_.resize(images.size());
        KJB_THROW(Not_implemented);
    }

    template<class Iterator>
    void load_images(Iterator first, Iterator last, float frame_rate = 30.0)
    {
        size_t nfrms = std::distance(first, last);

        frames_.resize(nfrms);
        frame_rate_ = frame_rate;

        size_t i = 0;
        for(; first != last; first++, i++)
        {
            Image img(*first);

            if(i == 0)
            {
                width_ = img.get_num_cols();
                height_ = img.get_num_rows();
            }
            else
            {
                if((int) width_ != img.get_num_cols() ||
                    (int) height_ != img.get_num_rows())
                {
                    KJB_THROW_2(Illegal_argument, "All images must be same size.");
                }
            }

            frames_[i] = image_to_frame_(img);
        }
    }

    void load_images(const std::vector<std::string>& fnames, float frame_rate = 30.0)
    {
        load_images(fnames.begin(), fnames.end(), frame_rate);
    }

    virtual size_t size() const
    {
        return frames_.size();
    }

    virtual Video_frame operator[](size_t i) const
    {
        if(i > size())
        {
            KJB_THROW(Index_out_of_bounds);
        }

        return Video_frame(frames_[i], width_, height_);
    }

    size_t get_width() const
    {
        return width_;
    }

    size_t get_height() const
    {
        return height_;
    }

    float get_frame_rate() const
    {
        return frame_rate_;
    }

    const unsigned char* get_buffer(size_t i) const
    {
        return frames_[i].get();
    }

    template <class Output_iterator>
    void to_images(Output_iterator frame_it)
    {
        Image img;
        size_t num_cols = width_;
        size_t num_rows = height_;
        img = Image(num_cols, num_rows);

        size_t stride = 3 * num_cols;
        for(size_t f = 0; f < frames_.size(); f++)
        {
            *frame_it++ = (*this)[f].to_image();
//            size_t out_col = 0;
//            int out_row = num_rows - 1;
//            size_t offset = 0;
//
//            for(size_t row = 0; row < num_rows; row++)
//            {
//                for(size_t col = 0; col < 3*num_cols; col += 3)
//                {
//                    img(out_row, out_col, Image::RED) = (float) frames_[f][offset + col + 0 ];
//                    img(out_row, out_col, Image::GREEN) = (float) frames_[f][offset + col + 1];
//                    img(out_row, out_col, Image::BLUE) = (float) frames_[f][offset + col + 2];
//                    ++out_col;
//                }
//                --out_row;
//                offset += stride;
//            }
//
//            *frame_it++ = img;
        }
    }

protected:
    static boost::shared_array<unsigned char> image_to_frame_(const Image& img)
    {
        size_t num_cols = img.get_num_cols();
        size_t num_rows = img.get_num_rows();

        boost::shared_array<unsigned char> buffer(new unsigned char[3 * num_rows * num_cols]);

        for(size_t row = 0; row < num_rows; row++)
        {
            for(size_t col = 0; col < 3*num_cols; col += 3)
            {
                size_t r = num_rows - row - 1;

                buffer[col + 0 + row*3*num_cols] = (unsigned char) img(r, col/3, Image::RED);
                buffer[col + 1 + row*3*num_cols] = (unsigned char) img(r, col/3, Image::GREEN);
                buffer[col + 2 + row*3*num_cols] = (unsigned char) img(r, col/3, Image::BLUE);
            }
        }

        return buffer;
    }
private:
    std::vector<boost::shared_array<unsigned char> > frames_;
    size_t width_;
    size_t height_;
    float frame_rate_;

    static bool ffmpeg_registered_;
};

}
#endif

