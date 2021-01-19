#ifndef MOVIEPLAYER_H
#define MOVIEPLAYER_H

#include <QString>

class MoviePlayer
{
public:
    MoviePlayer();

public:
    bool    play(const QString&  file);
};

#endif // MOVIEPLAYER_H
