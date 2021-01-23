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

           is_quit = false;
           is_pause= false;
           start_time = video_pts = 0;
           img_stream_index = aud_stream_index = -1;

           img_buf                       = aud_buf = 0;
           img_convert_ctx         = 0;
           aud_convert_ctx         = 0 ;
           frame_rgb = frame= frame_aud = 0;
           img_codec                   = aud_codec = 0;
           img_codec_ctx            = aud_codec_ctx = 0 ;
           fmt_ctx                        = 0;
 }

 AVDecodeContext::~AVDecodeContext(){

           if ( img_convert_ctx )       sws_freeContext(img_convert_ctx);
           if ( aud_convert_ctx)        swr_free(&aud_convert_ctx);

           if ( img_buf)                     av_free(img_buf);
           if ( aud_buf)                     av_free(aud_buf);

           if ( frame_rgb)                  av_frame_free(&frame_rgb);
           if ( frame)                         av_frame_free(&frame);
           if ( frame_aud)                 av_frame_free(&frame_aud);

           if ( img_codec_ctx)          avcodec_close(img_codec_ctx);
           if ( aud_codec_ctx)          avcodec_close(aud_codec_ctx);


           if ( fmt_ctx)                     avformat_close_input( &fmt_ctx );
}
