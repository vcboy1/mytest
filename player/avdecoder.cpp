
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

#include "avdecoder.h"
#include "impl/avpacketqueue.h"
#include "impl/avdecodecontext.h"
#include <thread>
#include <qdebug>
#include <qimage>
#include <QApplication>
/*****************************************************
 *
 *     解码器
 *
**************************************************** */

AVDecoder::AVDecoder(QObject *parent):QObject(parent),is_eof(true){
}

AVDecoder::~AVDecoder(){
}

// 音频解码线程
int       AVDecoder::audio_decode(){
    return -1;
}

 // 视频解码线程
 int      AVDecoder::vedio_decode(void*  ctx){

     AVDecodeContext& R = *(AVDecodeContext*)ctx;
     int             frame_finished = 0;
     int             vf_cnt=0;

     // PTS
     R.video_pts = 0;

     while ( true ){

          // 从队列中取出一个Packet
          AVPacket* pck = R.pck_queue.pop_video();
          if ( pck == nullptr ){

              // 读完退出
              if (R.is_eof )
                 break;

              // 还未放入数据，先休眠再取
              av_usleep(1000);
              continue;
          }


          std::thread::id  id = std::this_thread::get_id();
          qDebug()<< "<== pop_video: " << ++vf_cnt << " thread:" <<  *(uint32_t*)&id;

          // 解码packet
          avcodec_decode_video2(R.img_codec_ctx, R.frame,&frame_finished, pck);

          // 视频同步控制：更新pts
          int  pts = 0;
          if  (pck->dts == AV_NOPTS_VALUE && R.frame->opaque && *(uint64_t*) R.frame->opaque != AV_NOPTS_VALUE)
          {
              pts = *(uint64_t *) R.frame->opaque;
          }
          else  if  (pck->dts != AV_NOPTS_VALUE)
          {
              pts = pck->dts;
          }

          // video_pts* base = 以秒计数的显示时间戳, 再乘以AV_TIME_BASE转换为微秒
          R.video_pts = pts *  av_q2d(R.fmt_ctx->streams[R.img_stream_index]->time_base) * AV_TIME_BASE ;
          //qDebug()<< "dts: " << packet->dts << "  pts:" <<video_pts/1000 <<" time:"<< video_time/(double)AV_TIME_BASE;


          // Frame就绪
          if ( frame_finished ){

              sws_scale(R.img_convert_ctx,
                        R.frame->data,     R.frame->linesize,
                        0,                 R.img_codec_ctx->height,
                        R.frame_rgb->data, R.frame_rgb->linesize);

              // Frame转QImage
              emit onPlay( new QImage ( (uchar *)R.img_buf, R.img_codec_ctx->width,
                                         R.img_codec_ctx->height,QImage::Format_RGB888));

              // 视频同步控制：如果解码速度太快，延时
             int64_t real_time = av_gettime() - R.start_time;  //主时钟时间
             if  ( R.video_pts > real_time )
                 av_usleep( R.video_pts - real_time);
          }
          // 释放
          av_packet_unref(pck);
          av_packet_free(&pck);
     }
     R.img_thread_quit = true;
     return -1;
 }

 //  文件解析线程
 bool   AVDecoder::play(std::string  url){
/*
     std::thread   decodec_thread( &AVDecoder::playImpl,this,url );

     decodec_thread.detach();
     return true;
 */
     playImpl(url);
 }

 bool   AVDecoder::playImpl(std::string file){

     //if  ( url ==nullptr )
     if ( file.empty() )
         return false;

     const char* url = file.c_str();

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
     uint32_t i;
     for ( i = 0; i < R.fmt_ctx->nb_streams; ++i )

         switch ( R.fmt_ctx->streams[i]->codec->codec_type){

         case AVMEDIA_TYPE_VIDEO:
                     R.img_stream_index = i;   break;

         case AVMEDIA_TYPE_AUDIO:
                     R.aud_stream_index = i;   break;
         }

      if ( R.img_stream_index == -1 && R.aud_stream_index == -1)
          return false;

      //----------------- 视频解码准备工作 ------------------

      //获取视频解码器
      if ((R.img_codec_ctx  = R.fmt_ctx->streams[R.img_stream_index]->codec) == nullptr)
             return false;

      R.img_codec       = avcodec_find_decoder( R.img_codec_ctx->codec_id);
      if ( R.img_codec == nullptr  ||
            avcodec_open2( R.img_codec_ctx,  R.img_codec,nullptr) <0 )
          return false;

      //分配VideoFrame
       R.frame      = av_frame_alloc();
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
       R.img_convert_ctx = sws_getContext(
                                      R.img_codec_ctx->width, R.img_codec_ctx->height,
                                      R.img_codec_ctx->pix_fmt,  out_width,    out_height,
                                      AV_PIX_FMT_RGB24,SWS_BICUBIC,
                                       nullptr,nullptr,nullptr );

       //----------------- 音频解码准备工作 ------------------

        //获取音频解码器
       if  ((R.aud_codec_ctx  = R.fmt_ctx->streams[R.aud_stream_index]->codec)== nullptr)
                return false;

       R.aud_codec       = avcodec_find_decoder( R.aud_codec_ctx->codec_id);
       if ( R.aud_codec == nullptr ||
            avcodec_open2( R.aud_codec_ctx,  R.aud_codec,nullptr) <0)
               return false;

       // 分配AudioFrame
       R.frame_aud      = av_frame_alloc();
       R.aud_buf        = (uint8_t*)av_malloc(AVCODE_MAX_AUDIO_FRAME_SIZE) ;
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
       int vf_cnt = 0,nb_out_channel = av_get_channel_layout_nb_channels(out_ch_layout);

      //------------------- 视音频同步准备工作 ------------------

      //计时器,av_getetime()返回的是以微秒计时的时间
      R.start_time = av_gettime();

      //------------------- 启动解码线程 ------------------
      std::thread    video_thread( &AVDecoder::vedio_decode,this,(void*)&R );

      //------------------- 解码 ------------------
      R.img_thread_quit = false;
      R.is_eof          = false;
      while ( !R.is_eof ){

           // 流控:防止解码太快
           if ( R.pck_queue.size() > MAX_PACKET_SIZE){

               //qDebug() << "====== buffer full =======" ;
                qApp->processEvents();
                   std::this_thread::yield();
                   continue;
           }

           // 读取AVPacket出错
           if ( av_read_frame(R.fmt_ctx,packet) < 0 ){

                R.is_eof = true;
                continue;
           }

           //-----------  视频解码  -------------
           if ( packet->stream_index == R.img_stream_index ){

                R.pck_queue.push_video( packet);

                //qDebug()<< "push_video: " << (++vf_cnt);
                if ( ++vf_cnt > 300 ){

                    R.is_eof = true;
                    continue;
                }
           }
           else //-----------  音频解码  -------------
           if (packet->stream_index == R.aud_stream_index ){

                R.pck_queue.push_audio( packet);
           }
           av_packet_unref(packet);

      }

      av_packet_free(&packet);

      while (!R.img_thread_quit )
          qApp->processEvents();

      video_thread.join();
      return true;
}