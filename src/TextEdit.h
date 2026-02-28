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
        int getDefaultFontSize();
        void setShowLineNumbers(bool value);
        bool getShowLineNumbers();

    protected:
        void resizeEvent(QResizeEvent *event) override;
        void keyPressEvent(QKeyEvent *event) override;

    private slots:
        void updateLineNumberAreaWidth(int newBlockCount);
        void highlightCurrentLine();
        void updateLineNumberArea(const QRect &rect, int dy);

    private:
        QWidget *lineNumberArea;        
        int lineNumberPaddding = 3;
        int defaultFontSize = 0;
        bool showLineNumbers = true;

        static inline int numDigits(const int input) {
            return 1 + (input >= 1000000000) + (input >= 100000000) + (input >= 10000000) +
                   (input >= 1000000) + (input >= 100000) + (input >= 10000) +
                   (input >= 1000) + (input >= 100) + (input >= 10);
        }
    };
}
#endif // TEXTEDIT_H
