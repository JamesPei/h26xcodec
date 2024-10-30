#include <cxxopts.hpp>
#include <string>
#include <filesystem>
#include <iostream>
#include <chrono>
#include <map>
#include <nlohmann/json.hpp>
#include <h26xcodec/h26xdecoder.hpp>
#include <h26xcodec/h26xencoder.hpp>
#include <h26xcodec/converter.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

struct EncoderParameters{
    uint32_t width=0;
    uint32_t height=0;
    std::string input_pixel_format;
    uint32_t gop_size=0;
    uint32_t fps=25;
    uint32_t refs=0;
    uint32_t max_b_frames=0;
    uint32_t thread_num=4;
    std::map<std::string, std::string> options;
};

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

    std::vector<std::shared_ptr<AVFrame>> decoded_frames;
    decoder.decode_video(source_file_path, decoded_frames);

    if(target_format=="rgb" || target_format=="jpeg" || target_format=="jpg"){
        ConverterRGB24 converter;
        int i=0;
        for(auto frame: decoded_frames){
            int         w, h;
            std::tie(w, h)      = width_height(*frame);
            size_t out_size = converter.predict_size(w, h);

            std::string out_buffer(out_size, '\0');
            converter.convert(*frame, (unsigned char*)out_buffer.c_str());
            const std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();

            std::string output_file_name = std::to_string(now.time_since_epoch().count())+"_"+std::to_string(i)+"."+target_format;

            if(target_format=="jpg" || target_format=="jpeg"){
                auto converted_jpeg = converter.to_jpeg();
                out_buffer = *converted_jpeg;
            }
            std::ofstream output_stream(output_dir_path+"/"+output_file_name, std::ios::binary);
            output_stream.write(out_buffer.c_str(), out_size);
            i++;
        }
    }
}

// bool decode_frame_to_image(const std::string& source_file_path, const std::string& output_file_path, const std::string& source_format, const std::string& target_format){
//     fs::path source_path(source_file_path);
//     fs::path output_path(output_file_path);
// }

