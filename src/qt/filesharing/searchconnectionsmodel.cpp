#include "searchconnectionsmodel.h"

SearchConnectionsModel::SearchConnectionsModel(QObject *parent)
    : QStandardItemModel(parent)
{
    setHorizontalHeaderItem(0, new QStandardItem(tr("User name")));
    setHorizontalHeaderItem(1, new QStandardItem(tr("Host")));
    setHorizontalHeaderItem(2, new QStandardItem(tr("Port")));
    setHorizontalHeaderItem(3, new QStandardItem(tr("Wallet address")));
}
