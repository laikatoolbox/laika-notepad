#include "TextEdit.h"
#include "LineNumberArea.h"

#include <QPainter>
#include <QTextBlock>

LaikaNotepad::TextEdit::TextEdit(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new LaikaNotepad::LineNumberArea(this);

    connect(this, &LaikaNotepad::TextEdit::blockCountChanged, this, &LaikaNotepad::TextEdit::updateLineNumberAreaWidth);
    connect(this, &LaikaNotepad::TextEdit::updateRequest, this, &LaikaNotepad::TextEdit::updateLineNumberArea);
    connect(this, &LaikaNotepad::TextEdit::cursorPositionChanged, this, &LaikaNotepad::TextEdit::highlightCurrentLine);

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
    if (showLineNumbers) {
        setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
    } else {
        setViewportMargins(0, 0, 0, 0);
    }

}

void LaikaNotepad::TextEdit::updateLineNumberArea(const QRect &rect, int dy)
{
    if (showLineNumbers) {
        if (dy) {
            lineNumberArea->scroll(0, dy);
        } else {
            lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
        }

        if (rect.contains(viewport()->rect())) {
            updateLineNumberAreaWidth(0);
        }
    } else {
        setViewportMargins(0, 0, 0, 0);
    }
}

void LaikaNotepad::TextEdit::resizeEvent(QResizeEvent *e)
{
    if (showLineNumbers) {
        QPlainTextEdit::resizeEvent(e);
        QRect cr = contentsRect();
        lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
    }
}

void LaikaNotepad::TextEdit::highlightCurrentLine()
{
    if (showLineNumbers) {
        this->repaint();
    }
}

void LaikaNotepad::TextEdit::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    if (showLineNumbers) {
        QPainter painter(lineNumberArea);
        painter.fillRect(event->rect(), Qt::lightGray);

        QTextBlock block = firstVisibleBlock();
        int blockNumber = block.blockNumber();
        int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
        int bottom = top + qRound(blockBoundingRect(block).height());
        QFont font = painter.font();

        while (block.isValid() && top <= event->rect().bottom()) {
            if (block.isVisible() && bottom >= event->rect().top()) {
                QString number = QString::number(blockNumber + 1);
                bool isCurrentBlock = this->textCursor().blockNumber() == blockNumber;
                if (isCurrentBlock) {
                    painter.setPen(Qt::red);
                    font.setBold(true);
                    painter.setFont(font);
                } else {
                    font.setBold(false);
                    painter.setFont(font);
                    painter.setPen(Qt::black);
                }
                painter.drawText(0, top, lineNumberArea->width() - lineNumberPaddding, fontMetrics().height(),
                                 Qt::AlignRight, number);
            }

            block = block.next();
            top = bottom;
            bottom = top + qRound(blockBoundingRect(block).height());
            ++blockNumber;
        }
    }
}

void LaikaNotepad::TextEdit::keyPressEvent(QKeyEvent *event)
{
    QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

    // make SHIFT + Enter be a plain newline character
    auto modifiers = keyEvent->modifiers();
    if ((modifiers & Qt::ShiftModifier) && (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return)) {
        event->setModifiers(modifiers & ~Qt::ShiftModifier);
    }

    QPlainTextEdit::keyPressEvent(event);
}

int LaikaNotepad::TextEdit::getDefaultFontSize()
{
    return this->defaultFontSize;
}

void LaikaNotepad::TextEdit::setShowLineNumbers(bool value)
{
    this->showLineNumbers = value;
    this->updateLineNumberAreaWidth(0);
    this->highlightCurrentLine();
    this->repaint();
}


bool LaikaNotepad::TextEdit::getShowLineNumbers()
{
    return this->showLineNumbers;
}
