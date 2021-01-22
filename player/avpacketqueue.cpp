#ifdef __cplusplus
extern "C"
{
#endif
#include <libavutil/time.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#ifdef __cplusplus
}
#endif

#include "avpacketqueue.h"
#include "avdecodecontext.h"
#include <thread>


/*****************************************************
 *
 *    Packet出入队列，线程安全
 *
 *****************************************************/
AVPacketQueue::~AVPacketQueue(){
    clear();
}

void        AVPacketQueue::push_video(AVPacket*  p){

    if ( (p = clone(p)) == nullptr )
        return;

    lock_guard<mutex>   g(v_mutex);

    v_queue.push(p);
    total_size += p->size;
}



void       AVPacketQueue::push_audio(AVPacket*  p){

    if ( (p = clone(p)) == nullptr )
        return;

    lock_guard<mutex>   g(a_mutex);

    a_queue.push(p);
    total_size += p->size;
}




AVPacket*         AVPacketQueue::pop_video(){

    lock_guard<mutex>   g(v_mutex);
    AVPacket*                  p = nullptr;

    if (!v_queue.empty() ){

        p                = v_queue.front();

        v_queue.pop();
        total_size -= p->size;
    }
    return p;
}




AVPacket*         AVPacketQueue::pop_audio(){

    lock_guard<mutex>   g(a_mutex);
    AVPacket*                  p = nullptr;

    if (!a_queue.empty() ){

        p                = a_queue.front();
        a_queue.pop();
        total_size -= p->size;
    }
    return p;
}



// 清除视音频队列
void     AVPacketQueue::clear(){

      { // 清除视频队列

            lock_guard<mutex>   g(v_mutex);
            while ( !v_queue.empty() ){

                  AVPacket* p = v_queue.front();
                  av_packet_unref(p);
                  av_packet_free(&p);

                  v_queue.pop();
                  total_size -= p->size;
              }
        }

      { // 清除音频队列

            lock_guard<mutex>   g(a_mutex);
            while ( !a_queue.empty() ){

                  AVPacket* p = a_queue.front();
                  av_packet_unref(p);
                  av_packet_free(&p);

                  a_queue.pop();
                  total_size -= p->size;
              }
      }
}

