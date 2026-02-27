#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include "AboutDialog.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionNew_triggered()
{

}


void MainWindow::on_action_About_triggered()
{
    AboutDialog *aboutDialog  = new AboutDialog();
    aboutDialog->show();
}