bool encode_image_to_frame(const std::string& source_file_path, const std::string& output_file_path, const std::string& source_format, const std::string& target_format, const EncoderParameters& parameters, bool single_file){
    fs::path source_path(source_file_path);
    fs::path output_path(output_file_path);

    if(!fs::exists(source_path)){
        throw fs::filesystem_error("file not exist", std::error_code());
    }

    H26xEncoder encoder(target_format);
    encoder.SetWidth(parameters.width);
    encoder.SetHeight(parameters.height);
    encoder.SetInputPixelFormat(parameters.input_pixel_format);
    encoder.SetGopSize(parameters.gop_size);
    encoder.SetFps(parameters.fps);
    encoder.SetRefs(parameters.refs);
    encoder.SetMaxBFrames(parameters.max_b_frames);
    encoder.SetThreadNum(parameters.thread_num);
    encoder.SetOptions(parameters.options);
    encoder.Enable();

    std::vector<fs::path> image_files;   
    for(auto const& dir_entry: fs::directory_iterator(source_path)){
        if(dir_entry.is_regular_file()){
            image_files.emplace_back(dir_entry.path());
        }
    }

    // sort files in alphabetical order.
    std::sort(image_files.begin(), image_files.end());

    std::ofstream output_file;
    if(single_file && !fs::is_directory(output_path)){
        output_file=std::ofstream(output_path.string(), std::ios::binary|std::ios::app);
    }else if(fs::is_directory(output_path)){
        throw fs::filesystem_error("output path can't be a dir when --single setted", std::error_code());
    }

    ConverterRGB24 converter;

    std::vector<std::vector<char>> output_files;
    uint32_t i=0;
    for(fs::path image_path: image_files){
        size_t file_size=fs::file_size(image_path);
        std::ifstream input_image(image_path.string(), std::ios::binary);
        
        std::string buffer(file_size,'\0');
        std::vector<char> output;
        if(source_format=="jpeg" || source_format=="jpg"){
            std::unique_ptr<std::string> rgbframe = converter.from_jpeg(image_path.string());
            buffer = *rgbframe;
        }else{
            input_image.read(&buffer[0], file_size);
        }

        encoder.Encode((uint8_t*)buffer.c_str(), output);

        if(single_file){
            output_file.write(output.data(), output.size());
        }else{
            std::ofstream output_stream(output_path.string()+"/"+std::to_string(i)+"."+target_format, std::ios::binary);
            output_stream.write(output.data(), output.size());
            i++;
        }
    }

    std::vector<char> output;
    encoder.Encode(nullptr, output);
    if(single_file){
        output_file.write(output.data(), output.size());
    }else{
        std::ofstream output_stream(output_path.string()+"/"+std::to_string(i)+"."+target_format, std::ios::binary);
        output_stream.write(output.data(), output.size());
    }
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
        ("width", "image width, only for encoder", cxxopts::value<int>()->default_value("0"))
        ("height", "image height, only for encoder", cxxopts::value<int>()->default_value("0"))
        ("input_pixel_format", "input_pixel_format, only for encoder", cxxopts::value<std::string>()->default_value("RGB24"))
        ("gop_size", "gop size, only for encoder", cxxopts::value<int>()->default_value("0"))
        ("fps", "fps, only for encoder", cxxopts::value<int>()->default_value("25"))
        ("refs", "refs, only for encoder", cxxopts::value<int>()->default_value("0"))
        ("max_b_frames", "max_b_frames, only for encoder", cxxopts::value<int>()->default_value("0"))
        ("thread_num", "thread_num, only for encoder", cxxopts::value<int>()->default_value("4"))
        ("encoder_config", "a json file which include parameters of encoder, only for encoder", cxxopts::value<std::string>()->default_value(" "))
        ("single", "encode to a single file, only for encoder", cxxopts::value<bool>()->default_value("false"))
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
        if(source_format!="jpg" && source_format!="jpeg" && source_format!="png" && source_format!="yuv420p" && source_format!="rgb"){
            throw cxxopts::exceptions::specification("illegal source format");
        }
        if(target_format!="h264" && target_format!="h265" && target_format!="hevc"){
            throw cxxopts::exceptions::specification("illegal target format");
        }
        
        EncoderParameters encoder_parameters;
        if(result["encoder_config"].as<std::string>()!=" "){
            std::ifstream config_file(result["encoder_config"].as<std::string>());
            json data = json::parse(config_file);
            encoder_parameters.width = data["width"];
            encoder_parameters.height = data["height"];
            encoder_parameters.input_pixel_format = data["input_pixel_format"];
            encoder_parameters.gop_size = data["gop_size"];
            encoder_parameters.fps = data["fps"];
            encoder_parameters.refs = data["refs"];
            encoder_parameters.max_b_frames = data["max_b_frames"];
            encoder_parameters.thread_num = data["thread_num"];
        }

        if(result["width"].as<int>()){
            encoder_parameters.width = result["width"].as<int>();
        }
        if(result["height"].as<int>()){
            encoder_parameters.height = result["height"].as<int>();
        }

        if(encoder_parameters.width==0 || encoder_parameters.height==0){
            throw cxxopts::exceptions::specification("width and height must be specified");
        }

        encoder_parameters.input_pixel_format=result["input_pixel_format"].as<std::string>();
        encoder_parameters.gop_size=result["gop_size"].as<int>();
        encoder_parameters.fps=result["fps"].as<int>();
        encoder_parameters.refs=result["refs"].as<int>();
        encoder_parameters.max_b_frames=result["max_b_frames"].as<int>();
        encoder_parameters.thread_num=result["thread_num"].as<int>();

        bool output_single_file = result["single"].as<bool>();
        encode_image_to_frame(result["path"].as<std::string>(), result["output"].as<std::string>(), source_format, target_format, encoder_parameters, output_single_file);
    }

    return 0;
}
