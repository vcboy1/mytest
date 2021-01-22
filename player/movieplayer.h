#ifndef MOVIEPLAYER_H
#define MOVIEPLAYER_H

#include <QString>
#include <QImage>
#include <QObject>

class MoviePlayer : public  QObject
{
    Q_OBJECT

public:
    MoviePlayer(QObject *parent=Q_NULLPTR);

public:
    bool    play(const QString&  file,int  outWidth=-1, int outHeight=-1);

private:
    bool    play1(const QString&  file,int  outWidth=-1, int outHeight=-1);

signals:
    void    onStart(const QString&file);
    void    onPlay(QImage*  img);
    void    onStop(const QString&file);

public  slots:
    void    stop();
};

#endif // MOVIEPLAYER_H
