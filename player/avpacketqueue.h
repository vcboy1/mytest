#ifndef AVPACKETQUEUE_H
#define AVPACKETQUEUE_H


#include <queue>
#include <mutex>
#include <atomic>

using namespace std;
class AVPacket;

/*****************************************************
 *
 *     线程安全的AVPacket队列
 *
**************************************************** */
class  AVPacketQueue{

 public:
    ~AVPacketQueue();

 public:

    void                    push_video(AVPacket*  p);
    void                    push_audio(AVPacket*  p);

    AVPacket*         pop_video();
    AVPacket*         pop_audio();

    long                    size() const{    return total_size;    }


protected:

    void                    clear();
    AVPacket*         clone(AVPacket* src);

protected:
    queue<AVPacket*>       v_queue, a_queue;  // 视音频队列
    mutex                             v_mutex, a_mutex; //  视音频锁
    atomic<long>                 total_size;               //  总Packet大小
};


/*****************************************************
 *
 *     解码器
 *
 *****************************************************/
class  AVDecoder{

public:
     AVDecoder();
    ~AVDecoder();

public:

    // 音频解码线程
    int       audio_decode();

    // 视频解码线程
    int       vedio_decode();

   //  文件解析线程
    bool    play(const char *url);

protected:

    AVPacketQueue     pck_queue;      // packet缓存
    bool                         is_eof;             // 是否结束
};

#endif // AVPACKETQUEUE_H
