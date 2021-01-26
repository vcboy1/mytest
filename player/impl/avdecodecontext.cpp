/* 必须记住，使用C++编译器时一定要使用extern "C"，否则找不到链接文件 */
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

#include "avdecodecontext.h"

AVDecodeContext::AVDecodeContext(){

           img_thread_quit = false;
           aud_thread_quit = false;

           is_pause= false;
           is_eof  = false;

           start_time = video_pts = audio_pts = 0;
           img_stream_index = aud_stream_index = -1;

           img_buf                 = aud_buf = 0;
           img_convert_ctx         = 0;
           aud_convert_ctx         = 0 ;
           frame_rgb = frame= frame_aud = 0;
           img_codec               = aud_codec = 0;
           img_codec_ctx           = aud_codec_ctx = 0 ;
           fmt_ctx                 = 0;

           pcm_buf_pos             = pcm_buf;
           pcm_buf_len             = 0;
 }

 AVDecodeContext::~AVDecodeContext(){

           if ( img_convert_ctx )      sws_freeContext(img_convert_ctx);
           if ( aud_convert_ctx)       swr_free(&aud_convert_ctx);

           if ( img_buf)               av_free(img_buf);
           if ( aud_buf)               av_free(aud_buf);

           if ( frame_rgb)             av_frame_free(&frame_rgb);
           if ( frame)                 av_frame_free(&frame);
           if ( frame_aud)             av_frame_free(&frame_aud);

           if ( img_codec_ctx)         avcodec_close(img_codec_ctx);
           if ( aud_codec_ctx)         avcodec_close(aud_codec_ctx);


           if ( fmt_ctx)               avformat_close_input( &fmt_ctx );

}

 // 初始化视音频PTS
void    AVDecodeContext::init_pts(){

      video_pts = audio_pts = 0;
      start_time = av_gettime();
}

/*
 *
 *  计算视音频当前播放的PTS
 *
 */
int        AVDecodeContext::update_aud_pts(AVPacket*  pck){

     int  pts = 0;
     if  ( pck->dts == AV_NOPTS_VALUE && frame_aud->opaque &&   *(uint64_t*) frame_aud->opaque != AV_NOPTS_VALUE)
     {
         pts = *(uint64_t *) frame_aud->opaque;
     }
     else  if  (pck->dts != AV_NOPTS_VALUE)
     {
         pts = pck->dts;
     }

     //  pts* base = 以秒计数的显示时间戳, 再乘以AV_TIME_BASE转换为微秒
     audio_pts = pts *  av_q2d( fmt_ctx->streams[aud_stream_index]->time_base) * AV_TIME_BASE ;
     //qDebug()<< "【AUD】 dts: " << pck->dts << "  pts:" <<pts/1000 <<" time:"<< audio_pts/(double)AV_TIME_BASE;

     return pts;
 }

 int        AVDecodeContext::update_img_pts(AVPacket*  pck){

     int  pts = 0;
     if  (pck->dts == AV_NOPTS_VALUE && frame->opaque && *(uint64_t*) frame->opaque != AV_NOPTS_VALUE)
     {
         pts = *(uint64_t *) frame->opaque;
     }
     else  if  (pck->dts != AV_NOPTS_VALUE)
     {
         pts = pck->dts;
     }

     // video_pts* base = 以秒计数的显示时间戳, 再乘以AV_TIME_BASE转换为微秒
     video_pts = pts *  av_q2d(fmt_ctx->streams[img_stream_index]->time_base) * AV_TIME_BASE ;
     //qDebug()<< "<IMG> dts: " << pck->dts << "  pts:" <<pts/1000 <<" time:"<< video_pts/(double)AV_TIME_BASE;

     return pts;
 }

/*
 *
 *  视音频播放同步：如果解码速度太快，延时
 *
 */
 void      AVDecodeContext::aud_sync(){

     int64_t real_time = av_gettime() - start_time;  //主时钟时间
    if  ( audio_pts > real_time )
        av_usleep( audio_pts - real_time);
}

 void      AVDecodeContext::img_sync(){

     int64_t real_time = av_gettime() - start_time;  //主时钟时间
     if  ( video_pts > real_time )
         av_usleep( video_pts - real_time);
}
