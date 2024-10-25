struct SwsContext;
struct AVFrame;

class ConverterRGB24
{
  SwsContext *context;
  AVFrame *framergb;
  
public:
  ConverterRGB24();
  ~ConverterRGB24();
   
  /*  
    Returns, given a width and height, 
    how many bytes the frame buffer is going to need. 
  */
  int predict_size(int w, int h);
  /*  
    Given a decoded frame, convert it to RGB format and fill 
    out_rgb with the result. Returns a AVFrame structure holding 
    additional information about the RGB frame, such as the number of
    bytes in a row and so on. 
  */
  const AVFrame& convert(const AVFrame &frame, unsigned char* out_rgb);
};
