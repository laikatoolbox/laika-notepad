#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include "AboutDialog.h"
#include "SettingsDialog.h"
#include "TextStats.h"
#include <QFuture>
#include <QtConcurrent>
#include <QLocale>
#include <QClipboard>
#include <QFontDatabase>
#include <QMessageBox>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // load settings
    this->ui->actionLine_Numbers->setChecked(this->settings->showLineNumbers);
    this->ui->actionWord_Wrap->setChecked(this->settings->wordWrap);
    this->ui->actionAuto_Refresh_Text_Stats->setChecked(this->settings->autoRefreshTextStats);
    this->ui->actionStatus_Bar->setChecked(this->settings->showStatusBar);

    // set up undo/redo
    connect(ui->plainTextEdit, SIGNAL(undoAvailable(bool)), this, SLOT(undoAvailable(bool)));
    connect(ui->plainTextEdit, SIGNAL(redoAvailable(bool)), this, SLOT(redoAvailable(bool)));
    this->ui->actionUndo->setEnabled(false);
    this->ui->actionRedo->setEnabled(false);

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
    this->updateStats();
    this->updateWindowTitle();
    this->ui->findTableVIew->setModel(&this->findModel);
}

MainWindow::~MainWindow()
{
    delete this->settings;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // check if we have a modified document and ask to save if we do
    if (this->ui->plainTextEdit->document()->isModified())
    {
        bool userCancelled = this->modifiedDocumentGuard();

        // user cancelled, so bail out of closing
        if (userCancelled)
        {
            event->ignore();
            return;
        }
    }

    // we either don't have a modified document, the user
    // saved the modified document, or the user decided not to
    // save but close anyways
    event->accept();
}

void MainWindow::changeEvent(QEvent *event) {
    // we want to notify the text edit that
    // the theme has changed so it can update colors
    if (event->type() == QEvent::ThemeChange)
    {
        this->settingsChanged();
    }

    // call overridden function
    QMainWindow::changeEvent(event);
}

