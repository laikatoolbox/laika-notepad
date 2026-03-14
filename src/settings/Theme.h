#ifndef THEME_H
#define THEME_H

#include <QColor>
#include <string>

namespace LaikaSettings {
    class Theme {
    public:
        std::string name = "";
        QColor backgroundColor = QColor();
        QColor textColor = QColor();
        QColor lineNumberBackgroundColor = QColor();
        QColor lineNumberTextColor = QColor();
        QColor currentLineNumberTextColor = QColor();
    };
}

#endif // THEME_H
