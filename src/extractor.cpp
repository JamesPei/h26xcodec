#include <h26xcodec/extractor.hpp>
#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/bsf.h>
}

Extractor::Extractor(std::string source_path):source_file_path(source_path){}

void Extractor::extract(std::vector<std::string>& output_frames){
    AVFormatContext* fmt_ctx = nullptr;
    if (avformat_open_input(&fmt_ctx, source_file_path.c_str(), nullptr, nullptr) < 0) {
        std::cerr << "无法打开输入文件" << std::endl;
        return;
    }
    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        std::cerr << "无法获取流信息" << std::endl;
        avformat_close_input(&fmt_ctx);
        return;
    }

    int video_stream_index = -1;
    for (unsigned i = 0; i < fmt_ctx->nb_streams; i++) {
        AVStream* s = fmt_ctx->streams[i];
        if (s->codecpar->codec_type != AVMEDIA_TYPE_VIDEO) continue;
        if (s->codecpar->codec_id == AV_CODEC_ID_H264 || s->codecpar->codec_id == AV_CODEC_ID_HEVC) {
            video_stream_index = i;
            break;
        }
    }
    if (video_stream_index < 0) {
        std::cerr << "未找到 H.26x 视频流" << std::endl;
        avformat_close_input(&fmt_ctx);
        return;
    }

    // 从MP4文件中提取H.26x裸流（Elementary Stream）有两种标准格式：Annex B（带起始码）和AVCC（长度前缀); Annex B可以被大多数h26x解码器播放
    // 需要保持MP4内部的原始存储格式（用于分析容器结构或直接写回MP4）使用AVCC
    // BSF: Bitstream Filter
    const char* bsf_name = (fmt_ctx->streams[video_stream_index]->codecpar->codec_id == AV_CODEC_ID_HEVC)
                           ? "hevc_mp4toannexb" : "h264_mp4toannexb";
    const AVBitStreamFilter* bsf = av_bsf_get_by_name(bsf_name);
    if (!bsf) {
        std::cerr << "无法找到 BSF: " << bsf_name << std::endl;
        avformat_close_input(&fmt_ctx);
        return;
    }

    AVBSFContext* bsf_ctx = nullptr;
    if (av_bsf_alloc(bsf, &bsf_ctx) < 0) {
        avformat_close_input(&fmt_ctx);
        return;
    }

    // int avcodec_parameters_copy(AVCodecParameters *dst, const AVCodecParameters *src);
    // 把一个流的编解码参数复制到另一个目标结构，通常是在设置解码器/编码器或BSF的输入参数时使用
    // 把MP4视频流的AVCodecParameters复制进BSF的par_in，让h264_mp4toannexb/hevc_mp4toannexb能：
    // 1. 识别H.264/H.265
    // 2. 使用extradata（SPS/PPS）做 AVCC → Annex B 转换
    // 3. 在首个输出packet前正确插入参数集
    // 如果不做这步，BSF 无法正确处理 extradata，转换结果可能出错
    if (avcodec_parameters_copy(bsf_ctx->par_in, fmt_ctx->streams[video_stream_index]->codecpar) < 0) {
        av_bsf_free(&bsf_ctx);
        avformat_close_input(&fmt_ctx);
        return;
    }

    if (av_bsf_init(bsf_ctx) < 0) {
        av_bsf_free(&bsf_ctx);
        avformat_close_input(&fmt_ctx);
        return;
    }

    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = nullptr;
    pkt.size = 0;

    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index != video_stream_index) {
            av_packet_unref(&pkt);
            continue;
        }
        if (av_bsf_send_packet(bsf_ctx, &pkt) < 0) {
            av_packet_unref(&pkt);
            break;
        }
        av_packet_unref(&pkt);

        AVPacket annexb_pkt;
        av_init_packet(&annexb_pkt);
        annexb_pkt.data = nullptr;
        annexb_pkt.size = 0;
        // int av_bsf_receive_packet(AVBSFContext *ctx, AVPacket *pkt);
        // 从BSF取出一条已处理过的packet
        while (av_bsf_receive_packet(bsf_ctx, &annexb_pkt) == 0) {
            std::string frame(reinterpret_cast<const char*>(annexb_pkt.data), annexb_pkt.size);
            output_frames.push_back(std::move(frame));
            av_packet_unref(&annexb_pkt);
        }
    }

    AVPacket eos;
    av_init_packet(&eos);
    eos.data = nullptr;
    eos.size = 0;
    av_bsf_send_packet(bsf_ctx, &eos);

    AVPacket annexb_pkt;
    av_init_packet(&annexb_pkt);
    annexb_pkt.data = nullptr;
    annexb_pkt.size = 0;
    while (av_bsf_receive_packet(bsf_ctx, &annexb_pkt) == 0) {
        std::string frame(reinterpret_cast<const char*>(annexb_pkt.data), annexb_pkt.size);
        output_frames.push_back(std::move(frame));
        av_packet_unref(&annexb_pkt);
    }

    av_bsf_free(&bsf_ctx);
    avformat_close_input(&fmt_ctx);
}

