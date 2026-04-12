#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QPlainTextEdit>
#include <settings/SettingsStore.h>

namespace LaikaNotepad {
    class TextEdit : public QPlainTextEdit {
        Q_OBJECT

    public:
        TextEdit(QWidget *parent = nullptr);
        bool shouldCalculateBlocks = false;
        void lineNumberAreaPaintEvent(QPaintEvent *event);
        int lineNumberAreaWidth();
        int getDefaultFontSize();        

        // These two are the characters at the start and
        // end of the blocks visible in the viewport.
        // NOTE: The are calculated when doing line numbers and
        // will return -1 if both showLineNumbers and
        // shouldCalculateBlocks are false
        int startOfFirstBlock();
        int endOfLastBlock();

        // user settings
        void setSettings(LaikaSettings::SettingsStore *settings);
        bool showLineNumbers = true;
        QBrush lineNumberBackgroundColorBrush = QBrush();
        QPen currentLineNumberTextColorPen = QPen();
        QPen lineNumberTextColorPen = QPen();

    protected:
        void resizeEvent(QResizeEvent *event) override;
        void keyPressEvent(QKeyEvent *event) override;

    private slots:
        void updateLineNumberAreaWidth(int newBlockCount);
        void highlightCurrentLine();
        void updateLineNumberArea(const QRect &rect, int dy);

    private:
        QWidget *lineNumberArea;
        int defaultFontSize = 0;
        int lineNumberPaddding = 3;

        // calculated block stats
        int startChar = -1;
        int endChar = -1;

        static inline int numDigits(const int input) {
            return 1 + (input >= 1000000000) + (input >= 100000000) + (input >= 10000000) +
                   (input >= 1000000) + (input >= 100000) + (input >= 10000) +
                   (input >= 1000) + (input >= 100) + (input >= 10);
        }
    };
}
#endif // TEXTEDIT_H
