#include "movieplayer.h"
#include <windows.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <QImage>
#include <QDebug>
#define __STDC_CONSTANT_MACROS      //ffmpeg要求

/* 必须记住，使用C++编译器时一定要使用extern "C"，否则找不到链接文件 */
#ifdef __cplusplus
extern "C"
{
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <SDL2/include/SDL.h>
//#include <SDL2/include/SDL_thread.h>
#ifdef __cplusplus
}
#endif


const int kMaxPacketSize = 15*1024*1024;    // 音视频加起来最大缓存为15M


#undef main         // 避免和SDL2的main重定义
MoviePlayer::MoviePlayer(QObject *parent):QObject(parent)
{
}


bool   MoviePlayer::play(const QString&  file,int outWidth, int outHeight){

   // 设置日志的标准
   //av_log_set_level(AV_LOG_DEBUG);


   // 注册复用器,编码器等
    av_register_all();
    avformat_network_init();

    // 打开多媒体文件
    AVFormatContext *pFormatCtx = NULL;
    if ( avformat_open_input( &pFormatCtx,file.toLatin1().data() , NULL, NULL ) != 0 )
        return false;

    if ( avformat_find_stream_info(pFormatCtx,NULL) < 0 )
        return false;

    // 打印有关输入或输出格式的详细信息, 该函数主要用于debug
    av_dump_format( pFormatCtx, 0, file.toLatin1().data() , 0 );
    qDebug() <<  "---------------------------------------------------------------------";

    // 查找视音频流
   int   videoStream=-1, audioStream=-1,i =0;
   for ( i = 0; i < pFormatCtx->nb_streams; ++i )

       switch ( pFormatCtx->streams[i]->codec->codec_type){

       case AVMEDIA_TYPE_VIDEO:
                    videoStream = i;   break;

       case AVMEDIA_TYPE_AUDIO:
                     audioStream = i;   break;
       }
    if ( videoStream == -1 )
        return false;


    //获取视频解码器
    AVCodecContext*  pVideoCodecCtx  = pFormatCtx->streams[videoStream]->codec;
    AVCodec*             pVideoCodec       = avcodec_find_decoder( pVideoCodecCtx->codec_id);
    if ( pVideoCodec == NULL )
        return false;

    //打开解码器
    if (avcodec_open2( pVideoCodecCtx, pVideoCodec,NULL) <0 )
        return false;

    //分配VideoFrame
    AVFrame*   pFrame        = av_frame_alloc();
    AVFrame*   pFrameRGB = av_frame_alloc();
    if ( pFrame  == NULL || pFrameRGB == NULL )
        return false;

    //分配缓冲区
    if ( outWidth < 0 )     outWidth =  pVideoCodecCtx->width;
    if ( outHeight <0 )     outHeight=  pVideoCodecCtx->height;
    outWidth >>=2;
    outWidth <<=2;
    int         numBytes = avpicture_get_size( AV_PIX_FMT_RGB24, outWidth,outHeight);
    uint8_t* buffer      = (uint8_t*)av_malloc(numBytes);

    if ( buffer ==NULL )
        return false;
    avpicture_fill( (AVPicture*) pFrameRGB, buffer, AV_PIX_FMT_RGB24,outWidth, outHeight);

    emit  onStart(file);

    // 视频解码
    AVPacket            packet;
    int                       frameFinished = 0;
    for (i=0; av_read_frame(pFormatCtx,&packet) >= 0;){

        if ( packet.stream_index == videoStream ){

            //读完一个packet
            avcodec_decode_video2(pVideoCodecCtx, pFrame,&frameFinished, &packet);
            if ( frameFinished ){

                struct SwsContext*   img_convert_ctx = NULL;
                img_convert_ctx = sws_getCachedContext(img_convert_ctx,                 pVideoCodecCtx->width, pVideoCodecCtx->height,
                                                                               pVideoCodecCtx->pix_fmt,  outWidth, outHeight,
                                                                               AV_PIX_FMT_RGB24,SWS_BICUBIC,NULL,NULL,NULL );

                if ( !img_convert_ctx)
                    return false;

                sws_scale( img_convert_ctx, pFrame->data,
                                 pFrame->linesize , 0,pVideoCodecCtx->height,
                                 pFrameRGB->data, pFrameRGB->linesize);

                // Frame转QImage
                 emit onPlay( new QImage ( (uchar *)buffer, outWidth, outHeight,QImage::Format_RGB888));

                 if ( ++i > 300 )
                    break;

                qDebug() <<  "Decode Frame: " << i ;
            }
        }
        av_free_packet(&packet);
    }


    //释放资源
    av_free(buffer);
    av_frame_free(&pFrameRGB);
    av_frame_free(&pFrame);
    avcodec_close(pVideoCodecCtx);
    avformat_close_input( &pFormatCtx );

    emit  onStop(file);
    return true;
}


void    MoviePlayer::stop(){

}