//复制一个AVPacket
 AVPacket*         AVPacketQueue::clone(AVPacket* src){

     if (src == nullptr)
         return nullptr;

     AVPacket *dst = av_packet_alloc();
     if (av_packet_ref(dst, src < 0) )
         return nullptr;

     return dst;
 }


 /*****************************************************
  *
  *     解码器
  *
 **************************************************** */

 AVDecoder::AVDecoder():is_eof(true){
 }

 AVDecoder::~AVDecoder(){
 }

 // 音频解码线程
 int       AVDecoder::audio_decode(){
     return -1;
 }

  // 视频解码线程
  int      AVDecoder::vedio_decode(){
      return -1;
  }

  //  文件解析线程
  bool   AVDecoder::play(const char *url){

      if  ( url ==nullptr )
          return false;

      AVDecodeContext       R;

      // 注册复用器,编码器等
       av_register_all();
       avformat_network_init();

       // 打开多媒体文件
       if ( avformat_open_input( &(R.fmt_ctx), url , nullptr, nullptr ) < 0 ||
             avformat_find_stream_info(R.fmt_ctx,nullptr) < 0 )
                 return false;

       // 打印有关输入或输出格式的详细信息, 该函数主要用于debug
       av_dump_format( R.fmt_ctx, 0, url , 0 );

       // 查找视音频流
      int   video_stream=-1, audio_stream=-1,i =0;
      for ( i = 0; i < R.fmt_ctx->nb_streams; ++i )

          switch ( R.fmt_ctx->streams[i]->codec->codec_type){

          case AVMEDIA_TYPE_VIDEO:
                       video_stream = i;   break;

          case AVMEDIA_TYPE_AUDIO:
                        audio_stream = i;   break;
          }

       if ( video_stream == -1 && audio_stream == -1)
           return false;

       //----------------- 视频解码准备工作 ------------------

       //获取视频解码器
       if ((R.img_codec_ctx  = R.fmt_ctx->streams[video_stream]->codec) == nullptr)
              return false;

       R.img_codec       = avcodec_find_decoder( R.img_codec_ctx->codec_id);
       if ( R.img_codec == nullptr  ||
             avcodec_open2( R.img_codec_ctx,  R.img_codec,nullptr) <0 )
           return false;

       //分配VideoFrame
        R.frame         = av_frame_alloc();
        R.frame_rgb  = av_frame_alloc();
        if (R.frame  == nullptr || R.frame_rgb == nullptr)
            return false;

        //分配Video缓冲区
        int  out_width =  R.img_codec_ctx->width;
        int  out_height=  R.img_codec_ctx->height;

        int         num_bytes = avpicture_get_size( AV_PIX_FMT_RGB24, out_width,out_height);
        R.img_buf      = (uint8_t*)av_malloc(num_bytes);
        if ( R.img_buf ==nullptr )
            return false;

        avpicture_fill( (AVPicture*) R.frame_rgb, R.img_buf, AV_PIX_FMT_RGB24,out_width, out_height);

        // 分配视频格式转换器
        AVPacket*              packet =  av_packet_alloc();
        int                            frame_finished = 0;
        R.img_convert_ctx = sws_getContext(
                                                                       R.img_codec_ctx->width, R.img_codec_ctx->height,
                                                                       R.img_codec_ctx->pix_fmt,  out_width,    out_height,
                                                                       AV_PIX_FMT_RGB24,SWS_BICUBIC,
                                                                        nullptr,nullptr,nullptr );

        //----------------- 音频解码准备工作 ------------------

         //获取音频解码器
        if  ((R.aud_codec_ctx  = R.fmt_ctx->streams[audio_stream]->codec)== nullptr)
                 return false;

        R.aud_codec       = avcodec_find_decoder( R.aud_codec_ctx->codec_id);
        if ( R.aud_codec == nullptr ||
             avcodec_open2( R.aud_codec_ctx,  R.aud_codec,nullptr) <0)
                return false;

        // 分配AudioFrame
        R.frame_aud      = av_frame_alloc();
        R.aud_buf          = (uint8_t*)av_malloc(AVCODE_MAX_AUDIO_FRAME_SIZE) ;
        if ( R.frame_aud == nullptr || R.aud_buf == nullptr)
              return false;

        // 设置音频采样参数
        AVSampleFormat  in_sample_fmt  = R.aud_codec_ctx->sample_fmt;//采样格式
        AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;

        int in_sample_rate   = R.aud_codec_ctx->sample_rate;  //采样率
        int out_sample_rate = in_sample_rate;

        uint64_t in_ch_layout = R.aud_codec_ctx->channel_layout;//声道布局
        uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;


        // 音频格式转换器
        R.aud_convert_ctx = swr_alloc();
        swr_alloc_set_opts(R.aud_convert_ctx,
                                          out_ch_layout, out_sample_fmt, out_sample_rate,
                                          in_ch_layout,     in_sample_fmt,    in_sample_rate,
                                          0, nullptr);
        swr_init(R.aud_convert_ctx);

        //16bits 44100Hz PCM数据
        int nb_out_channel = av_get_channel_layout_nb_channels(out_ch_layout);

        //------------------- 视音频同步准备工作 ------------------

       //计时器,av_getetime()返回的是以微秒计时的时间
       int64_t                     start_time = av_gettime();
       int64_t                     video_time = 0;

       //------------------- 解码 ------------------
       is_eof = false;
       while ( !is_eof ){

            // 流控:防止解码太快
            if ( pck_queue.size() > MAX_PACKET_SIZE){

                    std::this_thread::yield();
                    continue;
            }

            // 读取AVPacket出错
            if ( av_read_frame(R.fmt_ctx,packet) < 0 ){

                 is_eof = true;
                 continue;
            }

            //-----------  视频解码  -------------
            if ( packet->stream_index == video_stream ){

                pck_queue.push_video( packet);

                 if ( ++i > 300 )
                        is_eof = true;
            }
            else //-----------  音频解码  -------------
            if (packet->stream_index == audio_stream ){

                 pck_queue.push_audio( packet);
            }
            av_packet_unref(packet);

       }

        av_packet_free(&packet);
        return true;
}


