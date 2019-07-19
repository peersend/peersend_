#ifndef SEARCHCONNECTIONSMODEL_H
#define SEARCHCONNECTIONSMODEL_H

#include <QStandardItemModel>

class SearchConnectionsModel : public QStandardItemModel
{
    Q_OBJECT

public:
    SearchConnectionsModel(QObject * parent);
};

#endif // SEARCHCONNECTIONSMODEL_H
