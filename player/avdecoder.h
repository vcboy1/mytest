#ifndef AVDECODER_H
#define AVDECODER_H

#include <QObject>
#include <string>

class QImage;


/*****************************************************
 *
 *     解码器
 *
 *****************************************************/
class  AVDecoder:public QObject{

    Q_OBJECT

public:
     AVDecoder(QObject *parent=nullptr);
    ~AVDecoder();

public:
      //播放视频
      bool        play(std::string url);

      // 停止播放视频
      void        stop();

      // 暂停播放
      void        pause();

      // 恢复播放
      void        resume();

      // 是否打开状态
      bool        isOpen() const;

      // 是否暂停状态
      bool        isPaused() const;

      // 是否播放状态
      bool        isPlaying() const;


      // 获取打开视频的时长：AV_TIME_BASE单位
      int64_t     duration() const;


      // 获取当前播放点的时间戳:AV_TIME_BASE单位
      int64_t     pos() const;


protected:

    // 音频解码线程
    int           audio_decode(void*  ctx);

    // 视频解码线程
    int           vedio_decode(void*  ctx);

    // 文件格式解码
    bool          format_decode(std::string url);

signals:
    void          onStart(std::string url, int64_t duration);
    void          onPlay(QImage*  img, int64_t time_stamp);
    void          onStop();

protected:
    void*         controller;            // 内部控制器
};


#endif // AVDECODER_H
