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

ConverterRGB24::ConverterRGB24()
{
  framergb = av_frame_alloc();
  if (!framergb)
    throw std::runtime_error("cannot allocate frame");
  context = nullptr;
}

ConverterRGB24::~ConverterRGB24()
{
  sws_freeContext(context);
  av_frame_free(&framergb);
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
  
  // Setup framergb with out_image as external buffer, let framergb point to out_image. Also say that we want RGB24 output.
  av_image_fill_arrays(framergb->data, framergb->linesize, out_image, AV_PIX_FMT_RGB24, w, h, 1);
  // Do the conversion.
  sws_scale(context, frame.data, frame.linesize, 0, h,
            framergb->data, framergb->linesize);
  framergb->width = w;
  framergb->height = h;
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
  return av_image_fill_arrays(framergb->data, framergb->linesize, nullptr, AV_PIX_FMT_RGB24, w, h, 1);
}