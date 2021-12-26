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
 * 函数名：MainWindow::analyWeatherXML
 * 功能：解析Api返回的数据
 * 参数：QByteArray json
 * 返回值：无
 */
void MainWindow::analyWeatherXML(QByteArray json)
{
    if(json.isEmpty())
        return ;

    QString date[5] = {"NULL"}; //存储日期
    QJsonParseError err;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(json, &err);

    QJsonObject jsonObj = jsonDoc.object().value("data").toObject();
    QJsonArray forecast = jsonObj.value("forecast").toArray();

    QJsonObject cityInfo = jsonDoc.object().value("cityInfo").toObject();
    ui->labelCity->setText(cityInfo.value("parent").toString() +
                           cityInfo.value("city").toString());
    ui->labelTemperature->setText(jsonObj.value("wendu").toString());
    ui->labelTime->setText("最后更新于：" +
                           jsonDoc.object().value("time").toString());

    QJsonObject yesterday = jsonObj.value("yesterday").toObject(); //昨天
    ui->textBrowserYesterday->clear();
    ui->textBrowserYesterday->append(JsonObj2String(yesterday));
    date[0] = yesterday.value("ymd").toString();
    ui->groupBoxYesterday->setTitle(date[0]);

    QJsonObject today = forecast[0].toObject(); //今天
    ui->textBrowserToday->clear();
    ui->textBrowserToday->append(JsonObj2String(today));
    ui->labelFengsu->setText("风向：" + today.value("fx").toString() +
                             today.value("fl").toString());
    dealIcon(jsonObj);
    date[1] = today.value("ymd").toString();
    ui->groupBoxToday->setTitle(date[1]);

    QJsonObject tomorrow = forecast[1].toObject(); //明天
    ui->textBrowserTomorrow->clear();
    ui->textBrowserTomorrow->append(JsonObj2String(tomorrow));
    date[2] = tomorrow.value("ymd").toString();
    ui->groupBoxTomorrow->setTitle(date[2]);

    QJsonObject day_3 = forecast[2].toObject(); //后天
    ui->textBrowserDay_3->clear();
    ui->textBrowserDay_3->append(JsonObj2String(day_3));
    date[3] = day_3.value("ymd").toString();
    ui->groupBoxDay_3->setTitle(date[3]);

    QJsonObject day_4 = forecast[3].toObject(); //大后天
    ui->textBrowserDay_4->clear();
    ui->textBrowserDay_4->append(JsonObj2String(day_4));
    date[4] = day_4.value("ymd").toString();
    ui->groupBoxDay_4->setTitle(date[4]);
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
