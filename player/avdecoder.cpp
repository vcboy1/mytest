
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavutil/time.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <SDL2/include/SDL.h>
#ifdef __cplusplus
}
#endif

#include "avdecoder.h"
#include "impl/avpacketqueue.h"
#include "impl/avdecodecontext.h"
#include "impl/avdecodecontroller.h"
#include <thread>
#include <qdebug>
#include <qimage>
#include <QApplication>

/*****************************************************
 *
 *   SDL2音频解码回调函数
 *
*****************************************************/

static  void  sdl2_fill_audio(void *udata,Uint8 *stream,int len){

    //std::thread::id  id = std::this_thread::get_id();
    //qDebug()<< "   fill audio thread: "  <<  *(uint32_t*)&id;

   AVDecodeContext*   R = (AVDecodeContext*)udata;

   SDL_memset(stream, 0, len);
   if( R->pcm_buf_len==0)
           return;
   len=(len > R->pcm_buf_len? R->pcm_buf_len:len);


   SDL_MixAudio(stream,R->pcm_buf_pos ,len,SDL_MIX_MAXVOLUME);

   R->pcm_buf_pos += len;
   R->pcm_buf_len  -= len;

   //qDebug()<< "   fill audio thread=>  write bytes: "  << len << " buf_len:" << R->pcm_buf_len;
}

/*****************************************************
 *
 *     解码器
 *
*****************************************************/

AVDecoder::AVDecoder(QObject *parent):
    QObject(parent){

       controller = new AVDecodeController();
}

AVDecoder::~AVDecoder(){

    delete controller;
}

