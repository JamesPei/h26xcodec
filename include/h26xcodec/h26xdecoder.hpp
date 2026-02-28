#pragma once

#ifndef __H26XCODEC_DECODER__
#define __H26XCODEC_DECODER__

/*
This h26x decoder class  is just a thin wrapper around libav 
functions to decode h26x videos. It would have been easy to use 
libav directly in   the python module code but I like to keep these 
things separate.
 
It is mostly based on roxlu's code. See
http://roxlu.com/2014/039/decoding-h264-and-yuv420p-playback

However, in contrast to roxlu's code the color space conversion is
done by libav functions - so on the CPU, I suppose.

Most functions/members throw exceptions. This way, error states are 
conveniently forwarded to python via the exception translation 
mechanisms of boost::python.  
*/

// for ssize_t (signed int type as large as pointer type)
#include <cstdlib>
#include <stdexcept>
#include <utility>
#include <vector>
#include <memory>
#include "h26xexceptions.hpp"

struct AVCodecContext;
struct AVFrame;
struct AVCodec;
struct AVCodecParserContext;
struct SwsContext;
struct AVPacket;
struct AVFormatContext;

class H26xDecoder
{
  /* Persistent things here, using RAII for cleanup. */
  AVCodecContext        *context;
  AVFrame               *frame;
  AVCodecParserContext  *parser;
  AVFormatContext       *formatContext;
  /* In the documentation example on the github master branch, the 
packet is put on the heap. This is done here to store the pointers 
to the encoded data, which must be kept around  between calls to 
parse- and decode frame. In release 11 it is put on the stack, too. 
  */
  AVPacket              *pkt;
public:
  H26xDecoder(std::string const& decoder_id);
  ~H26xDecoder();
  /* First, parse a continuous data stream, dividing it into 
packets. When there is enough data to form a new frame, decode 
the data and return the frame. parse returns the number 
of consumed bytes of the input stream. It stops consuming 
bytes at frame boundaries.
  */
  ptrdiff_t parse(const unsigned char* in_data, ptrdiff_t in_size);
  bool is_frame_available() const;
  const AVFrame& decode_frame();
  void decode_video(const std::string& video_path, std::vector<std::shared_ptr<AVFrame>>& decoded_frames);
};

void disable_logging();

/* Wrappers, so we don't have to include libav headers. */
std::pair<int, int> width_height(const AVFrame&);
int row_size(const AVFrame&);

/* all the documentation links
 * My version of libav on ubuntu 16 appears to be from the release/11 branch on github
 * Video decoding example: https://libav.org/documentation/doxygen/release/11/avcodec_8c_source.html#l00455
 * https://libav.org/documentation/doxygen/release/9/group__lavc__decoding.html
 * https://libav.org/documentation/doxygen/release/11/group__lavc__parsing.html
 * https://libav.org/documentation/doxygen/release/9/swscale_8h.html
 * https://libav.org/documentation/doxygen/release/9/group__lavu.html
 * https://libav.org/documentation/doxygen/release/9/group__lavc__picture.html
 * http://dranger.com/ffmpeg/tutorial01.html
 */

 #endif