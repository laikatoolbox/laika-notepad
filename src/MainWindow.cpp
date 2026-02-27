#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include "AboutDialog.h"
#include "SettingsDialog.h"

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

void MainWindow::on_actionSettings_triggered()
{
    SettingsDialog *settingsDialog  = new SettingsDialog();
    settingsDialog->show();
}


void MainWindow::on_actionAbout_triggered()
{
    AboutDialog *aboutDialog  = new AboutDialog();
    aboutDialog->show();
}

