#include "TextEdit.h"
#include "LineNumberArea.h"
#include <QPalette>
#include <QPainter>
#include <QTextBlock>
#include <QApplication>
#include <QStyleHints>
#include <QStyle>

LaikaNotepad::TextEdit::TextEdit(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new LaikaNotepad::LineNumberArea(this);

    connect(this, &LaikaNotepad::TextEdit::blockCountChanged, this, &LaikaNotepad::TextEdit::updateLineNumberAreaWidth);
    connect(this, &LaikaNotepad::TextEdit::updateRequest, this, &LaikaNotepad::TextEdit::updateLineNumberArea);
    connect(this, &LaikaNotepad::TextEdit::cursorPositionChanged, this, &LaikaNotepad::TextEdit::highlightCurrentLine);

    this->setSettings(nullptr);
    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
    this->defaultFontSize = this->font().pointSize();
}

int LaikaNotepad::TextEdit::lineNumberAreaWidth()
{
    auto digits = std::max(numDigits(blockCount()), 2);
    int space = lineNumberPaddding*2 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}

void LaikaNotepad::TextEdit::updateLineNumberAreaWidth(int newBlockCount)
{
    if (showLineNumbers)
    {
        setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
    }
    else
    {
        setViewportMargins(0, 0, 0, 0);
    }
}

void LaikaNotepad::TextEdit::updateLineNumberArea(const QRect &rect, int dy)
{
    if (showLineNumbers)
    {
        if (dy)
        {
            lineNumberArea->scroll(0, dy);
        }
        else
        {
            lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
        }

        if (rect.contains(viewport()->rect()))
        {
            updateLineNumberAreaWidth(0);
        }
    }
    else
    {
        setViewportMargins(0, 0, 0, 0);
    }
}

void LaikaNotepad::TextEdit::resizeEvent(QResizeEvent *e)
{
    if (showLineNumbers)
    {
        QPlainTextEdit::resizeEvent(e);
        QRect cr = contentsRect();
        lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
    }
}

void LaikaNotepad::TextEdit::highlightCurrentLine()
{
    if (showLineNumbers)
    {
        this->repaint();
    }
}

void LaikaNotepad::TextEdit::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    if (this->showLineNumbers || this->shouldCalculateBlocks)
    {
        QPainter painter(lineNumberArea);
        QFont font = painter.font();
        if (this->showLineNumbers)
        {
            painter.fillRect(event->rect(), this->lineNumberBackgroundColorBrush);
        }

        QTextBlock block = firstVisibleBlock();
        // save the starting char position
        this->startChar = block.position();
        int blockNumber = block.blockNumber();
        int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
        int bottom = top + qRound(blockBoundingRect(block).height());

        while (block.isValid() && top <= event->rect().bottom())
        {
            if (this->showLineNumbers)
            {
                if (block.isVisible() && bottom >= event->rect().top())
                {
                    QString number = QString::number(blockNumber + 1);
                    bool isCurrentBlock = this->textCursor().blockNumber() == blockNumber;
                    if (isCurrentBlock)
                    {
                        painter.setPen(this->currentLineNumberTextColorPen);
                        font.setBold(true);
                        painter.setFont(font);
                    }
                    else
                    {
                        font.setBold(false);
                        painter.setFont(font);
                        painter.setPen(this->lineNumberTextColorPen);
                    }

                    painter.drawText(0, top, lineNumberArea->width() - lineNumberPaddding, fontMetrics().height(),
                                     Qt::AlignRight, number);
                }
            }

            block = block.next();
            top = bottom;
            bottom = top + qRound(blockBoundingRect(block).height());
            ++blockNumber;
        }

        // save the ending char position
        this->endChar = block.position() + block.length();
    }
}

void LaikaNotepad::TextEdit::keyPressEvent(QKeyEvent *event)
{
    QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

    // make SHIFT + Enter be a plain newline character
    auto modifiers = keyEvent->modifiers();
    if ((modifiers & Qt::ShiftModifier) && (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return))
    {
        event->setModifiers(modifiers & ~Qt::ShiftModifier);
    }

    QPlainTextEdit::keyPressEvent(event);
}

