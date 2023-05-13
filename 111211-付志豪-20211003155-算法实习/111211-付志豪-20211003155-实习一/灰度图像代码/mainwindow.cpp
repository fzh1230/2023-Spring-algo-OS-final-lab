#include "C_Ucompress.h"
#include "dptocompress.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QFileDialog>
#include<fstream>
#include<QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->progressBar->setValue(0);
    setWindowTitle("BMP位图压缩/by.付志豪");
}


MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::on_searchBtn_clicked()
{
    QString path=QFileDialog::getOpenFileName(this,tr("BMP"),".",tr("(*.bmp)"));
    ui->path->setPlainText(path);
}
 void MainWindow:: on_sucpsBtn_clicked()
 {
     QString path=QFileDialog::getOpenFileName(this,tr("dp"),".",tr("(*.mdp)"));
     ui->upath->setPlainText(path);
 }
void MainWindow::on_cpsBtn_clicked()
{
    QString path=ui->path->toPlainText();
    std::string filename=path.toStdString();
    bool scu=compress(filename);
    if(scu)
    {
        QMessageBox::information(this,tr("恭喜你"),tr("压缩成功"));
    }
    else
    {
        QMessageBox::information(this,tr("对不起"),tr("压缩失败"));
    }
}
void MainWindow::on_uncpsBtn_clicked()
{
    QString path = ui->upath->toPlainText();
    std::string name=path.toStdString();

    bool IsSucceed=unCompress(name);
    if (IsSucceed==1)
    {

        QMessageBox::about(NULL, "恭喜你", "解压成功");
    }
    else
    {
        QMessageBox::about(NULL, "对不起", "文件异常");
    }
}
