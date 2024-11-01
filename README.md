# h26xcodec
A collection of H264/H265 encoder and decoder.

## Dependency
1. ffmpeg(version>=7.0 recommanded)
2. cxxopts
3. nlohmann_json

## Build
very simple, go into this dir and
```
mkdir build
cd build
cmake ..
make -jN
make install
```

## Usage
```
Usage:
  h26xcodec [OPTION...]

  -h, --help                    print usage
  -d, --decode                  docode h26x to image
  -v, --video                   decode video, only useful with -d
  -f, --frame                   decode frame, only useful with -d
  -e, --encode                  encode image to h26x
  -p, --path arg                file or dir path (default: .)
      --sf arg                  the format of source file, for decode is
                                h264/h265, for encode is
                                jpg/png/yuv420p/rgb (default: h265)
      --tf arg                  the format of target file, for decode is
                                jpg/png/yuv420p/rgb, for encode is
                                h264/h265 (default: jpeg)
  -o, --output arg              output path (default: .)
      --width arg               image width, only for encoder (default: 0)
      --height arg              image height, only for encoder (default: 0)
      --input_pixel_format arg  input_pixel_format, only for encoder
                                (default: RGB24)
      --gop_size arg            gop size, only for encoder (default: 0)
      --fps arg                 fps, only for encoder (default: 25)
      --refs arg                refs, only for encoder (default: 1)
      --max_b_frames arg        max_b_frames, only for encoder (default: 0)
      --thread_num arg          thread_num, only for encoder (default: 4)
      --encoder_config arg      a json file which include parameters of
                                encoder, only for encoder (default:  )
      --single                  encode to a single file, only for encoder
```

## Use json file to set encoder parameters 
Because there are too many configurable parameters, the encoder can put the parameters in a JSON file, and here is templete:
```json
{
 "width": 512,
 "height": 256,
 "input_pixel_format": "rgb",
 "fps": 10,
 "gop_size": 30,
 "refs": 1,
 "max_b_frames": 0,
 "thread_num": 4,
 "options": {
     "preset": "veryfast",
     "crf": "10",
     "tune": "zerolatency"
    }
}
```

## Examples
Suppose you have an H.265 video file named lr30v.h265 and a folder ./testout/encode_test containing multiple JPEG files, a folder ./frames containing multiple h265 frame file
1. decode video to jpg and output to ./testout  
`h26xcodec -d -p lr30v.h265 -o ./testout --tf jpg`
2. decode frames to jpg and output to ./testout  
`h26xcodec -d -f -p ./frames -o ./testout --sf h265 --tf jpg`
3. encode jpg to a h265 video which named lr30v.h265 with configure json file  
`h26xcodec -e -p ./testout/encode_test/ --sf jpg --tf h265 -o lr30v.h265 --encoder_config testcase.json --single`
4. encode jpg to a h265 video which named lr30v.h265 with parameters  
`h26xcodec -e -p testout/encode_test/ --sf jpg --tf h265 -o lr30v.h265 --width 512 --height 256 --fps 10 --gop_size 30 --single --refs 1 --single`
5. encode jpg to h265 frame  
`h26xcodec -e -p ./testout/encode_test/ --sf jpg --tf h265 -o lr30v.h265 --encoder_config testcase.json`