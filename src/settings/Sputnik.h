#include <string>
#include <map>
#include <vector>
#include <fstream>

#ifndef SPUTNIK_H
#define SPUTNIK_H

namespace Sputnik
{
/// @brief A set of values and objects related to each other.
/// think of it like a "namespace" in C++, which I was originally
/// going to call it but decided on "sector" to avoid conflicts
/// with the reserved keyword "namespace".
class Sector
{
public:
    Sector();
    Sector(const Sector &sector);
    ~Sector();

    std::map<std::string, std::map<std::string, std::string>> sections;
    std::multimap<std::string, std::map<std::string, std::string>> objects;

    std::map<std::string, std::string>& getSection(std::string sectionName);
    std::vector<std::map<std::string, std::string>> getObjects(std::string objectName);
};

struct ParseStatus
{
    // flag if the parse was a success
    bool success = true;
    // if not a success, not the line and error message
    // line number of -1 means the error wasn't line specific
    int lineNumber = -1;
    // the error message
    std::string errorMessage = "";
};

class File
{
private:
    void clear();
    std::map<std::string, std::string> *rootOfRoot();

public:
    File();
    ~File();

    std::map<std::string, Sector> sectors = {};

    ParseStatus parseText(std::string text);
    ParseStatus parseFile(std::string fileName);

    Sputnik::Sector root();
    Sputnik::Sector &sector(std::string key);

    std::string getValue(std::string key, std::string section, std::string sector);
    std::vector<std::string_view> getValueAsArray(std::string key, std::string section, std::string sector);

    std::vector<std::map<std::string, std::string>> getObjects(std::string objectName, std::string section, std::string sector);
};

enum class LineState
{
    AtRoot,
    InSection,
    InObject
};

std::vector<std::string_view> splitString(std::string_view text, char delimeter);
void replaceAll(std::string &subject, const std::string &search, const std::string &replace);
void sanitize(std::string &text);
void desanitize(std::string &text);
}

// String ops
std::vector<std::string_view> Sputnik::splitString(std::string_view text, char delimeter)
{
    std::vector<std::string_view> result;
    auto left = text.begin();

    for (auto it = left; it != text.end(); ++it)
    {
        if (*it == delimeter)
        {
            result.emplace_back(&*left, it - left);
            left = it + 1;
        }
    }

    if (left != text.end())
    {
        result.emplace_back(&*left, text.end() - left);
    }

    return result;
}

void Sputnik::replaceAll(std::string &subject, const std::string &search, const std::string &replace)
{
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos)
    {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}

void Sputnik::sanitize(std::string &text)
{
    Sputnik::replaceAll(text, "$", "$dl");
    Sputnik::replaceAll(text, ":", "$cl");
    Sputnik::replaceAll(text, "=", "$eq");
    Sputnik::replaceAll(text, ";", "$sc");
    Sputnik::replaceAll(text, "\r", "$r");
    Sputnik::replaceAll(text, "\n", "$n");
    Sputnik::replaceAll(text, "@", "$at");
    Sputnik::replaceAll(text, ">", "$gt");
    Sputnik::replaceAll(text, "<", "$lt");
}

void Sputnik::desanitize(std::string &text)
{
    Sputnik::replaceAll(text, "$cl", ":");
    Sputnik::replaceAll(text, "$eq", "=");
    Sputnik::replaceAll(text, "$sc", ";");
    Sputnik::replaceAll(text, "$r", "\r");
    Sputnik::replaceAll(text, "$n", "\n");
    Sputnik::replaceAll(text, "$at", "@");
    Sputnik::replaceAll(text, "$gt", ">");
    Sputnik::replaceAll(text, "$lt", "<");
    Sputnik::replaceAll(text, "$dl", "$");
}

// SECTOR
Sputnik::Sector::Sector()
{
    this->sections.insert({"root", {}});
}

Sputnik::Sector::Sector(const Sector &sector)
{
    this->sections = sector.sections;
    this->objects = sector.objects;
}

Sputnik::Sector::~Sector()
{
}

std::map<std::string, std::string> &Sputnik::Sector::getSection(std::string sectionName)
{
    return this->sections[sectionName];
}

std::vector<std::map<std::string, std::string>> Sputnik::Sector::getObjects(std::string objectName)
{
    auto range = this->objects.equal_range(objectName);
    std::vector<std::map<std::string, std::string>> objects = {};
    for (auto i = range.first; i != range.second; ++i)
    {
        objects.emplace_back(i->second);
    }

    return objects;
}

// FILE
Sputnik::File::File()
{
}

