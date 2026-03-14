#ifndef SETTINGSSTORE_H
#define SETTINGSSTORE_H

#include <QStandardPaths>
#include <QDir>
#include "Theme.h"

namespace LaikaSettings {
    class SettingsStore {
    public:
        // defaults
        const static bool defaultShowStatusBar = true;
        const static bool defaultAutoRefreshTextStats = true;
        const static bool defaultShowLineNumbers = true;
        const static bool defaultWordWrap = true;
        const static bool defaultLockToolbars = true;

        // values
        bool showStatusBar = defaultShowStatusBar;
        bool autoRefreshTextStats = defaultAutoRefreshTextStats;
        bool showLineNumbers = defaultShowLineNumbers;
        bool wordWrap = defaultWordWrap;
        bool lockToolbars = defaultLockToolbars;
        LaikaSettings::Theme *currentTheme = nullptr;

        std::string getSettingsFileLocation();
    };
}

#endif // SETTINGSSTORE_H
