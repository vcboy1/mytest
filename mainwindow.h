#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts>
#include <QJsonObject>
#include "dbmanager.h"
#include "model.h"

class QLabel;
class FindFilesThread;   // 查找文件线程：Dev分支

namespace Ui {

class MainWindow;
}




class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void            closeEvent(QCloseEvent *event) override;

private:
    void            initUI();
    void            initMenuBarAndToolbar();
    void            initStatusBar();
    void            initTextArea();
    void            initDockWidgets();
    void            initChart();
    void            initFindThread();
    void            initModel();
    void            initHistory();

    void            flushTextArea();
    void            flushChartArea();
    void            onTabbarClicked(int index);

    QAction*    createAction(const QString& iconThemeName,
                                          const QString& iconPath,
                                          const QString& actionDispName,
                                          const QString& tips,
                                          const QKeySequence &shortcut=QKeySequence::UnknownKey);

private:

    bool              maybeSave();
    void              documentWasModified();
    bool              loadFile(const QString & fileName);
    bool              saveFile(const QString &fileName);


 /* 命令响应 */
private:
      void              onCmd_FileNew();
      void              onCmd_FileOpen();
      bool              onCmd_FileSave();
      bool              onCmd_FileSaveAs();

      void              onCmd_EditCut();
      void              onCmd_EditCopy();
      void              onCmd_EditPaste();

      void              onCmd_About();

private:
    Ui::MainWindow       *ui;
    QLabel*                      labelTime;
    QChartView*              chartView,*barChartView,*areaChartView;
    FindFilesThread*        findFilesThread;

    //TextAreaDisplayAttr    dispAttr;
    //QString                        curFile;
    SDIModel                    model;
    DBManager                  db;
};



// 状态栏时间显示定时器
class StatusBarTimer:public QObject{

    Q_OBJECT

public:
    StatusBarTimer(QLabel*label, QObject * parent = 0 ):QObject(parent){
        m_pLabel    = label;
        m_nTimerId = startTimer(1000);
    }

    ~StatusBarTimer(){
        if ( m_nTimerId != 0 )
              killTimer(m_nTimerId);
    }

protected:
    void timerEvent( QTimerEvent *event );

    int          m_nTimerId;
    QLabel* m_pLabel;
};
#endif // MAINWINDOW_H
