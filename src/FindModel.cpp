#include "FindModel.h"

int FindModel::rowCount(const QModelIndex &) const
{
    return this->findResults.count();
}

int FindModel::columnCount(const QModelIndex &) const
{
    return 3;
}

QVariant FindModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole && role != Qt::EditRole) return {};
    const auto & findResult = this->findResults[index.row()];

    switch (index.column()) {
    case 0:
        return findResult.linePosition;
    case 1:
        return findResult.startPosition;
    case 2:
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
        return "Line";
    case 1:
        return "Start";
    case 2:
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
    this->currentResultIndex = 0;
    this->findResults.clear();
    this->endResetModel();
}


void FindModel::incrementIndex()
{
    int maxIndex = std::max((int)this->findResults.count() - 1, 0);
    int newIndex = this->currentResultIndex + 1;

    if (newIndex > maxIndex)
    {
        if (this->wrapAround)
        {
            this->currentResultIndex = 0;
        }
        else
        {
            this->currentResultIndex = maxIndex;
        }
    }
    else
    {
        this->currentResultIndex = newIndex;
    }
}

void FindModel::decrementIndex()
{
    int maxIndex = std::max((int)this->findResults.count() - 1, 0);
    int newIndex = this->currentResultIndex - 1;

    if (newIndex < 0)
    {
        if (this->wrapAround)
        {
            this->currentResultIndex = maxIndex;
        }
        else
        {
            this->currentResultIndex = 0;
        }
    }
    else
    {
        this->currentResultIndex = newIndex;
    }
}

FindResult* FindModel::resultAt(int row)
{
    if (row >= 0 && this->findResults.length() > row)
    {
        return &(this->findResults[row]);
    }

    return nullptr;
}

FindResult* FindModel::currentResult()
{
    return this->resultAt(currentResultIndex);
}

int FindModel::count()
{
    return this->findResults.count();
}
