#include <h26xcodec/extractor.hpp>

Extractor::Extractor(std::string source_path):source_file_path(source_path){}

void Extractor::extract(std::string target_format){
    // AVFormatContext* fmt_ctx = NULL;
    // int ret = avformat_open_input(&fmt_ctx, source_file_path, NULL, NULL);
    
    // if (ret < 0) {
    //     std::cout << "无法打开输入文件\n" << std::endl;
    //     return ret;
    // }

    // ret = avformat_find_stream_info(fmt_ctx, NULL);
    // if (ret < 0) {
    //     std::cout << "无法获取流信息\n" << std::endl;
    //     return ret;
    // }
    
    // // 找到 H.26x 视频流
    // int video_stream_index = -1;
    // for (int i = 0; i < fmt_ctx->nb_streams; i++) {
    //     AVStream* stream = fmt_ctx->streams[i];
    //     if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO &&
    //         (stream->codecpar->codec_id == AV_CODEC_ID_HEVC || 
    //          stream->codecpar->codec_id == AV_CODEC_ID_H264)) {
    //         video_stream_index = i;
    //         break;
    //     }
    // }
    
    // if (video_stream_index == -1) {
    //     std::cout << "未找到 H.26x 视频流\n" << std::endl;
    //     return -1;
    // }
    
    // // 逐包读取并写入 H.26x 数据
    // AVPacket pkt;
    // av_init_packet(&pkt);
    // pkt.data = NULL;
    // pkt.size = 0;
    
    // while (av_read_frame(fmt_ctx, &pkt) >= 0) {
    //     if (pkt.stream_index == video_stream_index) {
    //     }
    //     av_packet_unref(&pkt);
    // }
}