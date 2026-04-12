#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include "AboutDialog.h"
#include "SettingsDialog.h"
#include "TextStats.h"
#include "settings/Sputnik.h"
#include <QFuture>
#include <QtConcurrent>
#include <QLocale>
#include <QClipboard>
#include <QFontDatabase>
#include <QMessageBox>
#include <QFileDialog>
#include <QScrollBar>
#include <QHeaderView>
#include <algorithm>
#include <iostream>

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

    this->ui->findTableView->setModel(&this->findModel);
    this->ui->findTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
    this->ui->findTableView->horizontalHeader()->resizeSections(QHeaderView::ResizeMode::ResizeToContents);

    connect(this->ui->findTableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));

    viewportTimer = new QTimer(this);
    connect(viewportTimer, SIGNAL(timeout()), this, SLOT(recalculateViewport()));
    viewportTimer->start(1500);
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

    // redraw the highlight to match the theme
    this->selectCurrentFindResult();

    if (manuallyRefreshedLabel != nullptr)
    {
        manuallyRefreshedLabel->setVisible(!this->settings->autoRefreshTextStats);
    }

    // For some reason, dock widgets do not behave correctly
    // when switching themes (dark vs light mode) and will still
    // have random bits stuck in the old theme, but we can force
    // QT to not be jank by forcing a style reload
    this->setStyleSheet(styleSheet());
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

void MainWindow::clearExtraSelections()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    this->ui->plainTextEdit->setExtraSelections(extraSelections);
}

void MainWindow::selectCurrentFindResult(bool shouldReselect, bool moveCursorToPosition)
{
    std::vector<FindResult*> selectedFinds = this->findModel.currentResults();

    int textLength = this->ui->plainTextEdit->toPlainText().length();

    if (textLength == 0 || selectedFinds.size() == 0)
    {
        this->clearExtraSelections();
        return;
    }

    this->recalculateViewport();

    // Show the results that are in the currently visible text
    QList<QTextEdit::ExtraSelection> extraSelections;
    for(const FindResult* selectedFind : selectedFinds)
    {
        // Skip if not in the viewport
        if (extraSelections.count() > 0 && (selectedFind->endPosition > this->viewportEnd || selectedFind->startPosition < this->viewportStart))
        {
            continue;
        }

        // Move the cursor to the position of the find, if needed
        if (moveCursorToPosition)
        {
            QTextCursor newCursor = QTextCursor(this->ui->plainTextEdit->textCursor());
            newCursor.setPosition(std::min(selectedFind->endPosition, textLength - 1));
            this->ui->plainTextEdit->setTextCursor(newCursor);
            this->ui->plainTextEdit->moveCursor(QTextCursor::MoveOperation::NextCharacter);
        }

        // Highlight the find
        QTextEdit::ExtraSelection extra;
        extra.format.setBackground(this->ui->plainTextEdit->currentLineNumberTextColorPen.brush());
        extra.format.setForeground(this->ui->plainTextEdit->lineNumberBackgroundColorBrush);
        extra.cursor = this->ui->plainTextEdit->textCursor();
        extra.cursor.setPosition(std::min(selectedFind->startPosition, textLength - 1));
        int distance = selectedFind->endPosition - selectedFind->startPosition;
        extra.cursor.movePosition(QTextCursor::MoveOperation::NextCharacter, QTextCursor::MoveMode::KeepAnchor, distance + 1);
        extraSelections.append(extra);

        // Make the selection match in the table if we should reselect
        if (shouldReselect && this->ui->findTableView->currentIndex().row() != selectedFind->resultPosition)
        {
            this->ui->findTableView->selectRow(selectedFind->resultPosition);
        }
    }

    this->ui->plainTextEdit->setExtraSelections(extraSelections);

    if (moveCursorToPosition)
    {
        this->ui->plainTextEdit->ensureCursorVisible();
    }

    // Focus the text edit
    this->ui->plainTextEdit->setFocus();
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
    this->findModel.decrementIndex();
    this->selectCurrentFindResult();
}


void MainWindow::on_findNextButton_clicked()
{    
    this->findModel.incrementIndex();
    this->selectCurrentFindResult();
}


