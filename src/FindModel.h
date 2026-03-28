#ifndef FINDMODEL_H
#define FINDMODEL_H

#include <QAbstractTableModel>

class FindResult
{
public:
    int startPosition;
    int endPosition;
};


class FindModel : public QAbstractTableModel {
    QList<FindResult> findResults;
public:
    FindModel(QObject * parent = {}) : QAbstractTableModel{parent} {}
    int currentResultIndex = 0;
    bool wrapAround = true;
    bool matchCase = false;
    bool wholeWord = false;

    int rowCount(const QModelIndex &) const override;
    int columnCount(const QModelIndex &) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    FindResult* resultAt(int row);
    FindResult* currentResult();
    void append(const FindResult &result);
    void incrementIndex();
    void decrementIndex();
    void invalidate();
    int count();
};

#endif // FINDMODEL_H
