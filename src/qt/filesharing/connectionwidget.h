#ifndef CONNECTIONWIDGET_H
#define CONNECTIONWIDGET_H

#include <QWidget>
#include <QHostAddress>
#include <QItemSelection>
#include <QTcpSocket>

class QUdpSocket;
class SearchConnectionsModel;

namespace Ui {
class ConnectionWidget;
}

class ConnectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionWidget(QWidget *parent = 0);
    ~ConnectionWidget();

public slots:
    void onPortChanged(int);
    void onNameChanged(const QString&);
    void onWalletChanged(const QString&);

private slots:
    void onConnectBtnPressed();
    void onSearchButtonPressed();
    void readPendingDatagrams();
    void processTheDatagram(QByteArray&, QHostAddress&);
    void onClicked(const QModelIndex&);

signals:
    void connectEstablished(QTcpSocket*);

private:
    SearchConnectionsModel * m_model;
    QUdpSocket * m_broadcastSocket;

    Ui::ConnectionWidget *ui;

    quint16 m_port;
    QString m_name;
    QString m_wallet;
    QList<QHostAddress> m_adresses;
};

#endif // CONNECTIONWIDGET_H
