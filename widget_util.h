#ifndef WIDGET_UTIL_H
#define WIDGET_UTIL_H

#include <QString>
#include <QColor>

/*
 *
 *                         StyleSheet工具
 *
 */
class  StyleSheetUtil{

 /*
  *       色彩空间工具
  */
 public:

    // RGB(r,g,b)格式字符串
    static  QString     RGB_STR(const QColor&  color){

        return QString("rgb(%1,%2,%3)")
                    .arg(color.red())
                    .arg(color.green())
                    .arg(color.blue());
    }


    // RGBA(r,g,b,a)格式字符串
    static  QString     RGBA_STR(const QColor&  color){

        return QString("rgba(%1,%2,%3,%4)")
            .arg(color.red())
            .arg(color.green())
            .arg(color.blue())
            .arg(color.alpha());
    }

    // 背景颜色格式字符串
    static QString    BKG_COLOR_STYLE_STR(const QColor& c){

        return QString("background-color:%1").arg(RGBA_STR(c));
    }


    // 前景颜色格式字符串
    static QString    COLOR_STYLE_STR(const QColor& c){

        return QString("color:%1").arg(RGBA_STR(c));
    }


    // FlatButton样式
    static QString     FLAT_BUTTON_STYLE_STR(const QColor& bkg_c,
                                                                                  const QColor& border_c=Qt::black){

        return  QString("background-color:%1;border: 1px solid %2;")
                .arg( RGBA_STR(bkg_c))
                .arg( RGBA_STR(border_c));
    }
};



#endif // WIDGET_UTIL_H
