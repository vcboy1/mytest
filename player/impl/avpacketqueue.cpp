#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
}
#endif

#include "avpacketqueue.h"
#include <thread>




/*****************************************************
 *
 *    Packet出入队列，线程安全
 *
 *****************************************************/
AVPacketQueue::~AVPacketQueue(){
    clear();
}

void        AVPacketQueue::push_video(AVPacket*  p){

    if ( (p = clone(p)) == nullptr )
        return;

    lock_guard<mutex>   g(v_mutex);

    v_queue.push(p);
    total_size += p->size;
}



void       AVPacketQueue::push_audio(AVPacket*  p){

    if ( (p = clone(p)) == nullptr )
        return;

    lock_guard<mutex>   g(a_mutex);

    a_queue.push(p);
    total_size += p->size;
}




AVPacket*         AVPacketQueue::pop_video(){

    lock_guard<mutex>   g(v_mutex);
    AVPacket*                  p = nullptr;

    if (!v_queue.empty() ){

        p                = v_queue.front();

        v_queue.pop();
        total_size -= p->size;
    }
    return p;
}




AVPacket*         AVPacketQueue::pop_audio(){

    lock_guard<mutex>   g(a_mutex);
    AVPacket*                  p = nullptr;

    if (!a_queue.empty() ){

        p                = a_queue.front();
        a_queue.pop();
        total_size -= p->size;
    }
    return p;
}



// 清除视音频队列
void     AVPacketQueue::clear(){

      { // 清除视频队列

            lock_guard<mutex>   g(v_mutex);
            while ( !v_queue.empty() ){

                  AVPacket* p = v_queue.front();

                  v_queue.pop();
                  total_size -= p->size;

                  av_packet_unref(p);
                  av_packet_free(&p);
              }
        }

      { // 清除音频队列

            lock_guard<mutex>   g(a_mutex);
            while ( !a_queue.empty() ){

                  AVPacket* p = a_queue.front();

                  a_queue.pop();
                  total_size -= p->size;

                  av_packet_unref(p);
                  av_packet_free(&p);

              }
      }
}

//复制一个AVPacket
 AVPacket*         AVPacketQueue::clone(AVPacket* src){

     if (src == nullptr)
         return nullptr;

     AVPacket *dst = av_packet_alloc();
     if (av_packet_ref(dst, src )< 0 )
         return nullptr;

     return dst;
 }




