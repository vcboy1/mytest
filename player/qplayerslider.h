#ifndef QPLAYERSLIDER_H
#define QPLAYERSLIDER_H

#include <QObject>
#include <QSlider>


class QPlayerSlider : public QSlider
{
    Q_OBJECT


public:
    QPlayerSlider(QWidget *parent = nullptr);
    ~QPlayerSlider();

signals:
    void PosChanged();

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

    bool            m_bLButtonDown;

};

#endif // QPLAYERSLIDER_H
