#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include "AboutDialog.h"
#include "SettingsDialog.h"
#include "settings/SettingsStore.h"
#include "TextStats.h"
#include <QFuture>
#include <QtConcurrent>
#include <QLocale>
#include <QClipboard>
#include <QFontDatabase>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Load settings
    this->ui->actionLine_Numbers->setChecked(LaikaSettings::showLineNumbers);
    this->ui->actionWord_Wrap->setChecked(LaikaSettings::wordWrap);
    this->ui->actionAuto_Refresh_Text_Stats->setChecked(LaikaSettings::autoRefreshTextStats);
    this->ui->actionStatus_Bar->setChecked(LaikaSettings::showStatusBar);

    // set up statusbar
    manuallyRefreshedLabel = new QLabel(this);
    manuallyRefreshedLabel->setText("(Manually Refreshed)");
    ui->statusbar->addPermanentWidget(manuallyRefreshedLabel);

    lineCountLabel = new QLabel(this);
    lineCountLabel->setText("Lines: ");
    ui->statusbar->addPermanentWidget(lineCountLabel);
    lineCountNumberLabel = new QLabel(this);
    ui->statusbar->addPermanentWidget(lineCountNumberLabel);

    characterCountLabel = new QLabel(this);
    characterCountLabel->setText("Characters: ");
    ui->statusbar->addPermanentWidget(characterCountLabel);
    characterCountNumberLabel = new QLabel(this);
    ui->statusbar->addPermanentWidget(characterCountNumberLabel);

    wordCountLabel = new QLabel(this);
    wordCountLabel->setText("Words: ");
    ui->statusbar->addPermanentWidget(wordCountLabel);
    wordCountNumberLabel = new QLabel(this);
    ui->statusbar->addPermanentWidget(wordCountNumberLabel);

    this->settingsChanged();
    updateStats();
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


void MainWindow::on_plainTextEdit_textChanged()
{
    if (LaikaSettings::showStatusBar && LaikaSettings::autoRefreshTextStats)
    {
        updateStats();
    }
}

void MainWindow::settingsChanged()
{    
    ui->plainTextEdit->setShowLineNumbers(LaikaSettings::showLineNumbers);
    ui->plainTextEdit->setWordWrapMode(LaikaSettings::wordWrap ? QTextOption::WrapMode::WordWrap : QTextOption::WrapMode::NoWrap);
    ui->statusbar->setVisible(LaikaSettings::showStatusBar);

    if (manuallyRefreshedLabel != nullptr)
    {
        manuallyRefreshedLabel->setVisible(!LaikaSettings::autoRefreshTextStats);
    }
}

///
/// \brief MainWindow::updateStats Get the stats for the document's text.
/// I know it's not pretty, but it is orders of magnitude faster than the regex equivalent.
///
void MainWindow::updateStats()
{
    std::string text = ui->plainTextEdit->toPlainText().toStdString();
    QFuture<TextStats> future = QtConcurrent::run([&text] {
        int size = text.size();
        int wordCount = 0;
        int lineCount = 1;
        bool isPrevWhitespace = ' ';

        for(unsigned int i = 0; i < size; i++)
        {
            char cur = text[i];
            bool isCurrentCharNotWhitespace = (cur != ' ' && cur != '\t' && cur != '\n' && cur != '\r' && cur != '\v' && cur != '\f');

            if (isCurrentCharNotWhitespace && isPrevWhitespace)
            {
                wordCount++;
            }

            if (cur == '\n')
            {
                lineCount++;
            }

            isPrevWhitespace = (cur == ' ' || cur == '\t' || cur == '\n' || cur == '\r' || cur == '\v' || cur == '\f');
        }

        TextStats stats = { .lineCount = lineCount, .wordCount = wordCount, .characterCount = size };

        return stats;
    });
    TextStats result = future.result();

    lineCountNumberLabel->setText(QLocale().toString(result.lineCount));
    wordCountNumberLabel->setText(QLocale().toString(result.wordCount));
    characterCountNumberLabel->setText(QLocale().toString(result.characterCount));
}

void MainWindow::on_action_Refresh_Text_Stats_triggered()
{
    updateStats();
}


void MainWindow::on_actionLine_Numbers_changed()
{
    LaikaSettings::showLineNumbers = this->ui->actionLine_Numbers->isChecked();
    this->settingsChanged();
}


void MainWindow::on_actionWord_Wrap_changed()
{
    LaikaSettings::wordWrap = this->ui->actionWord_Wrap->isChecked();
    this->settingsChanged();
}


void MainWindow::on_actionAuto_Refresh_Text_Stats_changed()
{
    LaikaSettings::autoRefreshTextStats = this->ui->actionAuto_Refresh_Text_Stats->isChecked();
    this->settingsChanged();
}


void MainWindow::on_actionStatus_Bar_changed()
{
    LaikaSettings::showStatusBar = this->ui->actionStatus_Bar->isChecked();
    this->settingsChanged();
}


void MainWindow::on_actionGo_To_Start_triggered()
{
    ui->plainTextEdit->moveCursor(QTextCursor::MoveOperation::Start);
}


void MainWindow::on_actionGo_To_End_triggered()
{
    ui->plainTextEdit->moveCursor(QTextCursor::MoveOperation::End);
}


void MainWindow::on_actionCopy_A_ll_triggered()
{
    QClipboard *clipboard = QApplication::clipboard();
    QString allText = ui->plainTextEdit->toPlainText();
    clipboard->setText(allText);
}


void MainWindow::on_actionZoom_In_triggered()
{
    ui->plainTextEdit->zoomIn(1);
}


void MainWindow::on_actionZoom_Out_triggered()
{
    ui->plainTextEdit->zoomOut(1);
}


void MainWindow::on_action_Reset_Zoom_triggered()
{
    QFont font = ui->plainTextEdit->font();
    font.setPointSize(ui->plainTextEdit->getDefaultFontSize());
    ui->plainTextEdit->setFont(font);
}


void MainWindow::on_actionSelect_All_triggered()
{
    ui->plainTextEdit->selectAll();
}


void MainWindow::on_actionDeselect_All_triggered()
{
    QTextCursor cursor = ui->plainTextEdit->textCursor();
    cursor.clearSelection();
    ui->plainTextEdit->setTextCursor(cursor);
}

