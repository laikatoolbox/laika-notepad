#include "FindModel.h"

int FindModel::rowCount(const QModelIndex &) const
{
    return this->findResults.count();
}

int FindModel::columnCount(const QModelIndex &) const
{
    return 2;
}

QVariant FindModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole && role != Qt::EditRole) return {};
    const auto & findResult = this->findResults[index.row()];

    switch (index.column()) {
    case 0:
        return findResult.startPosition;
    case 1:
        return findResult.endPosition;
    default:
        return {};
    };
}

QVariant FindModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};

    switch (section)
    {
    case 0:
        return "Start";
    case 1:
        return "End";
    default:
        return {};
    }
}

void FindModel::append(const FindResult &result)
{
    this->beginInsertRows({}, findResults.count(), findResults.count());
    this->findResults.append(result);
    this->endInsertRows();
}

void FindModel::invalidate()
{
    this->beginResetModel();
    this->findResults.clear();
    this->endResetModel();
}

FindResult* FindModel::resultAt(int row)
{
    if (this->findResults.length() > row)
    {
        return &(this->findResults[row]);
    }

    return nullptr;
}
