#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_actionNew_triggered();

    void on_actionSettings_triggered();

    void on_actionAbout_triggered();

    void on_plainTextEdit_textChanged();

    void on_action_Refresh_Text_Stats_triggered();

    void on_actionLine_Numbers_changed();

    void on_actionWord_Wrap_changed();

    void on_actionAuto_Refresh_Text_Stats_changed();

    void on_actionStatus_Bar_changed();

    void on_actionGo_To_Start_triggered();

    void on_actionGo_To_End_triggered();

    void on_actionCopy_A_ll_triggered();

    void on_actionZoom_In_triggered();

    void on_actionZoom_Out_triggered();

    void on_action_Reset_Zoom_triggered();

    void on_actionSelect_All_triggered();

    void on_actionDeselect_All_triggered();

    void on_actionAbout_QT_triggered();

private:
    Ui::MainWindow *ui;
    QLabel *manuallyRefreshedLabel = nullptr;
    QLabel *lineCountLabel = nullptr;
    QLabel *lineCountNumberLabel = nullptr;
    QLabel *characterCountLabel = nullptr;
    QLabel *characterCountNumberLabel = nullptr;
    QLabel *wordCountLabel = nullptr;
    QLabel *wordCountNumberLabel = nullptr;
    void settingsChanged();
    void updateStats();
};
#endif // MAINWINDOW_H