int LaikaNotepad::TextEdit::getDefaultFontSize()
{
    return this->defaultFontSize;
}

int LaikaNotepad::TextEdit::startOfFirstBlock()
{
    if (this->shouldCalculateBlocks || this->showLineNumbers)
    {
        return this->startChar;
    }


    // we aren't calculating them right now
    return -1;
}

int LaikaNotepad::TextEdit::endOfLastBlock()
{
    if (this->shouldCalculateBlocks || this->showLineNumbers)
    {
        return this->endChar;
    }


    // we aren't calculating them right now
    return -1;
}

void LaikaNotepad::TextEdit::setSettings(LaikaSettings::SettingsStore *settings)
{
    // load basic values from settings or set defaults if no settings
    if (settings != nullptr)
    {
        this->showLineNumbers = settings->showLineNumbers;
    }
    else
    {
        this->showLineNumbers = LaikaSettings::SettingsStore::defaultShowLineNumbers;
    }

    // load theme values from settings or set defaults if no settings/theme
    if (settings != nullptr && settings->currentTheme != nullptr) // we have a theme
    {
        // basic text edit palette
        QPalette palette = this->palette();
        palette.setColor(QPalette::Base, settings->currentTheme->backgroundColor); // background
        palette.setColor(QPalette::Text, settings->currentTheme->textColor); // text
        this->setPalette(palette);

        // line number colors
        this->lineNumberBackgroundColorBrush = QBrush(settings->currentTheme->lineNumberBackgroundColor);
        this->lineNumberTextColorPen = QPen(settings->currentTheme->lineNumberTextColor);
        this->currentLineNumberTextColorPen = QPen(settings->currentTheme->currentLineNumberTextColor);
    }
    else // just use defaults
    {
        // basic text edit palette
        QStyle *style = qApp->style();
        QPalette palette = style->standardPalette();
        this->setPalette(palette);

        // line number colors
        QColor lineNumberBackgroundColor = palette.window().color().toHsl();
        QColor lineNumberTextColor = palette.window().color().toHsl(); // we base this off the window color
        QColor currentLineNumberTextColor = palette.accent().color().toHsl();

        Qt::ColorScheme currentScheme = QGuiApplication::styleHints()->colorScheme();

        if (currentScheme == Qt::ColorScheme::Dark)
        {
            lineNumberBackgroundColor.setHsv(
                lineNumberBackgroundColor.hsvHue(),
                lineNumberBackgroundColor.hsvSaturation(),
                std::min(lineNumberBackgroundColor.value() + 15, 255)
            );
            lineNumberTextColor.setHsv(
                lineNumberTextColor.hsvHue(),
                lineNumberTextColor.hsvSaturation(),
                std::min(lineNumberTextColor.value() + 125, 255)
            );
            currentLineNumberTextColor.setHsv(
                currentLineNumberTextColor.hsvHue(),
                std::max(currentLineNumberTextColor.hsvSaturation() - 25, 0),
                std::min(currentLineNumberTextColor.value() + 40, 255)
            );
        }
        else
        {
            lineNumberBackgroundColor.setHsv(
                lineNumberBackgroundColor.hsvHue(),
                lineNumberBackgroundColor.hsvSaturation(),
                std::max(lineNumberBackgroundColor.value() - 15, 0)
            );
            lineNumberTextColor.setHsv(
                lineNumberTextColor.hsvHue(),
                lineNumberTextColor.hsvSaturation(),
                std::max(lineNumberTextColor.value() - 125, 0)
            );
            currentLineNumberTextColor.setHsv(
                currentLineNumberTextColor.hsvHue(),
                std::min(currentLineNumberTextColor.hsvSaturation() + 50, 255),
                std::max(currentLineNumberTextColor.value() - 40, 0)
            );
        }

        this->lineNumberBackgroundColorBrush = QBrush(lineNumberBackgroundColor);
        this->lineNumberTextColorPen = QPen(lineNumberTextColor);
        this->currentLineNumberTextColorPen = QPen(currentLineNumberTextColor);
    }

    this->updateLineNumberAreaWidth(0);
    this->highlightCurrentLine();
    this->repaint();
}