void MainWindow::settingsChanged()
{
    ui->plainTextEdit->setSettings(this->settings);
    ui->plainTextEdit->setWordWrapMode(this->settings->wordWrap ? QTextOption::WrapMode::WordWrap : QTextOption::WrapMode::NoWrap);
    ui->statusbar->setVisible(this->settings->showStatusBar);

    // toolbars
    ui->mainToolbar->setMovable(!this->settings->lockToolbars);
    ui->viewToolbar->setMovable(!this->settings->lockToolbars);

    if (manuallyRefreshedLabel != nullptr)
    {
        manuallyRefreshedLabel->setVisible(!this->settings->autoRefreshTextStats);
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
        bool isPrevWhitespace = false;

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

void MainWindow::newDocument()
{
    ui->plainTextEdit->document()->clear();
    this->clearFileName();
}

bool MainWindow::modifiedDocumentGuard()
{
    if (ui->plainTextEdit->document()->isModified())
    {
        QMessageBox::StandardButton reply =
            QMessageBox::question(this, "Document Modified", "Do you want to save changes to the current document?", QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
        if (reply == QMessageBox::Yes)
        {
            this->saveDocument();
        }
        else if (reply == QMessageBox::No)
        {
            this->newDocument();
        }
        else
        {
            // return true because we're cancelling
            return true;
        }
    }
    else
    {
        this->newDocument();
    }

    return false;
}

void MainWindow::openDocumentFrom(QString &fileName)
{
    this->modifiedDocumentGuard();

    QFile file(fileName);
    if (file.open(QFile::ReadOnly))
    {
        QString fileText = file.readAll();
        this->ui->plainTextEdit->setPlainText(fileText);
        this->setFileName(fileName);
        this->ui->plainTextEdit->document()->setModified(false);
        file.close();
    }
}

void MainWindow::saveDocument()
{
    if (this->fileName.isEmpty())
    {
        this->saveDocumentAs();
    }
    else
    {
        this->saveDocumentTo(this->fileName);
    }
}

void MainWindow::saveDocumentAs()
{
    QString saveFileName = QFileDialog::getSaveFileName(this, QString(), QDir::homePath(), "Text files (*.txt);;All files (*.*)");

    if (!saveFileName.isEmpty())
    {
        this->saveDocumentTo(saveFileName);
    }
}

void MainWindow::saveDocumentTo(QString &fileName)
{
    QFile saveFile(fileName);
    if (saveFile.open(QFile::WriteOnly|QIODevice::Truncate|QIODevice::Text))
    {
        QTextStream out(&saveFile);
        out << ui->plainTextEdit->toPlainText();
        saveFile.close();
        this->setFileName(fileName);
        this->ui->plainTextEdit->document()->setModified(false);
    }
}

void MainWindow::updateWindowTitle()
{
    bool documentModified = this->ui->plainTextEdit->document()->isModified();
    QString modifiedText = documentModified ? " (" + tr("modified") + ")" : "";
    this->setWindowModified(documentModified);
    this->setWindowFilePath(this->fileName);

    if (this->fileName.isEmpty())
    {
        this->setWindowTitle(tr("Untitled") + modifiedText +  + " - Laika Notepad");
    }
    else
    {
        const QFileInfo info(this->fileName);
        this->setWindowTitle(info.fileName() + modifiedText + " - Laika Notepad");
    }
}

void MainWindow::setFileName(QString &fileName)
{
    this->fileName = QString(fileName);
    this->updateWindowTitle();
}

void MainWindow::clearFileName()
{
    this->fileName = "";
    this->updateWindowTitle();
}

void MainWindow::on_actionNew_triggered()
{
    this->modifiedDocumentGuard();
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
    if (this->settings->showStatusBar && this->settings->autoRefreshTextStats)
    {
        updateStats();
    }
}

void MainWindow::on_action_Refresh_Text_Stats_triggered()
{
    updateStats();
}


void MainWindow::on_actionLine_Numbers_changed()
{
    this->settings->showLineNumbers = this->ui->actionLine_Numbers->isChecked();
    this->settingsChanged();
}


void MainWindow::on_actionWord_Wrap_changed()
{
    this->settings->wordWrap = this->ui->actionWord_Wrap->isChecked();
    this->settingsChanged();
}


void MainWindow::on_actionAuto_Refresh_Text_Stats_changed()
{
    this->settings->autoRefreshTextStats = this->ui->actionAuto_Refresh_Text_Stats->isChecked();
    this->settingsChanged();
}


void MainWindow::on_actionStatus_Bar_changed()
{
    this->settings->showStatusBar = this->ui->actionStatus_Bar->isChecked();
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


void MainWindow::on_actionAbout_QT_triggered()
{
    QMessageBox::aboutQt(this);
}


void MainWindow::on_actionUndo_triggered()
{
    ui->plainTextEdit->undo();
}


void MainWindow::on_actionRedo_triggered()
{
    ui->plainTextEdit->redo();
}


void MainWindow::on_action_Copy_triggered()
{
    ui->plainTextEdit->copy();
}


void MainWindow::on_actionPaste_triggered()
{
    ui->plainTextEdit->paste();
}


void MainWindow::on_actionCut_triggered()
{
    ui->plainTextEdit->cut();
}


void MainWindow::undoAvailable(bool canUndo)
{
    this->ui->actionUndo->setEnabled(canUndo);
}

void MainWindow::redoAvailable(bool canRedo)
{
    this->ui->actionRedo->setEnabled(canRedo);
}

void MainWindow::on_actionSave_As_triggered()
{
    this->saveDocumentAs();
}


void MainWindow::on_actionSave_triggered()
{
    this->saveDocument();
}


void MainWindow::on_actionNew_From_Clipboard_triggered()
{
    this->modifiedDocumentGuard();
    ui->plainTextEdit->paste();
}


void MainWindow::on_plainTextEdit_modificationChanged(bool arg1)
{
    this->updateWindowTitle();
}


void MainWindow::on_actionLock_Toolbars_changed()
{
    this->settings->lockToolbars = ui->actionLock_Toolbars->isChecked();
    this->settingsChanged();
}


void MainWindow::on_actionOpen_triggered()
{
    QString openFileName = QFileDialog::getOpenFileName(this, QString(), QDir::homePath(), "Text files (*.txt);;All files (*.*)");

    if (!openFileName.isEmpty())
    {
        this->openDocumentFrom(openFileName);
    }
}


void MainWindow::on_findPreviousButton_clicked()
{

}


void MainWindow::on_findNextButton_clicked()
{

}


void MainWindow::on_findAllButton_clicked()
{
    this->findModel.invalidate();
    std::string textToFind = this->ui->findLineEdit->text().toStdString();
    std::string text = this->ui->plainTextEdit->toPlainText().toStdString();
    int textToFindLength = textToFind.length();

    if (textToFindLength == 0)
    {
        return;
    }

    int pos = text.find(textToFind);

    if (pos != std::string::npos)
    {
        this->findModel.append({pos, pos+textToFindLength});
    }

    while(pos != std::string::npos)
    {
        pos = text.find(textToFind, pos+1);
        if (pos != std::string::npos)
        {
            this->findModel.append({pos, pos+textToFindLength});
        }
    }

}


void MainWindow::on_replaceNextButton_clicked()
{

}


void MainWindow::on_replaceAllButton_clicked()
{

}


void MainWindow::on_actionFind_Replace_triggered()
{
    bool findReplaceVisible = !this->ui->findReplaceDock->isVisible();
    this->ui->findReplaceDock->setVisible(findReplaceVisible);

    if (findReplaceVisible)
    {
        this->ui->findLineEdit->setFocus();
    }
    else
    {
        this->ui->plainTextEdit->setFocus();
    }
}


void MainWindow::on_findTableVIew_doubleClicked(const QModelIndex &index)
{
    FindResult *selectedFind = this->findModel.resultAt(index.row());

    if (selectedFind != nullptr)
    {
        QList<QTextEdit::ExtraSelection> extraSelections;
        QTextEdit::ExtraSelection extra;
        extra.format.setBackground(Qt::red);

        extra.cursor = this->ui->plainTextEdit->textCursor();
        extra.cursor.setPosition(selectedFind->startPosition);
        extra.cursor.select(QTextCursor::WordUnderCursor);

        extraSelections.append(extra);
        this->ui->plainTextEdit->setExtraSelections(extraSelections);
    }
}

