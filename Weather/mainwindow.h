#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QDebug>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QFile>
#include <iostream>
#include <QTextStream>
#include <QPixmap>
#include <QFrame>



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void send_net(QString);
    void display();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

    void recv_net(QNetworkReply*);

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *manager;  //请求句柄

private:
    QString city;           //城市(只能通过查询框更改）
    QString showcity;       //国家城市
    QString reporttime;     //数据发布的时间
    QString weather;        //天气现象
    QString realtime;       //实时温度
    QString temperature;    //温度范围
    QString wind;           //风向 风力
    QString humidity;       //空气湿度
    QString airlevel;       //空气质量
    QString airtips;        //贴心提示

};

#endif // MAINWINDOW_H
