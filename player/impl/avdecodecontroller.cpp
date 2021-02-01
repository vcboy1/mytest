#include "avdecodecontroller.h"
#include "avdecodecontext.h"
#include <thread>
#include <assert.h>
#include <QDebug>

AVDecodeController::AVDecodeController()
    : decode_ctx(nullptr),
      cmd(STOP),
      seek_pos(-1)
{

}

void         AVDecodeController::open(AVDecodeContext* ctx){

     dump("----->Open(ctx): ");

     assert(ctx != nullptr);
     assert(decode_ctx == nullptr);

     std::lock_guard< std::mutex >  g(mutex);

     decode_ctx         = ctx;
     ctx->controller    = this;
     cmd                = PLAY;
     seek_pos           = -1;

     dump("<-----Open(ctx): ");
}

// 解码线程结束，通知控制器清理现场
void         AVDecodeController::close(AVDecodeContext* ctx){

    dump("----->Close(ctx): ");

    std::lock_guard< std::mutex >  g(mutex);

    if  ( ctx  &&  decode_ctx== ctx){

        decode_ctx    = nullptr;
        //decode_status= Stop;
        cmd                = STOP;
    }

    dump("<-----Close(ctx): ");
}

// 播放控制命令
void                AVDecodeController::stop(){

    if ( isOpen() )
       cmd = STOP;

    dump(" stop(ctx): ");
}

void                AVDecodeController::pause(){

    if ( isOpen() && (cmd == PLAY|| cmd == SEEK)){

        cmd = PAUSE;
        decode_ctx->pause(true);

        dump(" pause(ctx): ");
    }
}

void                AVDecodeController::play(){

    //std::lock_guard< std::mutex >  g(mutex);

    if ( isOpen() && (cmd == PAUSE|| cmd == SEEK)){

        cmd = PLAY;
        decode_ctx->pause(false);

        dump(" play(ctx): ");
    }

}

void               AVDecodeController::seek(int64_t timestamp){

       if ( isOpen() ){

           dump( " seek(ctx): ");

           seek_pos = timestamp;
           cmd = SEEK;


       }
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
     return decode_ctx != nullptr && cmd == SEEK;
}

void                AVDecodeController::dump(const char * title){

    const char *  cmd_name[] ={"PLAY","PAUSE","STOP","SEEK"};

    qDebug() << title << " ctx: " << decode_ctx << " cmd: " << cmd_name[cmd];
    if ( decode_ctx != nullptr)
         qDebug() << "        start time:" << decode_ctx->start_time/(double)1000000
                  << " pause time:" << decode_ctx->pause_time/(double)1000000
                  << " seek  pos :" << seek_pos/(double)1000000
                  << " v_pts:" << decode_ctx->video_pts/(double)1000000
                  << " a_pts:" << decode_ctx->audio_pts/(double)1000000
                  << " iseof:" << decode_ctx->is_eof;
}
