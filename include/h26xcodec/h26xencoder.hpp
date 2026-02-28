#pragma once

#ifndef __H26XCODEC_ENCODER__
#define __H26XCODEC_ENCODER__

#include <fstream>
#include <map>
#include <string>
#include <vector>
#include "h26xexceptions.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/mem.h>
#include <libavutil/opt.h>
#include <libavutil/pixfmt.h>
#include <libavutil/rational.h>
#include <libswscale/swscale.h>
}

class H26xEncoder
{
public:
    H26xEncoder(AVCodecID id = AV_CODEC_ID_NONE)
      : codec_id_{id}
      , codec_{nullptr}
      , context_{nullptr}
      , frame_{nullptr}
      , packet_{}
      , width_{-1}
      , height_{-1}
      , fps_{0}
      , gop_size_{-1}
      , max_b_frames_{0}
      , refs_{0}
      , thread_num_{4}
      , options_{}
      , input_pixel_format_{}
      , bits_per_pixel_{0}
      , swsContext_{nullptr}
      , frame_index_{0}
    {
    }

    H26xEncoder(std::string const& name)
      : codec_id_{AV_CODEC_ID_NONE}
      , codec_{nullptr}
      , context_{nullptr}
      , frame_{nullptr}
      , packet_{}
      , width_{-1}
      , height_{-1}
      , fps_{0}
      , gop_size_{-1}
      , max_b_frames_{0}
      , refs_{0}
      , thread_num_{4}
      , options_{}
      , input_pixel_format_{}
      , bits_per_pixel_{0}
      , swsContext_{nullptr}
      , frame_index_{0}
    {
        if (name == "h264" || name == "H264")
        {
            codec_id_ = AV_CODEC_ID_H264;
        }
        else if (name == "h265" || name == "H265" || name == "hevc" || name == "HEVC")
        {
            codec_id_ = AV_CODEC_ID_H265;
        }
    }

    ~H26xEncoder()
    {
        avcodec_free_context(&context_);
        sws_freeContext(swsContext_);
        av_frame_free(&frame_);
    }

    void Enable();

    void SetWidth(int value)
    {
        width_ = value;
    }

    int GetWidth()
    {
        return width_;
    }

    void SetHeight(int value)
    {
        height_ = value;
    }

    int GetHeight()
    {
        return height_;
    }

    void SetFps(int value)
    {
        fps_ = value;
    }

    int GetFps()
    {
        return fps_;
    }

    void SetGopSize(int value)
    {
        gop_size_ = value;
    }

    int GetGopSize()
    {
        return gop_size_;
    }

    void SetInputPixelFormat(AVPixelFormat value)
    {
        input_pixel_format_ = value;
    }

    void SetInputPixelFormat(std::string const& value)
    {
        if (value == "RGB24" || value == "RGB")
        {
            input_pixel_format_ = AV_PIX_FMT_RGB24;
        }
        else if (value == "YUV240P" || value == "NV12")
        {
            input_pixel_format_ = AV_PIX_FMT_YUV420P;
        }
    }

    AVPixelFormat GetInputPixelFormat()
    {
        return input_pixel_format_;
    }

    std::string GetInputPixelFormatString()
    {
        if (input_pixel_format_ == AV_PIX_FMT_YUV420P)
        {
            return "YUV420P";
        }
        if (input_pixel_format_ == AV_PIX_FMT_RGB24)
        {
            return "RGB24";
        }
        return "UNKNOWN";
    }

    void SetRefs(int value)
    {
        refs_ = value;
    }

    int GetRefs()
    {
        return refs_;
    }

    void SetMaxBFrames(int value)
    {
        max_b_frames_ = value;
    }

    int GetMaxBFrames()
    {
        return max_b_frames_;
    }

    void SetThreadNum(int value)
    {
        thread_num_ = value;
    }

    int GetThreadNum()
    {
        return thread_num_;
    }

    void SetOption(std::string const& name, std::string const& value)
    {
        options_[name] = value;
    }

    std::string GetOption(std::string const& name)
    {
        if (options_.find(name) == options_.end())
        {
            return "";
        }
        else
        {
            return options_[name];
        }
    }

    std::map<std::string, std::string>& GetOptions()
    {
        return options_;
    }

    void SetOptions(std::map<std::string, std::string> const& value)
    {
        options_ = value;
    }

    void createCodec();
    void createContext();
    void calculateBitsPerPixel();
    void createSwsContext();
    void createAVFrameAndAVPacket();

    void fillYuv420pFrame(uint8_t const* input_image);
    void fillRgb24Frame(uint8_t const* input_image);
    bool sendFrame();
    bool recvPacket(std::vector<char>& output);
    bool Encode(uint8_t const* input, std::vector<char>& output);
    bool Flush(std::vector<char>& output);

    std::string Str();

protected:
    AVCodecID                          codec_id_;
    AVCodecContext*                    context_;
    AVFrame*                           frame_;
    AVPacket                           packet_;
    AVCodec*                           codec_;
    int                                width_;
    int                                height_;
    int                                fps_;
    int                                gop_size_;
    int                                max_b_frames_;
    int                                refs_;
    int                                thread_num_;
    std::map<std::string, std::string> options_;
    AVPixelFormat                      input_pixel_format_;
    int                                bits_per_pixel_;
    SwsContext*                        swsContext_;

    int frame_index_;
};

#endif