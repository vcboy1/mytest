#include "avdecodecontroller.h"
#include "avdecodecontext.h"
#include <thread>
#include <assert.h>
#include <QDebug>

#ifdef __cplusplus
extern "C"
{
#endif
#include <libavutil/time.h>
#ifdef __cplusplus
}
#endif

AVDecodeController::AVDecodeController()
    : decode_ctx(nullptr),
      cmd(STOP),
      cmd_ctrl(NONE),
      seek_pos(-1)
{

}

void         AVDecodeController::open(AVDecodeContext* ctx){

     dump("----->OPEN(ctx): ");

     assert(ctx != nullptr);
     assert(decode_ctx == nullptr);

     std::lock_guard< std::mutex >  g(mutex);

     decode_ctx         = ctx;
     ctx->controller    = this;
     cmd                = PLAY;
     cmd_ctrl           = NONE;
     seek_pos           = -1;

     dump("<-----OPEN(ctx): ");
}

// 解码线程结束，通知控制器清理现场
void         AVDecodeController::close(AVDecodeContext* ctx){

    dump("----->CLOSE(ctx): ");

    std::lock_guard< std::mutex >  g(mutex);

    if  ( ctx  &&  decode_ctx== ctx){

        decode_ctx    = nullptr;
        cmd_ctrl      = NONE;
        cmd           = STOP;
    }

    dump("<-----CLOSE(ctx): ");
}

// 播放控制命令
void                AVDecodeController::stop(){

    if ( isOpen() )
       cmd = STOP;

    dump("<-----STOP(ctx): ");
}

void                AVDecodeController::pause(){

    if ( isOpen() && cmd == PLAY){

        cmd = PAUSE;
        decode_ctx->pause(true);

        dump("<-----PAUSE(ctx): ");
    }
}

void                AVDecodeController::play(){

    //std::lock_guard< std::mutex >  g(mutex);

    if ( isOpen() && cmd == PAUSE){

        cmd = PLAY;
        decode_ctx->pause(false);

        dump("<-----PLAY(ctx): ");
    }

}

void               AVDecodeController::seek(int64_t timestamp){

       if ( isOpen() && cmd_ctrl == NONE){

           seek_pos = timestamp;
           cmd_ctrl = SEEK;

           dump( "<-----SEEK(ctx): ");
       }
}

void               AVDecodeController::clear_seek_status(){

    seek_pos = -1;
    cmd_ctrl = AVDecodeController::NONE;

     dump( "<-----CLEAR_seek_status(ctx): ");
}

bool               AVDecodeController::isOpen() {

     std::lock_guard< std::mutex >  g(mutex);
     return decode_ctx != nullptr;
}

bool               AVDecodeController::isPaused() {

     std::lock_guard< std::mutex >  g(mutex);
     return decode_ctx != nullptr && cmd == PAUSE;
}

bool               AVDecodeController::isPlaying() {

     std::lock_guard< std::mutex >  g(mutex);
     return decode_ctx != nullptr && cmd == PLAY;
}

bool               AVDecodeController::isSeek() {

     std::lock_guard< std::mutex >  g(mutex);
     return decode_ctx != nullptr && cmd_ctrl == SEEK;
}

void                AVDecodeController::dump(const char * title){

    const char *  cmd_name[] ={"PLAY","PAUSE","STOP"};
    const char *  cmd_ctrl_name[] ={"NONE","SEEK"};

    qDebug() << title << " ctx: " << decode_ctx
             << " cmd: " << cmd_name[cmd]
             << " cmd_ctrl: " << cmd_ctrl_name[cmd_ctrl]   ;
    if ( decode_ctx != nullptr)
         qDebug() << "        start time:" << (av_gettime() - decode_ctx->start_time)/(double)1000000
                  << " pause time:" << decode_ctx->pause_time/(double)1000000
                  << " seek  pos :" << seek_pos/(double)1000000
                  << " v_pts:" << decode_ctx->video_pts/(double)1000000
                  << " a_pts:" << decode_ctx->audio_pts/(double)1000000
                  << " iseof:" << decode_ctx->is_eof;
}
