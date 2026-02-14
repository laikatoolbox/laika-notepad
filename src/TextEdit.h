#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QPlainTextEdit>

namespace LaikaNotepad {
    class TextEdit : public QPlainTextEdit {
        Q_OBJECT

    public:
        TextEdit(QWidget *parent = nullptr);

        void lineNumberAreaPaintEvent(QPaintEvent *event);
        int lineNumberAreaWidth();

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private slots:
        void updateLineNumberAreaWidth(int newBlockCount);
        void highlightCurrentLine();
        void updateLineNumberArea(const QRect &rect, int dy);

    private:
        QWidget *lineNumberArea;
    };
}
#endif // TEXTEDIT_H
