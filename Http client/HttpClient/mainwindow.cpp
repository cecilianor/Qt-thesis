#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupSlots();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::printReplyContent(QNetworkReply *reply)
{
    if(reply->error())
    {
        ui->ContentText->setText("");
        ui->Statustext->setText(QString("Request failed with the following error: ") + reply->errorString());
        return;
    }
    ui->Statustext->setText(QString("Seccess!"));
    QByteArray replyArray(reply->readAll());
    QString replyText(QString::fromStdString(replyArray.toStdString()));
    ui->ContentText->setText(replyText);
}

void MainWindow::sendBtnClicked()
{
    controller.sendRequest(ui->UrlInput->text());
}

void MainWindow::setupSlots()
{
    connect(ui->SendBtn, &QPushButton::clicked, this, &MainWindow::sendBtnClicked);
    connect(&controller, &NetworkController::finished, this, &MainWindow::printReplyContent);

}

