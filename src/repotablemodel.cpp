#include "repotablemodel.h"
#include <QSize>

namespace {
    namespace Column {
        enum Column : int {
            Path = 0,
            Status = 1,
            COUNT = 2,
        };
    }
}

RepoTableModel::RepoTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{}

QVariant RepoTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        switch (section) {
        case Column::Path:
            return tr("Repository Path");
        case Column::Status:
            return tr("Status");
        }
    }

    return QVariant();
}

int RepoTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    // FIXME: Implement me!
    return 0;
}

int RepoTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return Column::COUNT;
}

QVariant RepoTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // FIXME: Implement me!
    return QVariant();
}
