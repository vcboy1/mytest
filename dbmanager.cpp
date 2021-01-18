#include "dbmanager.h"
#include <QtDebug>

DBManager::DBManager(QObject *parent) : QObject(parent)
{

}

// 初始化数据库
bool        DBManager::initDB(){

    db  =  QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("sdi.db");

    if ( !db.open() )
        return false;

    QStringList tables = db.tables();
    if ( tables.contains("sdihis", Qt::CaseInsensitive) )
        return false;

   QSqlQuery    query(db);

   if ( !query.exec("CREATE TABLE sdihis (sid INTEGER PRIMARY KEY AUTOINCREMENT, path VARCHAR NOT NULL, openat TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP)")||
        !query.exec("CREATE UNIQUE INDEX idx_path on sdihis (path)")){

       qDebug("create table failed!");
        return false;
   }

    return true;
}

// 添加一条历史
bool        DBManager::addHistory(const QString&  absPath){


    QSqlQuery   query(db);

    QString sql = QString("select * from sdihis where path='%1'").arg(absPath) ;
    bool  bok = query.exec( sql );
    if ( !bok )
        return false;


    if ( query.next() )
        sql = QString("update sdihis set path= '%1',openat=CURRENT_TIMESTAMP where sid=%2").arg(absPath).arg(query.value("sid").toInt());
    else
        sql = QString( "insert into sdihis(path) values('%1')" ).arg(absPath);

    return query.exec(sql);
}

//  按降序读取历史列表
 QList<OpenRecord>  DBManager::getHistory() const{

         QSqlQuery                        query(db);
         QList<OpenRecord>          result;

         query.exec("select openat, path from sdihis order by openat desc");
         while ( query.next() ){

             OpenRecord        rec;

             rec.timestamp = query.value(0).toString();
             rec.path          = query.value(1).toString();

             result << rec;
         }

         return result;
 }
