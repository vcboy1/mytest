#ifndef MODEL_H
#define MODEL_H

#include <QString>





/*
 *   文本区配置
 */
 class TextAreaDisplayAttr{

public:
     TextAreaDisplayAttr():TextAreaDisplayAttr(20, QColor(171,178,191),QColor(40,42,52)){}

     TextAreaDisplayAttr(short  fsize, const QColor& tc, const QColor& bc){

         fontSize   = fsize;         textColor = tc;         bkColor   = bc;
     }

public:
       void         read(const QJsonObject& json);
       void         write(QJsonObject& json);

public:

    QString        fontName;
    int                fontSize;
    QColor        textColor;
    QColor        bkColor;
};

/*
 *   打开记录
 */
 class OpenRecord{

 public:
     QString        path;
     QString        timestamp;
 };


 /*
  *
  *    用户模型
  *
  */
 class SDIModel{

  public:
     void              load(){

         loadSetting();
     }

     void              save(){
         saveSetting();
     }

  private:
     void              loadSetting();
     void              saveSetting();

  public:

     TextAreaDisplayAttr     dispAttr;
     QString                        curFile;
 };


inline
 void             SDIModel::loadSetting(){

     QFile loadFile("setting.json");

     curFile = "C:/360Safe/mainwindow.cpp";
     if (!loadFile.open(QIODevice::ReadOnly))
         return ;

     QByteArray saveData = loadFile.readAll();

     QJsonDocument loadDoc( QJsonDocument::fromJson(saveData) );
     const QJsonObject& jsonObj = loadDoc.object();
     dispAttr.read( jsonObj);
     if ( jsonObj.contains("file") )
         curFile = jsonObj["file"].toString();
 }

inline
void            SDIModel::saveSetting(){

      QFile  saveFile("setting.json");
      if (!saveFile.open(QIODevice::WriteOnly))
          return ;

      QJsonObject settingObject;
      dispAttr.write(settingObject);

      if ( !curFile.isEmpty() )
         settingObject["file"] = curFile;

      QJsonDocument saveDoc(settingObject);
      saveFile.write(saveDoc.toJson());
 }
#endif // MODEL_H
