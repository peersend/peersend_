#include "transferwidger.h"
#include "ui_transferwidger.h"

#include <QFileDialog>
#include "constants.h"

TransferWidger::TransferWidger(QWidget *parent, QTcpSocket *socket)
  : QWidget(parent)
  , ui(new Ui::TransferWidger)
  //, m_socket(socket)
  , m_thread(NULL)
{
    ui->setupUi(this);
    m_socket = new TcpSocket(socket);
    connect(m_socket, SIGNAL(receivedMessage(const QString)), this, SLOT(onReceivedMessage(const QString)), Qt::QueuedConnection);
    connect(m_socket, SIGNAL(clientName(const QString&)), this, SIGNAL(clientName(const QString&)), Qt::QueuedConnection);
    m_thread = new QThread(this);

    m_socket->setDownloadingFolder("~"); //global var defined in main and constants

    m_socket->sendMessage(QString("%1(%2)").arg(ClientNAME).arg(ClientWALLET));
    connect(this, SIGNAL(closeSocket()), m_socket, SLOT(closeSocket()), Qt::QueuedConnection);
    connect(m_socket, SIGNAL(socketDiskonnected()), this, SLOT(deleteLater()), Qt::QueuedConnection);
    connect(this, SIGNAL(sendFile(QString)), m_socket, SLOT(sendFile(QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(sendMessage(QString)), m_socket, SLOT(sendMessage(QString)), Qt::QueuedConnection);
    connect(m_socket, SIGNAL(transmittingProgress(qint64,qint64)), this, SLOT(onTransmittingProgress(qint64,qint64)));
    connect(m_socket, SIGNAL(receivingProgress(qint64,qint64)), this, SLOT(onReceivingProgress(qint64,qint64)));

    connect(m_socket, SIGNAL(totalTransmittingProgress(qint64, qint64)), this, SLOT(onTotalTransmittingProgress(qint64, qint64)));
    connect(m_socket, SIGNAL(totalReceivingProgress(qint64, qint64)), this, SLOT(onTotalReceivingProgress(qint64, qint64)));


    connect(m_socket, SIGNAL(uploadSpeed(int)), this, SLOT(onTransmittSpeed(int)));
    connect(m_socket, SIGNAL(downloadSpeed(int)), this, SLOT(onReceiveSpeed(int)));

    connect(m_socket, SIGNAL(log(QString)), this, SLOT(onLog(QString)));

    m_socket->setParent(NULL);
    m_socket->moveToThread(m_thread);
    socket->moveToThread(m_thread);
    m_thread->start();
}

TransferWidger::~TransferWidger()
{
    m_thread->quit();
    m_thread->wait();
    delete ui;
}

QString TransferWidger::displaySpeed(quint64 bytes)
{
    static const quint64 kb = 1024;
    static const quint64 mb = 1024 * kb;
    static const quint64 gb = 1024 * mb;
    static const quint64 tb = 1024 * gb;
    if (bytes >= tb)
        return tr("%1 TB").arg(QLocale().toString(qreal(bytes) / tb, 'f', 3));
    if (bytes >= gb)
        return tr("%1 GB").arg(QLocale().toString(qreal(bytes) / gb, 'f', 2));
    if (bytes >= mb)
        return tr("%1 MB").arg(QLocale().toString(qreal(bytes) / mb, 'f', 1));
    if (bytes >= kb)
        return tr("%1 KB").arg(QLocale().toString(bytes / kb));
    return tr("%1 byte(s)").arg(QLocale().toString(bytes));
}

void TransferWidger::terminateTranswer()
{
    emit closeSocket();
    this->deleteLater();
}

void TransferWidger::on_tbTransmitSelectFiles_clicked()
{
    QStringList files = QFileDialog::getOpenFileNames(
                this,
                tr("Select one or more files to send"),
                "",
                "All (*.*)");

    foreach (const QString& aName, files) {
        emit sendFile(aName);
    }

}

void TransferWidger::on_pbSendMessage_clicked()
{
    if(m_socket) {
        QString msg = ui->leChat->text();
        emit sendMessage(msg);
        QString htmlMsg = QString("<font color=\"blue\">%1: %2</font><br>")
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                .arg(msg);
        ui->teChat->insertHtml(htmlMsg);
        ui->leChat->clear();
    }
}

void TransferWidger::onReceivedMessage(const QString msg)
{
    QString htmlMsg = QString("<font color=\"red\">%1: %2</font><br>")
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
            .arg(msg);
    ui->teChat->insertHtml(htmlMsg);
}

void TransferWidger::onTransmittingProgress(qint64 current, qint64 total)
{
    current /= 1000;
    total /= 1000;
    if(ui->pbTransmitCurrent->maximum() != total)
        ui->pbTransmitCurrent->setMaximum(total);
    ui->pbTransmitCurrent->setValue(current);
}

void TransferWidger::onReceivingProgress(qint64 current, qint64 total)
{
    current /= 1000;
    total /= 1000;

    if(ui->pbReceiveCurrent->maximum() != total)
        ui->pbReceiveCurrent->setMaximum(total);
    ui->pbReceiveCurrent->setValue(current);
}

void TransferWidger::onTotalReceivingProgress(qint64 current, qint64 total)
{
    if(ui->pbReceiveTotal->maximum() != total)
        ui->pbReceiveTotal->setMaximum(total);
    ui->pbReceiveTotal->setValue(current);
}

void TransferWidger::onTotalTransmittingProgress(qint64 current, qint64 total)
{
    if(ui->pbTransmitTotal->maximum() != total)
        ui->pbTransmitTotal->setMaximum(total);
    ui->pbTransmitTotal->setValue(current);
}

void TransferWidger::onTransmittSpeed(const int speed)
{
    QString msg = QString("%1/sec").arg(displaySpeed(speed));
    ui->lblTransmitSpeed->setText(msg);
}

void TransferWidger::onReceiveSpeed(const int speed)
{
    QString msg = QString("%1/sec").arg(displaySpeed(speed));
    ui->lblReceiveSpeed->setText(msg);
}

void TransferWidger::onLog(QString msg)
{
    ui->teLog->append(msg);
}
