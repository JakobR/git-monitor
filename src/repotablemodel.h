#ifndef REPOTABLEMODEL_H
#define REPOTABLEMODEL_H

#include <QAbstractTableModel>

class RepoTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit RepoTableModel(QObject *parent = nullptr);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
};

#endif // REPOTABLEMODEL_H
