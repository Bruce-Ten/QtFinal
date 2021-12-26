#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QNetworkReply>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(mNetManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(onReplyFinished(QNetworkReply*)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * 函数名：MainWindow::onGetWeather
 * 功能：访问天气Api并捕获返回的数据
 * 参数：无
 * 返回值：无
 */
void MainWindow::onGetWeather()
{
    qDebug() << QTime::currentTime().toString();
    //天气预报V2接口
    //http://t.weather.sojson.com/api/weather/city/v2/101180710
    QString localApi = "http://t.weather.itboy.net/api/weather/city/101180710";
    QString webApi = "http://t.weather.itboy.net/api/weather/city/";

    if(!cityID.isEmpty())
        mNetRequest->setUrl(QUrl(webApi + cityID));
    else
        mNetRequest->setUrl(QUrl(localApi));

    mNetRequest->setHeader(QNetworkRequest::UserAgentHeader, "RT-Thread ART");
    mNetManager->get(*mNetRequest);
}

/**
 * 函数名：MainWindow::onReplyFinished
 * 功能：将捕获的数据转换为字符串，便于调试
 * 参数：QNetworkReply *reply
 * 返回值：无
 */
void MainWindow::onReplyFinished(QNetworkReply* reply)
{
    qDebug() << QTime::currentTime().toString();

    QByteArray weather = reply->readAll();

    if(!weather.isEmpty())
        analyWeatherXML(weather); //解析天气信息的数据

    reply->deleteLater();
}



/**
 * 函数名：MainWindow::JsonObj2String
 * 功能：Json对象转换为String对象
 * 参数：const QJsonObject jsonObj
 * 返回值：QString对象
 */
QString MainWindow::JsonObj2String(const QJsonObject jsonObj)
{
    QString weather;

    if(!jsonObj.isEmpty())
    {
        weather += jsonObj.value("week").toString() + "\n";

        if(!jsonObj.value("fx").toString().isEmpty())
            weather += jsonObj.value("fx").toString() + "\n";
        else
            weather += jsonObj.value("fengxiang").toString() + "\n";

        if(!jsonObj.value("ganmao").toString().isEmpty())
            weather += jsonObj.value("ganmao").toString() + "\n";

        weather += jsonObj.value("high").toString() + "\n";
        weather += jsonObj.value("low").toString() + "\n";
        weather += jsonObj.value("type").toString() + "\n";
        weather += jsonObj.value("notice").toString();
    }

    return weather;
}

/**
 * 函数名：MainWindow::serachCity
 * 功能：搜索输入框输入的城市
 * 参数：无
 * 返回值：无
 */
void MainWindow::searchCity()
{
    QFile loadFile("://data/cityData.json");//文件为城市代码数据库

    if(!loadFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, QString("警告"),
                             QString("无法加载城市数据库！\n请联系开发者"));
        return ;
    }

    QByteArray allData = loadFile.readAll();//读取数据
    loadFile.close();

    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(allData, &json_error));

    if(json_error.error != QJsonParseError::NoError)
    {
        qDebug() << "json error!";
        return ;
    }

    if(jsonDoc.isNull() || jsonDoc.isEmpty())
    {
        qInfo() << "parse json null or empty";
        return ;
    }

    QVariantList list = jsonDoc.toVariant().toList();//把数据库转换为数列
    QString searchData;

    for(int i = 0; i < list.count(); i++)
    {
        QVariantMap map = list[i].toMap();
        searchData = ui->searchBar->text();

        //这里用contains函数，而不是直接比较的原因是，数据库里市级城市名字不包含市，
        //所以用contains兼用性更好
        if(searchData.contains(map["city_name"].toString()))
            cityID = map["city_code"].toString();

        if(map["city_name"].toString().contains(searchData))
            cityID = map["city_code"].toString();
    }

    if(QString(cityID).isEmpty())
    {
        QMessageBox::warning(this, QString("警告"),
                             QString("没有找到" + searchData + "的城市ID" +
                                     "\n请检查重试"));
        return ;
    }

    QTimer::singleShot(1500, this, SLOT(onGetWeather()));
}
