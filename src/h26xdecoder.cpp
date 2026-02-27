extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/mem.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <h26xcodec/h26xdecoder.hpp>
#include <h26xcodec/converter.hpp>
#include <tuple>
#include <chrono>
#include <fstream>

typedef unsigned char ubyte;

/* For backward compatibility with release 9 or so of libav */
#if (LIBAVCODEC_VERSION_MAJOR <= 54) 
#  define av_frame_alloc avcodec_alloc_frame
#  define av_frame_free  avcodec_free_frame  
#endif


H26xDecoder::H26xDecoder(std::string const& decoder_id):formatContext(nullptr)
{
  AVCodecID codec_id = AV_CODEC_ID_NONE;
  if(decoder_id == "h264")
      codec_id = AV_CODEC_ID_H264;
  else if(decoder_id == "h265")
      codec_id = AV_CODEC_ID_H265;
  else
  {
      std::string msg = std::string("Unknown decoder id ") + decoder_id;
      throw H26xInitFailure(msg.c_str());
  }

  const AVCodec* codec = avcodec_find_decoder(codec_id);
  if (!codec)
    throw H26xInitFailure("cannot find decoder");
  
  context = avcodec_alloc_context3(codec);
  if (!context)
    throw H26xInitFailure("cannot allocate context");

  // Note: CODEC_CAP_TRUNCATED was prefixed with AV_...
  if(codec->capabilities & AV_CODEC_FLAG2_CHUNKS) {
    context->flags |= AV_CODEC_FLAG2_CHUNKS;
  }  

  int err = avcodec_open2(context, codec, nullptr);
  if (err < 0)
    throw H26xInitFailure("cannot open context");

  parser = av_parser_init(codec_id);
  if (!parser)
    throw H26xInitFailure("cannot init parser");
  
  frame = av_frame_alloc();
  if (!frame)
    throw H26xInitFailure("cannot allocate frame");

  pkt = new AVPacket;
  if (!pkt)
    throw H26xInitFailure("cannot allocate packet");
  av_init_packet(pkt);
}

H26xDecoder::~H26xDecoder()
{
  av_parser_close(parser);
  avcodec_free_context(&context);
  av_free(context);
  av_frame_free(&frame);
  delete pkt;
}


ptrdiff_t H26xDecoder::parse(const ubyte* in_data, ptrdiff_t in_size)
{
  auto nread = av_parser_parse2(parser, context, &pkt->data, &pkt->size, 
                                in_data, in_size, 
                                0, 0, AV_NOPTS_VALUE);
  return nread;
}


bool H26xDecoder::is_frame_available() const
{
  return pkt->size > 0;
}

const AVFrame& H26xDecoder::decode_frame()
{
#if (LIBAVCODEC_VERSION_MAJOR > 56)
  int ret;
  if (pkt) {
    ret = avcodec_send_packet(context, pkt);
    if (!ret) {
      ret = avcodec_receive_frame(context, frame);
      if (!ret)
        return *frame;
    }
  }
  throw H26xDecodeFailure("error decoding frame");
#else
  int got_picture = 0;
  int nread = avcodec_decode_video2(context, frame, &got_picture, pkt);
  if (nread < 0 || got_picture == 0)
    throw H26xDecodeFailure("error decoding frame\n");
  return *frame;
#endif
}

void H26xDecoder::decode_video(const std::string& video_path, std::vector<std::shared_ptr<AVFrame>>& decoded_frames){
  // Open input file
  int error_code = avformat_open_input(&formatContext, video_path.c_str(), nullptr, nullptr);
  if(error_code<0){
    throw H26xDecodeFailure("could't open video");
  };
  if(!formatContext){
    throw H26xDecodeFailure("could't open video");    
  }

  // Find stream information
  if(avformat_find_stream_info(formatContext, NULL) < 0){
    throw H26xDecodeFailure("could't find stream information");
  }

  // Find the first video stream
  int videoStreamIndex = -1;
  for(uint32_t i=0; i<formatContext->nb_streams; i++){
    if(formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
      videoStreamIndex = i;
      break;
    }
  }

  if(videoStreamIndex == -1){
    throw H26xDecodeFailure("could't find a video stream");
  }

  // Get the codec parameters for the video stream
  const AVCodecParameters* codecpar = formatContext->streams[videoStreamIndex]->codecpar;
  const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
  avcodec_parameters_to_context(context, codecpar);
  if (avcodec_open2(context, codec, NULL) < 0) {
      throw H26xDecodeFailure("could't open codec");    
  }

  // Allocate frames
  AVFrame* frame = av_frame_alloc();
  AVFrame* frameRGB = av_frame_alloc();

  if (!frame || !frameRGB) {
      throw H26xDecodeFailure("could't allocate frame");    
  }
  // Allocate buffer for the RGB frame
  int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, context->width, context->height, 1);
  uint8_t* buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));

  av_image_fill_arrays(frame->data, frame->linesize, buffer, AV_PIX_FMT_RGB24, context->width, context->height, 1);

  while (av_read_frame(formatContext, pkt) >= 0)
  {
    if(pkt->stream_index == videoStreamIndex){
      int response = avcodec_send_packet(context, pkt);
      if (response >= 0) {
          while (avcodec_receive_frame(context, frame) >= 0) {
              decoded_frames.push_back(std::make_shared<AVFrame>(*frame));
          }
      }
    }
    av_packet_unref(pkt);
  }
}

std::pair<int, int> width_height(const AVFrame& f)
{
  return std::make_pair(f.width, f.height);
}

int row_size(const AVFrame& f)
{
  return f.linesize[0];
}


void disable_logging()
{
  av_log_set_level(AV_LOG_QUIET);
}
