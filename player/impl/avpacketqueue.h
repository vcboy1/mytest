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
    AVPacketQueue();
    ~AVPacketQueue();

 public:

    void                    push_video(AVPacket*  p);
    void                    push_audio(AVPacket*  p);

    AVPacket*               pop_video();
    AVPacket*               pop_audio();

    long                    size() const{    return total_size;    }


protected:

    void                    clear();
    AVPacket*               clone(AVPacket* src);

protected:
    queue<AVPacket*>        v_queue, a_queue;  // 视音频队列
    mutex                   v_mutex, a_mutex; //  视音频锁
    atomic<long>            total_size;        //  总Packet大小
};



#endif // AVPACKETQUEUE_H
