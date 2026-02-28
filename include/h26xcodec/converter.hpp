#pragma once

#ifndef __H26XCODEC_CONVERTOR__
#define __H26XCODEC_CONVERTOR__

#include <memory>

struct SwsContext;
struct AVFrame;

class Converter{
public:
  Converter() = default;
  virtual ~Converter(){};

  /*  
    Returns, given a width and height, 
    how many bytes the frame buffer is going to need. 
  */
  virtual int predict_size(int w, int h){ return -1; };
  /*  
    Given a decoded frame, convert it to RGB format and fill 
    out_rgb with the result. Returns a AVFrame structure holding 
    additional information about the RGB frame, such as the number of
    bytes in a row and so on. 
  */
  virtual void convert(const AVFrame &frame, unsigned char* out_image){};
};

class ConverterRGB24: public Converter
{
public:
  ConverterRGB24();
  ~ConverterRGB24();

  int predict_size(int w, int h) override;
  void convert(const AVFrame &frame, unsigned char* out_image) override;
  std::unique_ptr<std::string> to_jpeg();
  std::unique_ptr<std::string> from_jpeg(std::string jpeg_path);

private:
  SwsContext *context;
  SwsContext *swsContext;
  AVFrame *frameRGB;
  const AVCodec* jpegCodec;
  AVCodecContext* jpegContext;
  AVPacket packet;
};

#endif