void MainWindow::on_findAllButton_clicked()
{
    this->findModel.invalidate();
    QString textToFind = this->ui->findLineEdit->text();
    QString text = this->ui->plainTextEdit->toPlainText();

    int textToFindLength = textToFind.length();
    int maxCharFindLength = textToFindLength - 1;

    if (textToFindLength == 0)
    {
        return;
    }

    bool matchCase = this->findModel.matchCase;
    bool wholeWord = this->findModel.wholeWord;

    if (!matchCase)
    {
        textToFind = textToFind.toLower();
        text = text.toLower();
    }

    bool findRunning = true;
\
    int linePos = 0;
    int wordPos = 0;
    int resultPos = 0;
    const int totalChars = text.size();

    int startOfMatchSoFar = -1;
    int characterInMatchSoFar = 0;
    QChar prevChar = '\0';
    QChar curChar = '\0';

    for (int i = 0; i < totalChars; ++i)
    {
        curChar = text.at(i);
        bool isNewLine = curChar == '\n';
        bool isWhitespace = curChar.isSpace();
        bool isPunctuation = curChar.isPunct();

        if (isNewLine)
        {
            linePos++;
            prevChar = curChar;
            startOfMatchSoFar = -1;
            characterInMatchSoFar = 0;
            continue;
        }

        if (startOfMatchSoFar == -1 && curChar == textToFind[0])
        {
            startOfMatchSoFar = i;
        }

        // we are currently inside a match, running character by character
        if (startOfMatchSoFar != -1)
        {
            // check if the current char matches the relative character in the text to find
            if (characterInMatchSoFar < textToFindLength
                && curChar == textToFind[characterInMatchSoFar])
            {
                // everything has matched and we've reached the end, so add result and reset
                if (characterInMatchSoFar >= maxCharFindLength)
                {
                    this->findModel.append({resultPos, linePos + 1, startOfMatchSoFar, i});
                    resultPos++;
                    startOfMatchSoFar = -1;
                    characterInMatchSoFar = 0;
                }
                else
                {
                    characterInMatchSoFar++;
                }

            }
            else // character doesn't match so reset
            {
                startOfMatchSoFar = -1;
                characterInMatchSoFar = 0;
            }
        }

        prevChar = curChar;
    }

    // clear highlights and select first result
    this->clearExtraSelections();
    if (this->findModel.count() > 0)
    {
        this->findModel.currentResultIndices = {0};
        this->selectCurrentFindResult();
    }

    this->ui->findTableView->horizontalHeader()->resizeSections(QHeaderView::ResizeMode::Stretch);
}

void MainWindow::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    this->selectSelectedFinds();
}

void MainWindow::on_replaceSelectedButtonButton_clicked()
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


void MainWindow::on_findTableView_doubleClicked(const QModelIndex &index)
{
    //this->findModel.currentResultIndex = index.row();
    //this->selectCurrentFindResult();
}


void MainWindow::on_findTableView_activated(const QModelIndex &index)
{

}

void MainWindow::on_matchCaseCheckBox_checkStateChanged(const Qt::CheckState &arg1)
{
    this->findModel.matchCase = this->ui->matchCaseCheckBox->isChecked();
}


void MainWindow::on_wholeWordCheckBox_checkStateChanged(const Qt::CheckState &arg1)
{
    this->findModel.wholeWord = this->ui->wholeWordCheckBox->isChecked();
}


void MainWindow::on_clearResultsButton_clicked()
{
    this->findModel.invalidate();
    this->selectCurrentFindResult();
    this->clearExtraSelections();
    this->ui->findTableView->horizontalHeader()->resizeSections(QHeaderView::ResizeMode::Stretch);
}

void MainWindow::on_clearSelectionButton_clicked()
{
    this->ui->findTableView->selectionModel()->clearSelection();
}


void MainWindow::on_selectAllButton_clicked()
{
    this->isSelectingAll = true;
    this->ui->findTableView->selectAll();
    this->findModel.selectAll();
    this->isSelectingAll = false;
    this->selectCurrentFindResult(false);
}

void MainWindow::selectSelectedFinds()
{
    if (!this->isSelectingAll)
    {
        const QModelIndexList &selectedIndices = this->ui->findTableView->selectionModel()->selectedIndexes();
        std::vector<int> selectedFindIndices = {};
        for (int i = 0; i < selectedIndices.size(); i++)
        {
            selectedFindIndices.push_back(selectedIndices[i].row());
        }

        this->findModel.currentResultIndices = selectedFindIndices;
        this->selectCurrentFindResult(false);
    }
}

void MainWindow::recalculateViewport()
{
    if (this->findModel.currentResultIndices.size() == 0)
    {
        // we don't care about this if we're not trying to find any text
        return;
    }

    int oldViewportStart = this->viewportStart;
    int oldViewportEnd = this->viewportEnd;

    this->viewportStart = this->ui->plainTextEdit->cursorForPosition(QPoint(0, 0)).position();
    QPoint bottomRight(this->ui->plainTextEdit->viewport()->width() - 1, this->ui->plainTextEdit->viewport()->height() - 1);
    this->viewportEnd = this->ui->plainTextEdit->cursorForPosition(bottomRight).position();

    /*

    TODO: rely entirely on the calculations coming from startOfFirstBlock() and endOfLastBlock() for maybe? better performance
    std::cout << "From timer: " << this->viewportStart << " -> " << this->viewportEnd << std::endl;
    std::cout << "From textbox: " << this->ui->plainTextEdit->startOfFirstBlock() << " -> " << this->ui->plainTextEdit->endOfLastBlock() << std::endl;

    */

    // if the viewport has changed, we want to reselect the finds
    if (this->viewportStart != oldViewportStart || this->viewportEnd != oldViewportEnd)
    {
        this->selectCurrentFindResult(false, false);
    }
}