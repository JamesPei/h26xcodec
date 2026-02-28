#pragma once

#ifndef __H26XCODEC_VIDEO_READER__
#define __H26XCODEC_VIDEO_READER__

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include <string>

enum class FrameFormat{
    UNKNOWN,
    H264,
    H265
};

class VideoReader {
public:
    VideoReader(std::string source_file_path):source_file_path(source_file_path){};
    ~VideoReader();

    void Open();

    FrameFormat get_frame_format();

    std::string get_file_format();

private:
    std::string source_file_path;
    std::string file_format;
    FrameFormat frame_format;
    AVFormatContext* fmt_ctx;
};

#endif