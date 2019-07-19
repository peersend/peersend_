#ifndef SERVERWIDGET_H
#define SERVERWIDGET_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include "constants.h"

namespace Ui {
class ServerWidget;
}

class ServerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ServerWidget(QWidget *parent = 0);
    ~ServerWidget();

    QString getName() const;
    QString getWallet() const;
    int getPort() const;

signals:
    void nameChanged(const QString&);
    void walletChanged(const QString&);
    void portChanged(int);

    void connectEstablished(QTcpSocket*);

private slots:
    void onNewConnection();
    void onPortChaned();

private:
    Ui::ServerWidget *ui;
    QTcpServer m_server;
};

#endif // SERVERWIDGET_H
