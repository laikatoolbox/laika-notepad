#ifndef FINDMODEL_H
#define FINDMODEL_H

#include <QAbstractTableModel>
#include <vector>

class FindResult
{
public:
    int resultPosition;
    int linePosition;
    int startPosition;
    int endPosition;
};

class FindModel : public QAbstractTableModel {
    QList<FindResult> findResults;
public:
    FindModel(QObject * parent = {}) : QAbstractTableModel{parent} {}
    std::vector<int> currentResultIndices = {};
    bool wrapAround = true;
    bool matchCase = false;
    bool wholeWord = false;

    int rowCount(const QModelIndex &) const override;
    int columnCount(const QModelIndex &) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    FindResult* resultAt(int row);
    std::vector<FindResult*> currentResults();
    void append(const FindResult &result);
    int firstSelectedIndex();
    void incrementIndex();
    void decrementIndex();
    void invalidate();
    void selectAll();
    int count();
};

#endif // FINDMODEL_H
