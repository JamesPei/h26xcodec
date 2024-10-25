#include <cxxopts.hpp>
#include <string>
#include <filesystem>
#include <iostream>
#include <h26xcodec/h26xdecoder.hpp>
#include <h26xcodec/h26xencoder.hpp>

namespace fs = std::filesystem;

std::string str_tolower(std::string s){
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

bool decode_video_to_image(const std::string& source_file_path, const std::string& output_dir_path, const std::string& source_format, const std::string& target_format){
    fs::path source_path(source_file_path);
    fs::path output_path(output_dir_path);

    if(!fs::exists(source_path)){
        throw fs::filesystem_error("source file not exists", std::error_code());
    }

    H26xDecoder decoder(source_format);
    size_t len = fs::file_size(source_path);
    std::ifstream input_stream(source_file_path, std::ios::binary);

    std::string data_in(len, '\0');
    input_stream.read(&data_in[0], len);
    ssize_t num_consumed = decoder.parse((unsigned char*)data_in.c_str(), len);

    decoder.decode_video(source_file_path, output_dir_path);
}

bool decode_frame_to_image(const std::string& source_file_path, const std::string& output_file_path, const std::string& source_format, const std::string& target_format){
    fs::path source_path(source_file_path);
    fs::path output_path(output_file_path);

    // H26xDecoder decoder(source_format);

    // if(!fs::exists(source_path)){
    //     throw fs::filesystem_error("source file not exists", std::error_code());
    // }

    // if(fs::is_directory(source_path)){
    //     for(const fs::directory_entry& dir_entry: fs::recursive_directory_iterator(source_path)){
    //         if(fs::is_regular_file(dir_entry.path())){
    //             std::string data_in(len, '\0');
    //             input_stream.read(&data_in[0], len);
    //             ssize_t num_consumed = decoder.parse((unsigned char*)data_in.c_str(), len);

    //             bool is_frame_available = false;
    //             // if((is_frame_available = decoder.is_frame_available())){
    //             //     consr auto& frame = decoder.decode_frame();
    //             // }
    //         }
    //     }
    // }else if (fs::is_regular_file(source_path)){

    // }
}

bool encode_image_to_frame(const std::string& source_file_path, const std::string& output_file_path, const std::string& source_format, const std::string& target_format){
    fs::path source_path(source_file_path);
    fs::path output_path(output_file_path);

    if(!fs::exists(source_path)){
        throw fs::filesystem_error("file not exist", std::error_code());
    }

    H26xEncoder encoder(target_format);
}

int main(int argc, char const *argv[])
{
    std::string usage_prompt = "h26xcodec is a tools collection of encoder、ecoderfor、and converter";
    cxxopts::Options options("h26xcodec", usage_prompt);
    options.add_options()
        ("h,help", "print usage")
        ("d,decode", "docode h26x to image")
        ("v,video", "decode video, only useful with -d")
        ("f,frame", "decode frame, only useful with -d", cxxopts::value<bool>()->default_value("false"))
        ("e,encode", "encode image to h26x", cxxopts::value<bool>()->default_value("false"))
        ("c,convert", "convert image format", cxxopts::value<bool>()->default_value("false"))
        ("p,path", "file or dir path", cxxopts::value<std::string>()->default_value("."))
        ("sf", "the format of source file, for decode is h264/h265, for encode is jpg/png/yuv420p/rgb", cxxopts::value<std::string>()->default_value("h265"))
        ("tf", "the format of target file, for decode is jpg/png/yuv420p/rgb, for encode is h264/h265", cxxopts::value<std::string>()->default_value("jpeg"))
        ("o,output", "output path", cxxopts::value<std::string>()->default_value("."))
        ("params", "other parameters", cxxopts::value<std::string>()->default_value("."))
        ;
    auto result = options.parse(argc, argv);

    if(result.count("help")){
        std::cout << options.help() << std::endl;
    }

    bool opt_decode = result["decode"].as<bool>();
    bool opt_encode = result["encode"].as<bool>();
    if(opt_decode && opt_encode){
        throw cxxopts::exceptions::specification("decode or encode can only be chosen one at a time");
    }

    std::string source_format = str_tolower(result["sf"].as<std::string>());
    std::string target_format = str_tolower(result["tf"].as<std::string>());
    if(opt_decode){
        if(source_format!="h264" && source_format!="h265" && source_format!="hevc"){
            throw cxxopts::exceptions::specification("illegal source format");
        }
        if(target_format!="jpg" && target_format!="jpeg" && target_format!="png" && target_format!="yuv420p" && target_format!="rgb"){
            throw cxxopts::exceptions::specification("illegal target format");
        }

        if(result.count("v")){
            decode_video_to_image(result["path"].as<std::string>(), result["output"].as<std::string>(), source_format, target_format);
        }else if(result.count("f")){

        }else{

        }
    }else if(opt_encode){
        if(source_format!="jpg" || source_format!="jpeg" || source_format!="png" || source_format!="yuv420p" || source_format!="rgb"){
            throw cxxopts::exceptions::specification("illegal source format");
        }
        if(target_format!="h264" || target_format!="h265" || target_format!="hevc"){
            throw cxxopts::exceptions::specification("illegal target format");
        }

    }

    return 0;
}
