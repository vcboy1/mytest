#ifndef AVDECODER_H
#define AVDECODER_H

#include <QObject>

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

    // 音频解码线程
    int         audio_decode();

    // 视频解码线程
    int         vedio_decode(void*  ctx);

   //  文件解析线程
    bool        play(const char *url);

signals:
    void        onPlay(QImage*  img);

protected:

    bool              is_eof;         // 是否结束
};


#endif // AVDECODER_H
