#ifndef FINDFILESTHREAD_H
#define FINDFILESTHREAD_H

#include <QThread>
#include <QString>
#include <QStringList>
#include <QDir>

class FindFilesThread : public QThread
{
    Q_OBJECT

public:
    FindFilesThread(const QString& root,QObject*  parent);

protected:
    virtual void run() override;

// 信号集
signals:
    void    finished();
    void    found(const QString&  file);
    void    error();

  //槽
 public    slots:
    void    cancelFind();

private:
    bool   findSubDir(const QString& dir);

private:
    QString             rootPath;
    QStringList       filters;
    bool                  isCancel;
};

#endif // FINDFILESTHREAD_H
