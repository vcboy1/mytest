#ifndef AVDECODECONTEXT_H
#define AVDECODECONTEXT_H

class SwsContext;
class SwrContext;
class AVFrame;
class AVCodec;
class AVCodecContext;
class AVFormatContext;


/*****************************************************
 *
 *     解码资源上下文，内部使用
 *
 *****************************************************/
class AVDecodeContext
{

public:
       AVDecodeContext();

       ~AVDecodeContext();

 public:
#define AVCODE_MAX_AUDIO_FRAME_SIZE	192000  /* 1 second of 48khz 32bit audio */
#define MAX_PACKET_SIZE                                  (15*1024*1024)       /* 音视频加起来最大缓存为15M*/

 public:
      // 视频解码上下文
       SwsContext*              img_convert_ctx;
       unsigned char*           img_buf;
       AVFrame*                  frame_rgb;
       AVFrame*                  frame;
       AVCodec*                  img_codec;
       AVCodecContext*     img_codec_ctx;

       // 音频解码上下文
       SwrContext*              aud_convert_ctx;
       unsigned char*           aud_buf;
       AVFrame*                  frame_aud;
       AVCodec*                  aud_codec;
       AVCodecContext*     aud_codec_ctx;

       // 文件格式
       AVFormatContext*    fmt_ctx;
};

#endif // AVDECODECONTEXT_H
