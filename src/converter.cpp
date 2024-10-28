extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/mem.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <h26xcodec/converter.hpp>
#include <stdexcept>
#include <iostream>
#include <fstream>

ConverterRGB24::ConverterRGB24():context(nullptr),swsContext(nullptr),jpegCodec(avcodec_find_encoder(AV_CODEC_ID_MJPEG))
{
  frameRGB = av_frame_alloc();
  if (!frameRGB)
    throw std::runtime_error("cannot allocate frame");
  jpegContext = avcodec_alloc_context3(jpegCodec);
  jpegContext->pix_fmt = AV_PIX_FMT_YUVJ420P;
  jpegContext->time_base = {1, 25};
  // Allocate a packet for output JPEG
  av_init_packet(&packet);
  packet.data = NULL;
  packet.size = 0;
}

ConverterRGB24::~ConverterRGB24()
{
  av_packet_unref(&packet);
  avcodec_close(jpegContext);
  avcodec_free_context(&jpegContext);
  sws_freeContext(context);
  av_frame_free(&frameYUV);
  av_frame_free(&frameRGB);
}

void ConverterRGB24::convert(const AVFrame &frame, unsigned char* out_image)
{
  int w = frame.width;
  int h = frame.height;
  int pix_fmt = frame.format;
  
  context = sws_getCachedContext(context, 
                                 w, h, (AVPixelFormat)pix_fmt, 
                                 w, h, AV_PIX_FMT_RGB24, SWS_BILINEAR,
                                 nullptr, nullptr, nullptr);
  if (!context)
    throw std::runtime_error("cannot allocate context");
  
  // Setup frameRGB with out_image as external buffer, let frameRGB point to out_image. Also say that we want RGB24 output.
  av_image_fill_arrays(frameRGB->data, frameRGB->linesize, out_image, AV_PIX_FMT_RGB24, w, h, 1);
  // Do the conversion.
  sws_scale(context, frame.data, frame.linesize, 0, h,
            frameRGB->data, frameRGB->linesize);
  frameRGB->width = w;
  frameRGB->height = h;
}

/*
Determine required size of framebuffer.

avpicture_get_size is used in http://dranger.com/ffmpeg/tutorial01.html 
to do this. However, avpicture_get_size returns the size of a compact 
representation, without padding bytes. Since we use av_image_fill_arrays to
fill the buffer we should also use it to determine the required size.
*/
int ConverterRGB24::predict_size(int w, int h)
{
  return av_image_fill_arrays(frameRGB->data, frameRGB->linesize, nullptr, AV_PIX_FMT_RGB24, w, h, 1);
}

std::unique_ptr<std::string> ConverterRGB24::to_jpeg() {
    jpegContext->height = frameRGB->height;
    jpegContext->width = frameRGB->width;

    if (avcodec_open2(jpegContext, jpegCodec, NULL) < 0) {
        std::cerr << "Could not open JPEG codec." << std::endl;
        throw std::runtime_error("Could not open JPEG codec.");
    }

    // 转换 RGB24 -> YUVJ420P
    swsContext = sws_getContext(
        frameRGB->width, frameRGB->height, AV_PIX_FMT_RGB24, // 输入格式
        jpegContext->width, jpegContext->height, AV_PIX_FMT_YUVJ420P, // 输出格式
        SWS_BICUBIC, NULL, NULL, NULL);

    if (!swsContext) {
        std::cerr << "Could not initialize the conversion context." << std::endl;
        throw std::runtime_error("Could not initialize the conversion context.");
    }

    // 创建目标 AVFrame 用于存储 YUVJ420P 格式的数据
    frameYUV = av_frame_alloc();
    frameYUV->format = AV_PIX_FMT_YUVJ420P;
    frameYUV->width = jpegContext->width;
    frameYUV->height = jpegContext->height;
    av_frame_get_buffer(frameYUV, 32);

    // 将 RGB24 数据转换为 YUVJ420P
    sws_scale(
        swsContext,
        frameRGB->data, frameRGB->linesize, 0, frameRGB->height,
        frameYUV->data, frameYUV->linesize);

    int ret = avcodec_send_frame(jpegContext, frameYUV);
    if (ret < 0) {
        std::cerr << "Error sending frame to JPEG codec." << std::endl;
        throw std::runtime_error("Error sending frame to JPEG codec.");
    }

    ret = avcodec_receive_packet(jpegContext, &packet);
    if (ret == 0) {
      return std::make_unique<std::string>((char*)packet.data, packet.size);
    }
}