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
    enum PlayCommand {  PLAY = 0, PAUSE , STOP, SEEK};

// 播放控制命令
public:
    void                stop();
    void                pause();
    void                play();
    void                seek(int64_t timestamp);


    bool                isOpen() ;
    bool                isPaused();
    bool                isPlaying();
    bool                isSeek();

public:
    void                open(AVDecodeContext* ctx);
    void                close(AVDecodeContext* ctx);
    void                dump(const char * tilte);

public:
    AVDecodeContext*    decode_ctx;
    std::mutex          mutex;

    std::atomic_int     cmd;   //控制命令
    int64_t             seek_pos;
};

#endif // AVDECODECONTROLLER_H
