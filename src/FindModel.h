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

    int rowCount(const QModelIndex &) const override;
    int columnCount(const QModelIndex &) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    FindResult* resultAt(int row);
    void append(const FindResult &result);
    void invalidate();
};

#endif // FINDMODEL_H
