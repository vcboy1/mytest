#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QtSql>
#include <QList>
#include "model.h"

class DBManager : public QObject
{
    Q_OBJECT
public:
    explicit DBManager(QObject *parent = nullptr);

signals:

public slots:

 public:
    // 初始化数据库
    bool        initDB();

    // 添加一条历史
    bool        addHistory(const QString&  absPath);

   //  按降序读取历史列表
    QList<OpenRecord>  getHistory() const;

 private:
    QSqlDatabase        db;
};

#endif // DBMANAGER_H
