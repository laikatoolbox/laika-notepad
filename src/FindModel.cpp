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
        return "Line #";
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
    this->currentResultIndices = {};
    this->findResults.clear();
    this->endResetModel();
}

void FindModel::selectAll()
{
    this->currentResultIndices.clear();
    for (int i=0; i<this->findResults.size(); i++)
    {
        this->currentResultIndices.push_back(i);
    }
}

int FindModel::firstSelectedIndex()
{
    if (this->currentResultIndices.size() > 0)
    {
        return this->currentResultIndices[0];
    }

    return -1;
}

void FindModel::incrementIndex()
{
    int curIndex = this->firstSelectedIndex();

    if (curIndex != -1)
    {
        int maxIndex = std::max((int)this->findResults.count() - 1, 0);
        int newIndex = curIndex + 1;

        if (newIndex > maxIndex)
        {
            if (this->wrapAround)
            {
                this->currentResultIndices = {0};
            }
            else
            {
                this->currentResultIndices = {maxIndex};
            }
        }
        else
        {
            this->currentResultIndices = {newIndex};
        }
    }
}

void FindModel::decrementIndex()
{
    int curIndex = this->firstSelectedIndex();

    if (curIndex != -1)
    {
        int maxIndex = std::max((int)this->findResults.count() - 1, 0);
        int newIndex = curIndex - 1;

        if (newIndex < 0)
        {
            if (this->wrapAround)
            {
                this->currentResultIndices = {maxIndex};
            }
            else
            {
                this->currentResultIndices = {0};
            }
        }
        else
        {
            this->currentResultIndices = {newIndex};
        }
    }
}

FindResult* FindModel::resultAt(int row)
{
    int findResultCount = this->findResults.length();
    if (row >= 0 && findResultCount > 0 && findResultCount > row)
    {
        return &(this->findResults[row]);
    }

    return nullptr;
}

std::vector<FindResult*> FindModel::currentResults()
{
    std::vector<FindResult*> results = {};

    for (const int& i : this->currentResultIndices)
    {
        FindResult *currentResult = this->resultAt(i);

        if (currentResult != nullptr)
        {
            results.push_back(currentResult);
        }
    }

    return results;
}

int FindModel::count()
{
    return this->findResults.count();
}
