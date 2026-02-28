#include <h26xcodec/video_reader.hpp>
#include <iostream>
#include <libavformat/avformat.h>

VideoReader::~VideoReader(){
    if(fmt_ctx){
        avformat_close_input(&fmt_ctx);
    }
}

void VideoReader::Open(){
    fmt_ctx = nullptr;

    int ret = avformat_open_input(&fmt_ctx, source_file_path.c_str(), NULL, NULL);
    if(ret < 0){
        std::cout << "Can't Open source file:" << source_file_path << std::endl;
        char errbuf[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_strerror(ret, errbuf, sizeof(errbuf));
        std::cerr << "avformat_open_input failed: " << errbuf << std::endl;
        return;
    }

    file_format = fmt_ctx->iformat->name;

    ret = avformat_find_stream_info(fmt_ctx, NULL);
    if(ret < 0){
        std::cout << "Can't read stream" << std::endl;
        return;
    }

    // int video_stream_index = -1;
    if(fmt_ctx->nb_streams){
        AVStream* stream = fmt_ctx->streams[0];
        if(stream->codecpar->codec_id == AV_CODEC_ID_HEVC){
            frame_format = FrameFormat::H265;
        }else if(stream->codecpar->codec_id == AV_CODEC_ID_H264){
            frame_format = FrameFormat::H264;
        }
    }else{
        std::cout << "Can't found streams" << std::endl;
        return;
    }
}

FrameFormat VideoReader::get_frame_format(){
    return frame_format;
}

std::string VideoReader::get_file_format(){
    return file_format;
};