#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "widget_util.h"
#include "findfilesthread.h"
#include <QtWidgets>
#include <QDebug>
#include <QApplication>
#include "movieplayer.h"
#include <avdecoder.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),findFilesThread(0),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initModel();

    initUI();
    initFindThread();
}

MainWindow::~MainWindow()
{
    delete ui;
}

 void      MainWindow::closeEvent( QCloseEvent *event ){

     event->accept();
     model.save();
 }

 /***************************************
  *
  *     初始化查找线程
  *
  ***************************************/
 void     MainWindow::initFindThread(){

     if ( findFilesThread )
         return;

     findFilesThread = new FindFilesThread("/",this);

     connect(findFilesThread, &FindFilesThread::found, [this](const QString&  file){

            ui->listFiles->addItem( file);
            if  (ui->listFiles->count() > 500 )
                    findFilesThread->cancelFind();
     });
     findFilesThread->start();
 }

 /***************************************
  *
  *     初始化数据库
  *
  ***************************************/
 void    MainWindow::initModel(){

       model.load();
       db.initDB();

 }

/***************************************
 *
 *      初始化界面
 *
 ***************************************/

void   MainWindow::initUI(){

        initMenuBarAndToolbar();
        initStatusBar();
        initDockWidgets();
        initTextArea();
        initChart();
        initHistory();
        initPlayer();


        flushTextArea();
        loadFile( model.curFile );
}

void   MainWindow::initMenuBarAndToolbar(){

      // 添加File菜单
     QMenu*       fileMenu = menuBar()->addMenu(tr("&File"));
     QToolBar*    fileToolbar = addToolBar(tr("&File"));

         QAction*    act = createAction("document-new", ":/images/new.png", tr("&New"),"Create a new file", QKeySequence::New);
         connect(act, &QAction::triggered, this, &MainWindow::onCmd_FileNew);
         fileMenu->addAction(act);
         fileToolbar->addAction(act);

         act = createAction("document-open", ":/images/open.png", tr("&Open"),"Open a file",QKeySequence::Open);
         connect(act, &QAction::triggered, this, &MainWindow::onCmd_FileOpen);
         fileMenu->addAction(act);
         fileToolbar->addAction(act);

         act = createAction("document-save", ":/images/save.png", tr("&Save"),"Save the document to disk",QKeySequence::Save);
         connect(act, &QAction::triggered, this, &MainWindow::onCmd_FileSave);
         fileMenu->addAction(act);
         fileToolbar->addAction(act);

         act = createAction("document-save-as", "", tr("Save &As..."),"Save the document undere a new name",QKeySequence::SaveAs);
         connect(act, &QAction::triggered, this, &MainWindow::onCmd_FileSaveAs);
         fileMenu->addAction(act);

         fileMenu->addSeparator();
         act = createAction("application-exit", "", tr("E&xit..."),"Exit the application",QKeySequence::Quit);
          connect(act, &QAction::triggered, this, &MainWindow::close);
         fileMenu->addAction(act);



     // 添加Edit菜单
     QMenu*       editMenu = menuBar()->addMenu(tr("&Edit"));
     QToolBar*    editToolbar = addToolBar(tr("Edit"));

         act = createAction("edit-cut", ":/images/cut.png", tr("Cu&t"),
                                    "Cut the current selection's contents to the clipboard",QKeySequence::Cut);
         connect(act, &QAction::triggered, this, &MainWindow::onCmd_EditCut);
         editMenu->addAction(act);
         editToolbar->addAction(act);
         connect(ui->editDoc, &QPlainTextEdit::copyAvailable, act, &QAction::setEnabled);
         act->setEnabled(false);

         act = createAction("edit-copy", ":/images/copy.png", tr("&Copy"),
                                    "Copy the current selection's contents to the clipboard",QKeySequence::Copy);
         connect(act, &QAction::triggered, this, &MainWindow::onCmd_EditCopy);
         editMenu->addAction(act);
         editToolbar->addAction(act);
         connect(ui->editDoc, &QPlainTextEdit::copyAvailable, act, &QAction::setEnabled);
         act->setEnabled(false);

         act = createAction("edit-paste", ":/images/paste.png", tr("&Paste"),
                                    "Paste the clipboard's contents into the current selection",QKeySequence::Paste);
         connect(act, &QAction::triggered, this, &MainWindow::onCmd_EditPaste);
         editMenu->addAction(act);
         editToolbar->addAction(act);

      // 添加Views菜单
      QMenu*      viewMenu= menuBar()->addMenu(tr("&View"));

            viewMenu->addAction( ui->dockProperties->toggleViewAction() );


        // 添加Movie菜单
       QMenu*       movieMenu = menuBar()->addMenu(tr("&Movie"));


           act = createAction("Movie-open", ":/images/open.png", tr("&Open"),"Open a Movie",QKeySequence::Open);
           connect(act, &QAction::triggered, this, &MainWindow::onCmd_MovieOpen);
           movieMenu->addAction(act);

      // 添加Help菜单
      QMenu *      helpMenu = menuBar()->addMenu(tr("&Help"));
      QToolBar*   helpToolbar = addToolBar(tr("Edit"));

          act = helpMenu->addAction(tr("&About"), this, &MainWindow::onCmd_About);
          act = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);

          QFontComboBox* font = new QFontComboBox(this);
          connect(    font,
                          &QFontComboBox::currentTextChanged,
                          [this](const QString &name){

                                 this->model.dispAttr.fontName = name;
                                 flushTextArea();
                           });

          helpToolbar->addWidget(font);
          font->setFixedHeight(28);
          helpToolbar->addAction(helpMenu->menuAction());


}

