#include "qplayerslider.h"


#include <QSlider>
#include <QDebug>
#include <QMouseEvent>

QPlayerSlider::QPlayerSlider(QWidget* parent)
    : QSlider(parent)
{
    m_posClicked = -1;
}

QPlayerSlider::~QPlayerSlider()
{

}

void QPlayerSlider::setValue(int value){

    if (!m_bLButtonDown)
        QSlider::setValue(value);
}

void QPlayerSlider::mousePressEvent(QMouseEvent *event)
{
    //获取当前点击位置,得到的这个鼠标坐标是相对于当前QSlider的坐标
    int currentX = event->pos().x();

    //获取当前点击的位置占整个Slider的百分比
    double per = currentX *1.0 /this->width();

    //利用算得的百分比得到具体数字
    m_posClicked = per*(this->maximum() - this->minimum()) + this->minimum();
    m_bLButtonDown = true;
    //设定滑动条位置
    //this->setValue(value);

    //滑动条移动事件等事件也用到了mousePressEvent,加这句话是为了不对其产生影响，是的Slider能正常相应其他鼠标事件
    //QSlider::mousePressEvent(event);
}

void QPlayerSlider::mouseReleaseEvent(QMouseEvent *event)
{
    QSlider::mouseReleaseEvent(event);

    if ( m_bLButtonDown ){

        emit PosChanged(m_posClicked);
        m_bLButtonDown = false;
        m_posClicked = -1;
    }
}
