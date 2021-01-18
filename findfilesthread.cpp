#include "findfilesthread.h"
#include <QDir>
#include <QStringList>

FindFilesThread::FindFilesThread(const QString& root,QObject* parent):QThread(parent)
{
    rootPath = root;
    isCancel = false;
    filters << QString("*.txt")<< QString("*.cpp")<< QString("*.h")<< QString("*.java");
}

void FindFilesThread::run() {

    isCancel = false;

    // 正常结束
    if ( findSubDir( rootPath) )
        emit  finished();

}

bool  FindFilesThread::findSubDir(const QString& path){

    // 如果cancel，立即返回
    if ( isCancel )
        return false;

    // 目录是否存在
    QDir    dir( path );
    if (  !dir.exists() ){

        emit error();
        return false;
    }

    // 过滤符合的文件
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    dir.setNameFilters( filters );
    foreach(QFileInfo mfi ,dir.entryInfoList()){

        emit found(mfi.absoluteFilePath() );
        if ( isCancel )
            return false;
    }


    // 过滤子目录
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    dir.setNameFilters( QStringList() );
    foreach(QFileInfo mfi ,dir.entryInfoList())
           findSubDir( mfi.absoluteFilePath() );


    return true;
}

 //槽
void    FindFilesThread::cancelFind(){

    isCancel = true;
}