void   MainWindow::initStatusBar(){

    statusBar()->showMessage("Ready....");

    //加入时间显示控件
    labelTime = new QLabel(QTime::currentTime().toString("h:m:s"));
    labelTime->setAlignment(Qt::AlignCenter);
    labelTime->setMinimumSize(labelTime->sizeHint());
    labelTime->setFrameShape( QFrame::Panel);
    labelTime->setFrameShadow(QFrame::Sunken);
    statusBar()->addPermanentWidget(labelTime);

    new StatusBarTimer(labelTime, this);
}

void   MainWindow::initDockWidgets(){

    // 字体大小
    ui->spinTextAreaFontSize->setRange(8,40);
    ui->spinTextAreaFontSize->setValue( model.dispAttr.fontSize );
    ui->spinTextAreaFontSize->setSuffix(" px");
    connect(    ui->spinTextAreaFontSize,
                    static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                     [=](int v){

                            model.dispAttr.fontSize = v;
                            flushTextArea();
                 });

     // 字体颜色
    connect(   ui->btnFontColor, &QPushButton::clicked, [this](bool checked){

           QColor c= QColorDialog::getColor( model.dispAttr.textColor, this,"选择文字颜色");
           if ( c.isValid() ){

               model.dispAttr.textColor =c;           flushTextArea();
           }
    });

    // 背景颜色
   connect(   ui->btnBkgColor, &QPushButton::clicked, [this](bool checked){

          QColor c= QColorDialog::getColor( model.dispAttr.bkColor, this,"选择背景颜色");
          if ( c.isValid() ){

              model.dispAttr.bkColor =c;           flushTextArea();
          }
   });

   // 文件列表点击
   connect( ui->listFiles, &QListWidget::currentTextChanged, [this](const QString &selFile){

             loadFile(selFile);
   });
}

void   MainWindow::initTextArea(){

    connect(ui->editDoc->document(), &QTextDocument::contentsChanged,
                this, &MainWindow::documentWasModified);

    flushTextArea();
}


void    MainWindow::initChart(){
    
    chartView =  new    QChartView();
    ui->girdLayoutChart->addWidget(chartView, 0,1);

    barChartView = new QChartView();
    ui->girdLayoutChart->addWidget(barChartView, 1,0,1,2);

    areaChartView = new QChartView();
    ui->girdLayoutChart->addWidget(areaChartView, 0,0);

    connect( ui->tabMain, &QTabWidget::tabBarClicked, this,onTabbarClicked);
}

void   MainWindow::initHistory(){

     QSqlQueryModel     *sqlModel = new QSqlQueryModel();
     sqlModel->setQuery("select openat, path from sdihis order by openat desc");
     sqlModel->setHeaderData(0, Qt::Horizontal, tr("时间"));
     sqlModel->setHeaderData(1, Qt::Horizontal, tr("文件"));
     ui->tableHistory->setModel(sqlModel);
     ui->tableHistory->setSelectionBehavior(QAbstractItemView::SelectRows);
     ui->tableHistory->setAlternatingRowColors( true );
     ui->tableHistory->horizontalHeader()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
     ui->tableHistory->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
     ui->tableHistory->setColumnWidth(1, -1);
     ui->tableHistory->show();

     connect( ui->tableHistory, &QTableView::doubleClicked, [&,this](const QModelIndex &index){

               int                  row         = index.row();

               QModelIndex idxPath    = index.model()->index(row,1);
               QString          file          = index.model()->data(idxPath).toString();
               //QString  file = sqlModel->record(row).value(1).toString();
               if ( loadFile( file) )
                   ui->tabMain->setCurrentIndex(0);
     });
}

void  MainWindow::initPlayer(){


}

