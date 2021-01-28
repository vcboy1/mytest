#ifndef AVDECODECONTROLLER_H
#define AVDECODECONTROLLER_H

#include <mutex>
#include <atomic>

class AVDecodeContext;


/*
 *
 *    解码控制器
 *
 */
class AVDecodeController
{
public:
    AVDecodeController();

public:
    enum PlayCommand {  PLAY = 0, PAUSE , STOP};

// 播放控制命令
public:
    void                stop();
    void                pause();
    void                play();


    bool                isOpen() ;
    bool                isPaused();
    bool                isPlaying();

public:
    void                open(AVDecodeContext* ctx);
    void                close(AVDecodeContext* ctx);
    void                dump(const char * tilte);

public:
    AVDecodeContext*        decode_ctx;
    std::mutex                        mutex;

    std::atomic_int                 cmd;   //控制命令
};

#endif // AVDECODECONTROLLER_H
