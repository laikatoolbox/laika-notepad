#include "SettingsStore.h"

std::string LaikaSettings::SettingsStore::getSettingsFileLocation() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString settingsFile = QDir(path).filePath("laika-notepad.spk");

    return settingsFile.toStdString();
}