void  MainWindow::flushChartArea(){

    // 分析数据
    auto                     doc =  ui->editDoc->document();
    QLineSeries *series= new QLineSeries();

    int                       maxCount =0;
    for ( int i =0; i < doc->blockCount(); ++i){

        auto len = doc->findBlockByNumber(i).text().length();
         *series << QPointF(i+1,  len );
         if (maxCount < len )
              maxCount = len;
    }

    // 折线图
    QChart::ChartTheme       theme=QChart::ChartThemeDark;

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("QLineSeries");
    chart->createDefaultAxes();
    chart->axisX()->setRange(0, doc->blockCount()+1);
    chart->axisY()->setRange(0, maxCount+1);
    chart->setTheme(theme);
    chartView->setChart(chart);



    // 面状图
    QAreaSeries* areaSeries =new QAreaSeries(series,0);
    chart = new QChart();
    chart->addSeries(areaSeries);
    chart->setTitle("QAreaSeries");
    chart->createDefaultAxes();
    chart->setTheme(theme);
    areaChartView->setChart(chart);


    // 柱状图
    QBarSet* set1 = new QBarSet("");
    QBarSeries* barSeries = new QBarSeries();//给每一列分配区域
    barSeries->append(set1);

    for ( int i =0; i < doc->blockCount(); ++i){

           auto len = doc->findBlockByNumber(i).text().length();
          *set1 << len;
    }
   chart = new QChart();
   chart->addSeries(barSeries);
   chart->setTitle("QBarSeries");

   QValueAxis* axisY = new QValueAxis();
   axisY->setRange(0, maxCount+1);
   chart->addAxis(axisY, Qt::AlignLeft);//放到左边
   barSeries->attachAxis(axisY);
   chart->setTheme(theme);
   barChartView->setChart(chart);
}

void   MainWindow:: onTabbarClicked(int index){

    switch ( index ){

    case 1:        flushChartArea();                break;
    case 2:
       {
             ((QSqlQueryModel*)ui->tableHistory->model())->setQuery("select openat, path from sdihis order by openat desc");
             ui->tableHistory->show();    break;
        }
    }
}

void   MainWindow::flushTextArea(){


    QFont font = ui->editDoc->font();
    font.setPixelSize( model.dispAttr.fontSize);
    if  (!model.dispAttr.fontName.isEmpty() )
        font.setFamily(model.dispAttr.fontName);
    ui->editDoc->setFont(font);

    QPalette p = ui->editDoc->palette();
    p.setColor(QPalette::Active, QPalette::Base,  model.dispAttr.bkColor);
    p.setColor(QPalette::Inactive, QPalette::Base, model.dispAttr.bkColor);
    p.setColor(QPalette::Active, QPalette::Text,   model.dispAttr.textColor);
    p.setColor(QPalette::Inactive, QPalette::Text, model.dispAttr.textColor);
    ui->editDoc->setPalette(p);

    // 字色
    p = ui->btnFontColor->palette();
    ui->btnFontColor->setStyleSheet( StyleSheetUtil::FLAT_BUTTON_STYLE_STR(model.dispAttr.textColor) );
    ui->btnFontColor->setPalette(p);

    // 背景色
    p = ui->btnBkgColor->palette();
    ui->btnBkgColor->setStyleSheet( StyleSheetUtil::FLAT_BUTTON_STYLE_STR( model.dispAttr.bkColor)  );
    ui->btnBkgColor->setPalette(p);
}



// 创建QAction
QAction*    MainWindow::createAction(
                                      const QString& iconThemeName,
                                      const QString& iconPath,
                                      const QString& actionDispName,
                                      const QString& tips,
                                      const QKeySequence &shortcut){

    QIcon          icon    = QIcon::fromTheme( iconThemeName, QIcon(iconPath));
    QAction *   act      = new QAction( icon, actionDispName, this);

    act->setStatusTip(tips);
    if ( shortcut != QKeySequence::UnknownKey )
        act->setShortcut(shortcut);

    return act;
}


 /*************************************************
  *
  *   命令响应
  *
  **************************************************/
void              MainWindow::onCmd_FileNew(){

      if ( !maybeSave() )
          return;

     model.curFile.clear();
     ui->editDoc->clear();
     ui->editDoc->document()->setModified(false);
     documentWasModified();
     flushChartArea();
}

void              MainWindow::onCmd_FileOpen(){

    if ( !maybeSave() )
        return;

     QString curPath=QDir::currentPath();//获取系统当前目录
     QString dlgTitle="选择一个文件"; //对话框标题
     QString filter="文本文件(*.txt);;程序文件(*.h *.cpp *.java);;所有文件(*.*)"; //文件过滤器
     QString aFileName=QFileDialog::getOpenFileName(this,dlgTitle,curPath,filter);
     if ( aFileName.isEmpty() )
         return;

     loadFile( aFileName);
}

bool              MainWindow::onCmd_FileSave(){

    if ( model.curFile.isEmpty() )
         return onCmd_FileSaveAs();


    return saveFile(model.curFile);
}

