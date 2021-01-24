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
      //播放
      bool        play(std::string url);

public:

    // 音频解码线程
    int           audio_decode();

    // 视频解码线程
    int           vedio_decode(void*  ctx);

    // 文件格式解码
    bool          format_decode(std::string url);

signals:
    void          onPlay(QImage*  img);

protected:
};


#endif // AVDECODER_H