Sputnik::File::~File()
{
}

void Sputnik::File::clear()
{
    // Clear the sectors
    this->sectors.clear();
    // this->sectors.insert({"root", Sputnik::Sector()});
}

std::map<std::string, std::string> *Sputnik::File::rootOfRoot()
{
    return &(sectors["root"].sections["root"]);
}

Sputnik::ParseStatus Sputnik::File::parseText(std::string text)
{
    Sputnik::ParseStatus status;
    return status;
}

Sputnik::ParseStatus Sputnik::File::parseFile(std::string fileName)
{
    Sputnik::ParseStatus status;
    std::ifstream file(fileName);

    if (file.is_open())
    {
        // File opened, so clear (reset all values) this Sputnik file
        this->clear();

        // Loop by line
        std::string line;
        // Variables to keep track of state that spans multiple lines
        Sputnik::LineState lineState = Sputnik::LineState::AtRoot;
        std::map<std::string, std::string> *currentSectionOrObject = this->rootOfRoot();

        while (std::getline(file, line))
        {
            bool sectionStart = line.starts_with(':');
            bool objectStart = line.starts_with('@');
            if (sectionStart || objectStart)
            { // sections and objects share a lot of logic
                bool kickBackToRoot = false;
                std::string sectionOrObjectName(line.begin() + 1, line.end());

                // An empty section or object definition means we're being
                // kicked back to root
                if (sectionOrObjectName.length() == 0)
                {
                    kickBackToRoot = true;
                    sectionOrObjectName = "root";
                }

                // Get the sector based on the section or object's definition
                // If it has a '>', the text to the left is the sector's name.
                // If it doesn't, the sector is the default "root" sector.
                std::vector<std::string_view> sectorSplit = Sputnik::splitString(sectionOrObjectName, '>');
                std::string sectorName = "root";
                if (sectorSplit.size() > 1)
                {
                    sectorName = std::string(sectorSplit[0]);
                    sectionOrObjectName = std::string(sectorSplit[1]);
                }

                // desanitize the names
                Sputnik::desanitize(sectorName);
                Sputnik::desanitize(sectionOrObjectName);

                Sputnik::Sector *sector = &(this->sectors[sectorName]);

                // Logic that differs between sections and objects
                if (sectionStart)
                { // section specific logic
                    lineState = Sputnik::LineState::InSection;

                    // Only create a new section if the section doesn't already exist
                    if (!sector->sections.contains(sectionOrObjectName))
                    {
                        std::map<std::string, std::string> object = {};
                        sector->sections.emplace(sectionOrObjectName, object);
                    }

                    currentSectionOrObject = &(sector->sections[sectionOrObjectName]);
                }
                else
                { // object specific logic
                    lineState = Sputnik::LineState::InObject;

                    // create a new object and append it
                    auto object = sector->objects.insert({sectionOrObjectName, {}});
                    currentSectionOrObject = &object->second;
                }

                if (kickBackToRoot)
                {
                    lineState = Sputnik::LineState::AtRoot;
                    currentSectionOrObject = this->rootOfRoot();
                }
            }
            else if (line.starts_with(';'))
            { // comment
                // do nothing
            }
            else if (line.find("=") != std::string::npos)
            { // key=value line
                // get the key and value
                std::vector<std::string_view> kvSplit = Sputnik::splitString(line, '=');
                std::string key = std::string(kvSplit[0]);
                std::string value = std::string(kvSplit[1]);

                // desanitize them
                Sputnik::desanitize(key);
                Sputnik::desanitize(value);

                // assign key=value
                (*currentSectionOrObject)[key] = value;
            }
        }

        file.close();
    }

    return status;
}

Sputnik::Sector &Sputnik::File::sector(std::string key)
{
    return this->sectors[key];
}

std::string Sputnik::File::getValue(std::string key, std::string section = "root", std::string sector = "root")
{
    Sputnik::Sector &foundSector = this->sector(sector);
    std::map<std::string, std::string> &foundSection = foundSector.getSection(section);
    return foundSection[key];
}

std::vector<std::string_view> Sputnik::File::getValueAsArray(std::string key, std::string section = "root", std::string sector = "root")
{
    std::string value = this->getValue(sector, key);
    return Sputnik::splitString(value, ';');
}

std::vector<std::map<std::string, std::string>> Sputnik::File::getObjects(std::string objectName, std::string section = "root", std::string sector = "root")
{
    Sputnik::Sector &foundSector = this->sector(sector);
    return foundSector.getObjects(objectName);
}
#endif