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
      void       stop();

      // 暂停播放
      void       pause();

      // 恢复播放
      void      resume();

      // 是否打开
      bool      isOpen() const;

protected:

    // 音频解码线程
    int           audio_decode(void*  ctx);

    // 视频解码线程
    int           vedio_decode(void*  ctx);

    // 文件格式解码
    bool          format_decode(std::string url);

signals:
    void          onPlay(QImage*  img);

protected:
    void*        controller;            // 内部控制器
};


#endif // AVDECODER_H
