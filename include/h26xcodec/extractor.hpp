#pragma once

#ifndef __H26XCODEC_EXTRACTOR__
#define __H26XCODEC_EXTRACTOR__

#include <string>
#include <vector>
#include <functional>
#include "video_reader.hpp"

struct AVFrame;

class Extractor {
public:
    Extractor(std::string source_file_path);
    void extract(std::vector<std::string>& output_frames);
    /** 直接从 MP4 解码并回调每一帧，绕过 parser，适用于容器格式 */
    void extract_decoded(std::function<void(const AVFrame&)> on_frame);

private:
    std::string source_file_path;
};

#endif