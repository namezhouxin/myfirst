#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setMinimumSize(600,400);
    this->setWindowTitle("天气查询");
    this->setWindowIcon(QIcon(":/icon/icon.png"));
    this->setStatusTip("嘘，不要到处声张，这是一个查询天气的神秘程序");
    ui->tips->setWordWrap(true);
    ui->local->setStyleSheet("outline: none");
    QPixmap pixl(":/icon/local.png");
    pixl = pixl.scaled(QSize(40,40), Qt::KeepAspectRatio);
    ui->local->setPixmap(QPixmap(pixl));

    manager = new QNetworkAccessManager(this);
    QNetworkRequest quest;
    QString url = "https://v0.yiketianqi.com/api?unescape=1&version=v62&appid=98751895&appsecret=z1WXHdgM";
    quest.setUrl(QUrl(url));
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(recv_net(QNetworkReply*)));
    manager->get(quest);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::recv_net(QNetworkReply* reply)
{

    QString allinfo = reply->readAll();
    qDebug()<<"recv_net:"<<allinfo;
    QJsonParseError err;
    QJsonDocument json_recv = QJsonDocument::fromJson(allinfo.toUtf8(),&err);
    qDebug()<<"Json Error:"<<err.error;
    if(!json_recv.isNull())
    {
        QJsonObject object = json_recv.object();
        if(object.contains("errcode"))
        {
            QMessageBox::warning(this,"警告","城市输入错误，请输入正确的城市名");
            return;
        }
        city = object.value("city").toString();
        realtime = object.value("tem").toString();
        showcity = QString("%1 %2").arg(object.value("country").toString()).arg(city);
        reporttime = QString("%1 %2 %3").arg(object.value("date").toString()).arg(object.value("week").toString()).arg(object.value("update_time").toString());
        weather = object.value("wea").toString();
        temperature =QString("%1%2%3").arg(object.value("tem2").toString()).arg("~").arg(object.value("tem1").toString());
        wind = QString("%1 %3").arg(object.value("win").toString()).arg(object.value("win_speed").toString());
        humidity = object.value("humidity").toString();
        airlevel = object.value("air_level").toString();
        airtips = object.value("air_tips").toString();
    }
    display();
    reply->deleteLater();

}

void MainWindow::display()
{
    QString pc = QString("%1").arg(showcity);
    QString wea = QString("%1").arg(weather);
    QString real = QString("%1%2").arg(realtime).arg("℃");
    QString temp = QString("%1%2").arg(temperature).arg("℃");
    QString humi = QString("%1 %2").arg("湿度").arg(humidity);
    QString report = QString("%1 %2").arg(reporttime).arg("更新");
    QString level = QString("%1 %2").arg("空气质量").arg(airlevel);
    QString tip = QString("%1 %2").arg("温馨提示:").arg(airtips);
    ui->postion->setText(pc);
    ui->weather->setText(wea);
    ui->realtime->setText(real);
    ui->temp->setText(temp);
    ui->wind->setText(wind);
    ui->humitiy->setText(humi);
    ui->air->setText(level);
    ui->tips->setText(tip);
    ui->time->setText(report);
}

void MainWindow::send_net(QString city)
{
    QNetworkRequest quest;
    QString url = "https://v0.yiketianqi.com/api?unescape=1&version=v62&appid=98751895&appsecret=z1WXHdgM&city=";
    url = QString("%1%2").arg(url).arg(city);
    quest.setUrl(QUrl(url));
    manager->get(quest);
}

void MainWindow::on_pushButton_clicked()
{
    QString City = ui->lineEdit->text();
    if(City.isEmpty())
    {
        return;
    }
    send_net(City);
    ui->lineEdit->clear();
}

void MainWindow::on_pushButton_2_clicked()
{
    send_net(city);
}



