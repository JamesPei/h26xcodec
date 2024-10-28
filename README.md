# h26xcodec
A collection of H264/H265 encoder and decoder.

## Dependency
1. ffmpeg
2. cxxopts

## Build
very simple
```
mkdir build
cd build
cmake ..
make
```

## Usage
```
Usage:
  h26xcodec [OPTION...]

  -h, --help        print usage
  -d, --decode      docode h26x to image
  -v, --video       decode video, only useful with -d
  -f, --frame       decode frame, only useful with -d
  -e, --encode      encode image to h26x
  -c, --convert     convert image format
  -p, --path arg    file or dir path (default: .)
      --sf arg      the format of source file, for decode is h264/h265, for
                    encode is jpg/png/yuv420p/rgb (default: h265)
      --tf arg      the format of target file, for decode is
                    jpg/png/yuv420p/rgb, for encode is h264/h265 (default:
                    jpeg)
  -o, --output arg  output path (default: .)
      --params arg  other parameters (default: .)
```

## Examples
1. decode a video to images

2. encode images to a video

3. convert image to different format