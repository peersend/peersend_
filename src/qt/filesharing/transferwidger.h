#ifndef TRANSFERWIDGER_H
#define TRANSFERWIDGER_H

#include <QWidget>
#include <QThread>
#include "TcpSocket.h"
#include "constants.h"

namespace Ui {
class TransferWidger;
}

class TransferWidger : public QWidget
{
    Q_OBJECT

public:
    explicit TransferWidger(QWidget *parent = 0, QTcpSocket * socket = 0);
    ~TransferWidger();

public slots:
    void terminateTranswer();

private:
    QString displaySpeed(quint64 bytes);

private slots:
    void on_tbTransmitSelectFiles_clicked();
    void on_pbSendMessage_clicked();
    void onReceivedMessage(const QString);
    void onTransmittingProgress(qint64 current, qint64 total);
    void onReceivingProgress(qint64 current, qint64 total);
    void onTotalReceivingProgress(qint64 current, qint64 total);
    void onTotalTransmittingProgress(qint64 current, qint64 total);
    void onTransmittSpeed(const int);
    void onReceiveSpeed(const int);
    void onLog(QString);

signals:
    void clientName(const QString&);
    void sendFile(const QString&);
    void sendMessage(const QString&);
    void closeSocket();

private:
    Ui::TransferWidger *ui;
    TcpSocket * m_socket;
    QThread *m_thread;

};

#endif // TRANSFERWIDGER_H
