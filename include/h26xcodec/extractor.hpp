#pragma once

#ifndef __H26XCODEC_EXTRACTOR__
#define __H26XCODEC_EXTRACTOR__

#include <string>

class Extractor {
public:
    Extractor(std::string source_file_path);
    void extract(std::string target_format);

private:
    std::string source_file_path;
};

#endif