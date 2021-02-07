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
    void PosChanged(int value);

public Q_SLOTS:
    void setValue(int value);

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

    bool            m_bLButtonDown;
    int             m_posClicked;

};

#endif // QPLAYERSLIDER_H
