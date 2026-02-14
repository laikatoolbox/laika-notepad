#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H

#include "TextEdit.h"

namespace LaikaNotepad {
    class LineNumberArea : public QWidget
    {
    public:
        LineNumberArea(TextEdit *edit) : QWidget(edit), textEdit(edit)
        {}

        QSize sizeHint() const override
        {
            return QSize(textEdit->lineNumberAreaWidth(), 0);
        }

    protected:
        void paintEvent(QPaintEvent *event) override
        {
            textEdit->lineNumberAreaPaintEvent(event);
        }

    private:
        TextEdit *textEdit;
    };
}

#endif // LINENUMBERAREA_H
