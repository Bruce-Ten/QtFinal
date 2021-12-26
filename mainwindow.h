#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkReply>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QString cityID;
    QString message;
private slots:
    void on_refreshButton_clicked();

    void on_changeButton_clicked();

    void on_cutButton_onclicked();

private:
    Ui::MainWindow *ui;
    void onGetWeather();
    void onReplyFinished(QNetworkReply* reply);
    void analyWeatherXML(QByteArray json);
};

#endif // MAINWINDOW_H