void Extractor::extract_decoded(std::function<void(const AVFrame&)> on_frame) {
    AVFormatContext* fmt_ctx = nullptr;
    if (avformat_open_input(&fmt_ctx, source_file_path.c_str(), nullptr, nullptr) < 0) {
        std::cerr << "无法打开输入文件" << std::endl;
        return;
    }
    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        std::cerr << "无法获取流信息" << std::endl;
        avformat_close_input(&fmt_ctx);
        return;
    }

    int video_stream_index = -1;
    for (unsigned i = 0; i < fmt_ctx->nb_streams; i++) {
        AVStream* s = fmt_ctx->streams[i];
        if (s->codecpar->codec_type == AVMEDIA_TYPE_VIDEO &&
            (s->codecpar->codec_id == AV_CODEC_ID_H264 || s->codecpar->codec_id == AV_CODEC_ID_HEVC)) {
            video_stream_index = i;
            break;
        }
    }
    if (video_stream_index < 0) {
        std::cerr << "未找到 H.26x 视频流" << std::endl;
        avformat_close_input(&fmt_ctx);
        return;
    }

    const AVCodec* codec = avcodec_find_decoder(fmt_ctx->streams[video_stream_index]->codecpar->codec_id);
    if (!codec) {
        std::cerr << "无法找到解码器" << std::endl;
        avformat_close_input(&fmt_ctx);
        return;
    }

    AVCodecContext* ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        avformat_close_input(&fmt_ctx);
        return;
    }
    if (avcodec_parameters_to_context(ctx, fmt_ctx->streams[video_stream_index]->codecpar) < 0) {
        avcodec_free_context(&ctx);
        avformat_close_input(&fmt_ctx);
        return;
    }
    if (avcodec_open2(ctx, codec, nullptr) < 0) {
        avcodec_free_context(&ctx);
        avformat_close_input(&fmt_ctx);
        return;
    }

    AVFrame* frame = av_frame_alloc();
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = nullptr;
    pkt.size = 0;
    if (!frame) {
        avcodec_free_context(&ctx);
        avformat_close_input(&fmt_ctx);
        return;
    }

    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index != video_stream_index) {
            av_packet_unref(&pkt);
            continue;
        }
        if (avcodec_send_packet(ctx, &pkt) < 0) {
            av_packet_unref(&pkt);
            break;
        }
        av_packet_unref(&pkt);
        while (avcodec_receive_frame(ctx, frame) == 0) {
            on_frame(*frame);
        }
    }

    avcodec_send_packet(ctx, nullptr);
    while (avcodec_receive_frame(ctx, frame) == 0) {
        on_frame(*frame);
    }

    av_frame_free(&frame);
    avcodec_free_context(&ctx);
    avformat_close_input(&fmt_ctx);
}