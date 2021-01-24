#ifndef AVDECODECONTEXT_H
#define AVDECODECONTEXT_H

class SwsContext;
class SwrContext;
class AVFrame;
class AVCodec;
class AVCodecContext;
class AVFormatContext;

#include "avpacketqueue.h"
#include <atomic>

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
#define MAX_PACKET_SIZE             (15*1024*1024) /* 音视频加起来最大缓存为15M*/
#define MIN_PAUSE_SLEEP_US          (30*1000)      /* 暂停时最小休眠时间，微秒 */
#define WAIT_PACKET_SLEEP_US        (1*1000)       /* AVPacketQueue中无数据时等待休眠时间，微秒 */

  enum PlayStatus { Reading=0, Playing, Pause };

public:
      // 视频解码上下文
       SwsContext*         img_convert_ctx;
       unsigned char*      img_buf;
       AVFrame*            frame_rgb;
       AVFrame*            frame;
       AVCodec*            img_codec;
       AVCodecContext*     img_codec_ctx;
       int                 img_stream_index;

       // 音频解码上下文
       SwrContext*         aud_convert_ctx;
       unsigned char*      aud_buf;
       AVFrame*            frame_aud;
       AVCodec*            aud_codec;
       AVCodecContext*     aud_codec_ctx;
       int                 aud_stream_index;

       // 文件格式
       AVFormatContext*    fmt_ctx;

       // 视音频包队列
       AVPacketQueue       pck_queue;

       // DTS/PTS 同步控制
       int64_t             start_time,video_pts;

       // 控制命令
       std::atomic_bool    img_thread_quit,aud_thread_quit;
       std::atomic_bool    is_pause,is_eof;

       // 播放状态
       PlayStatus          play_status;

public:
       //SDL2 解码专用

       uint8_t            pcm_buf[AVCODE_MAX_AUDIO_FRAME_SIZE];
       uint8_t*           pcm_buf_pos;
       uint32_t           pcm_buf_len;
};

#endif // AVDECODECONTEXT_H