bool              MainWindow::onCmd_FileSaveAs(){

    QString curPath=QCoreApplication::applicationDirPath(); //获取应用程序的路径
    QString dlgTitle="保存文件"; //对话框标题
    QString filter="文本文件(*.txt);;程序文件(*.h *.cpp *.java);;所有文件(*.*)"; //文件过滤器
    QString file=QFileDialog::getSaveFileName(this,dlgTitle,curPath,filter);

    return saveFile(file);
}

void              MainWindow::onCmd_EditCut(){

        ui->editDoc->cut();
}

void              MainWindow::onCmd_EditCopy(){

        ui->editDoc->copy();

}

void              MainWindow::onCmd_EditPaste(){

        ui->editDoc->paste();
}

void              MainWindow::onCmd_About(){

    QMessageBox::about(this, tr("About Application"),
             tr("The <b>Application</b> example demonstrates how to "
                "write modern GUI applications using Qt, with a menu bar, "
                "toolbars, and a status bar."));
}

bool              MainWindow::maybeSave()
//! [40] //! [41]
{
    if (!ui->editDoc->document()->isModified())
        return true;
    const QMessageBox::StandardButton ret
           = QMessageBox::warning(this, tr("Application"),
                               tr("The document has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return onCmd_FileSave();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void             MainWindow::documentWasModified()
{
    bool  modified = ui->editDoc->document()->isModified();
    setWindowModified(modified );

    if ( modified )
        setWindowTitle( QString("MainWindow[%1](*)").arg(model.curFile));
    else
        setWindowTitle( QString("MainWindow[%1]").arg(model.curFile));
}

bool            MainWindow::loadFile(const QString & fileName){

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return false;

    QTextStream in(&file);

    ui->editDoc->setPlainText(in.readAll());
    statusBar()->showMessage( fileName, 2000);

    model.curFile = fileName;
    ui->editDoc->document()->setModified(false);
    documentWasModified();
    flushChartArea();

    db.addHistory(fileName);
    return true;
}

bool            MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName),
                                  file.errorString()));
        return false;
    }

    QTextStream out(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    out <<  ui->editDoc->toPlainText();
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif


    statusBar()->showMessage(tr("File saved"), 2000);

    model.curFile = fileName;
    ui->editDoc->document()->setModified(false);
    documentWasModified();
    //setWindowModified(false);
    return true;
}


void     MainWindow::onMoviePlay(QImage* img){

    QSize size = ui->labelPlayer->size();

    ui->labelPlayer->setPixmap(
              QPixmap::fromImage(*img).scaled(size,Qt::KeepAspectRatio));
    delete img;
    qApp->processEvents();
}

void     MainWindow::onCmd_MovieOpen(){

     QString   aFileName1 = "C:\\Users\\vcboy1\\Desktop\\V1\\[电影天堂www.dytt89.com]电话BD韩语中字.mp4";
     AVDecoder decoder;

     QObject::connect( &decoder, &AVDecoder::onPlay,
                       this,&MainWindow::onMoviePlay,Qt::QueuedConnection);
     decoder.play(aFileName1.toUtf8().data());
     return;

/*
     QString curPath="E:\\数媒资源\\绘本动画提交";
     QString dlgTitle="选择一个视频"; //对话框标题
     QString filter="视频文件(*.mp4 *.avi *.mkv);;所有文件(*.*)"; //文件过滤器
     QString aFileName=QFileDialog::getOpenFileName(this,dlgTitle,curPath,filter);
     if ( aFileName.isEmpty() )
         return;

     QString   aFileName = "e:/1.mp4";
     MoviePlayer   player;

     connect( &player, &MoviePlayer::onPlay, [this](QImage*  img){

            ui->labelPlayer->setPixmap( QPixmap::fromImage(*img));
            delete img;
            qApp->processEvents();
     });
     connect( &player, &MoviePlayer::onStop, [this](const QString&file){

            ui->labelPlayer->clear();
     });

     QSize size = ui->labelPlayer->size();
     player.play(aFileName, size.width(), size.height());
*/
}


/***********************************************
 *
 *      杂 项
 *
 * *********************************************/

void   StatusBarTimer::timerEvent( QTimerEvent *event ){

    m_pLabel->setText(QTime::currentTime().toString("hh:mm:ss"));
}



void   TextAreaDisplayAttr::read(const QJsonObject& json){

      fontName = json["fontName"].toString();
      fontSize   =  json["fontSize"].toInt();
      QRgb rgb =  json["textColor"].toInt();
      textColor.setRgba(rgb);
      rgb =  json["bkColor"].toInt();
      bkColor.setRgba(rgb);
}

void    TextAreaDisplayAttr::write(QJsonObject& json){

    json["fontName"] = fontName;
    json["fontSize"]   = fontSize;
    json["textColor"] = (int)textColor.rgba();
    json["bkColor"]   = (int)bkColor.rgba();

}