// 音频解码线程
int       AVDecoder::audio_decode(void*  ctx){

    AVDecodeContext&     R = *(AVDecodeContext*)ctx;
    AVDecodeController& C = *(AVDecodeController*)controller;
    int             frame_finished = 0;

    //16bits 44100Hz PCM数据
//  int nb_out_channel = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    int nb_out_channel = av_get_channel_layout_nb_channels(R.aud_codec_ctx->channel_layout);
    int bytes_per_sec  = 2 * nb_out_channel * R.aud_codec_ctx->sample_rate;



     qDebug() << "  ******* audio thread start ******* " << bytes_per_sec;
    //------------------- 音频解码 ------------------
   while ( C.cmd != AVDecodeController::STOP ){

          // 如果是暂停状态，不解码休眠
          if ( C.cmd ==  AVDecodeController::PAUSE){

             av_usleep(MIN_PAUSE_SLEEP_US);
             continue;
          }

          // 从队列中取出一个Packet
          AVPacket* pck = R.pck_queue.pop_audio();
          if ( pck == nullptr ){

              // 读完退出
              if (R.is_eof )
                 break;

              // 还未放入数据，先休眠再取
              av_usleep(WAIT_PACKET_SLEEP_US);
              continue;
          }

          int  conv_bytes = avcodec_decode_audio4( R.aud_codec_ctx, R.frame_aud,&frame_finished,pck);
          if ( frame_finished ){

               //还原成PCM数据
                conv_bytes =  swr_convert(
                                         R.aud_convert_ctx,
                                         (uint8_t **)&R.aud_buf,
                                          R.frame_aud->nb_samples,
                                          (const uint8_t **)R.frame_aud->data,
                                          R.frame_aud->nb_samples);

                int data_size = av_samples_get_buffer_size(
                                                           NULL,
                                                           nb_out_channel ,
                                                           R.frame_aud->nb_samples,
                                                           AV_SAMPLE_FMT_S16,
                                                           1);


                // copy to pcm raw buffer
                memcpy( R.pcm_buf, R.aud_buf ,data_size);
                R.pcm_buf_pos = R.pcm_buf;
                R.pcm_buf_len = data_size;

                // 计算音频当前播放的PTS
                R.update_aud_pts(pck);

                while (R.pcm_buf_len > 0 )
                   av_usleep(AV_TIME_BASE/R.aud_codec_ctx->sample_rate*10);

                // 音频播放同步：如果解码速度太快，延时
                R.aud_sync();
//qDebug()<< " samples:" << R.frame_aud->nb_samples;
          }

          // 释放
          av_packet_unref(pck);
          av_packet_free(&pck);
    }


    R.aud_thread_quit = true;
    qDebug() << "  ******* audio thread over *******";
    return 0;
}

 // 视频解码线程
 int    AVDecoder::vedio_decode(void*  ctx){

     AVDecodeContext&     R = *(AVDecodeContext*)ctx;
     AVDecodeController& C = *(AVDecodeController*)controller;
     int             frame_finished = 0;
     int             vf_cnt=0;

     qDebug() << "  ******* video thread start *******";
     while ( C.cmd != AVDecodeController::STOP ){

          // 如果是暂停状态，不解码休眠
          if ( C.cmd ==  AVDecodeController::PAUSE){

                 av_usleep(MIN_PAUSE_SLEEP_US);
                 continue;
          }

          // 从队列中取出一个Packet
          AVPacket* pck = R.pck_queue.pop_video();
          if ( pck == nullptr ){

              // 读完退出
              if (R.is_eof )
                 break;

              // 还未放入数据，先休眠再取
              av_usleep(WAIT_PACKET_SLEEP_US);
              continue;
          }


          //std::thread::id  id = std::this_thread::get_id();
          //qDebug()<< "<== pop_video: " << ++vf_cnt << " thread:" <<  *(uint32_t*)&id;

          // 解码packet
          avcodec_decode_video2(R.img_codec_ctx, R.frame,&frame_finished, pck);

          // 计算音频当前播放的PTS
          R.update_img_pts(pck);

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
              R.img_sync();
          }
          // 释放
          av_packet_unref(pck);
          av_packet_free(&pck);
     }

     R.img_thread_quit = true;
    qDebug() << "  ******* video thread over *******";
     return 0;
 }

 //  文件解析线程
 bool   AVDecoder::play(std::string  url){

     if ( url.empty() )
         return false;

     // 如果还在播放中，先停止
     stop();

     // 打开新文件
     std::thread   decodec_thread( &AVDecoder::format_decode,this,url );
     decodec_thread.detach();
     return true;
}

 bool   AVDecoder::format_decode(std::string file){

      const char* url = file.c_str();

     // 初始化内部控制类
     AVDecodeContext       R;   //析构时释放资源
     AVDecodeController& C = *(AVDecodeController*)controller;
     C.open(&R);

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
       int out_sample_rate  = in_sample_rate;

       uint64_t in_ch_layout = R.aud_codec_ctx->channel_layout;//声道布局
       uint64_t out_ch_layout= in_ch_layout;
       //uint64_t out_ch_layout= AV_CH_LAYOUT_STEREO;


       // 音频格式转换器
       R.aud_convert_ctx = swr_alloc();
       swr_alloc_set_opts(R.aud_convert_ctx,
                                         out_ch_layout, out_sample_fmt, out_sample_rate,
                                         in_ch_layout,  in_sample_fmt,  in_sample_rate,
                                         0, nullptr);
       swr_init(R.aud_convert_ctx);

       //16bits 44100Hz PCM数据
       int nb_out_channel = av_get_channel_layout_nb_channels(out_ch_layout);

       //------------------- SDL2初始化 ------------------
       SDL_AudioSpec wanted_spec;

       wanted_spec.freq      = R.aud_codec_ctx->sample_rate;
       wanted_spec.format    = AUDIO_S16SYS;
       wanted_spec.channels  = R.aud_codec_ctx->channels;
       wanted_spec.silence   = 0;
       wanted_spec.callback  = sdl2_fill_audio;
       wanted_spec.userdata  = (void*)&R;
       // 编码器是否支持变长音频编码
       if ( R.aud_codec_ctx->frame_size > 0 )
           wanted_spec.samples   =  R.aud_codec_ctx->frame_size;
       else
           wanted_spec.samples   =  1024;

       if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER) ||
          SDL_OpenAudio(&wanted_spec, NULL)<0   )
            return false;

       SDL_PauseAudio(0);

      //------------------- 解码 ------------------
      R.img_thread_quit = false;
      R.is_eof          = false;

      //------------------- 启动解码线程 ------------------
      std::thread    audio_thread( &AVDecoder::audio_decode,this,(void*)&R );
      std::thread    video_thread( &AVDecoder::vedio_decode,this,(void*)&R );



      //------------------- 视音频同步准备工作 ------------------
      R.init_pts();

      while ( C.cmd  != AVDecodeController::STOP  ){

           // 如果是暂停命令，或者缓存队列满
          if ( C.cmd == AVDecodeController::PAUSE  ||
                R.pck_queue.size() > MAX_PACKET_SIZE){

               av_usleep(AV_TIME_BASE/10);
               continue;
           }

           // 读取AVPacket出错
           if ( av_read_frame(R.fmt_ctx,packet) < 0 ){

                R.is_eof = true;
                break;
           }

           //-----------  视频解码  -------------
           if ( packet->stream_index == R.img_stream_index ){

                R.pck_queue.push_video( packet);

                //qDebug()<< "push_video: " << (++vf_cnt);
               /*
                  if ( ++vf_cnt > 300*3 ){

                    R.is_eof = true;
                    continue;
                }
                */
           }
           else //-----------  音频解码  -------------
           if (packet->stream_index == R.aud_stream_index ){

                 R.pck_queue.push_audio( packet);
           }
           av_packet_unref(packet);

      }

      av_packet_free(&packet);
      while (!R.img_thread_quit || !R.aud_thread_quit )
          av_usleep(1000);

      video_thread.join();
      audio_thread.join();

      SDL_CloseAudio();
      SDL_Quit();

      emit onStop();

      qDebug() << "  ******* decode thread over *******";
      return true;
}


 // 停止播放视频
 void   AVDecoder::stop(){

     AVDecodeController& C = *(AVDecodeController*)controller;
     C.stop();

     // 等待当前解码线程结束
     while (  C.isOpen() )
         std::this_thread::yield();

     qDebug() << "  ******* main thread over *******";

}

 // 暂停播放
 void   AVDecoder::pause(){

     AVDecodeController& C = *(AVDecodeController*)controller;
     C.pause();
 }

 // 恢复播放
 void   AVDecoder::resume(){

     AVDecodeController& C = *(AVDecodeController*)controller;
     C.play();
 }

 // 是否打开
 bool  AVDecoder::isOpen() const{

     AVDecodeController& C = *(AVDecodeController*)controller;
     return C.isOpen();
}

 // 是否暂停状态
 bool  AVDecoder::isPaused() const{

     AVDecodeController& C = *(AVDecodeController*)controller;
     return C.isPaused();
 }

 // 是否播放状态
 bool  AVDecoder::isPlaying() const{

     AVDecodeController& C = *(AVDecodeController*)controller;
     return C.isPlaying();
 }
