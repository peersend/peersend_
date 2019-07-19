#include "connectionwidget.h"
#include "ui_connectionwidget.h"
#include "searchconnectionsmodel.h"
#include "constants.h"
#include <QtNetwork> //mod
#include <QUdpSocket>
#include <QNetworkInterface>
#include <QMessageBox>
#include <QTcpSocket>

QString ClientNAME;
QString ClientWALLET;

ConnectionWidget::ConnectionWidget(QWidget *parent)
    : QWidget(parent)
    , m_model(new SearchConnectionsModel(this))
    , m_broadcastSocket(new QUdpSocket(this))
    , ui(new Ui::ConnectionWidget)

{
    ui->setupUi(this);

    ui->lvItems->setModel(m_model);
    m_broadcastSocket->bind(/*QHostAddress::Any, */DiscoveryPort/*, QAbstractSocket::ReuseAddressHint*/);

    connect(m_broadcastSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
    connect(ui->pbConnect, SIGNAL(clicked(bool)), this, SLOT(onConnectBtnPressed()));
    connect(ui->tbSearch,  SIGNAL(clicked(bool)), this, SLOT(onSearchButtonPressed()));

    connect(ui->lvItems, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(onClicked(const QModelIndex&)));

    m_adresses = QNetworkInterface::allAddresses();
}

ConnectionWidget::~ConnectionWidget()
{
    delete ui;
}

void ConnectionWidget::onPortChanged(int newPort)
{
    if(newPort > 0 && newPort < 0xFFFF)
        m_port = newPort;
}

void ConnectionWidget::onNameChanged(const QString & newName)
{
    m_name = newName;
    ClientNAME = newName;
}

void ConnectionWidget::onWalletChanged(const QString & newWallet)
{
    m_wallet = newWallet;
    ClientWALLET = newWallet;
}

void ConnectionWidget::onConnectBtnPressed()
{
    QTcpSocket *socket = new QTcpSocket(NULL);
    QString host = ui->lineEdit->text();
    quint16 port = ui->spinBox->value();
    socket->connectToHost(host, port);
    if(socket->waitForConnected(ConnectionTimeout)) {
        emit connectEstablished(socket);
    }
    else {
        QMessageBox::critical(this, tr("Connection Error"),
                              QString("Can't connect to the %1:%2").arg(host).arg(port));
    }
}

void ConnectionWidget::onSearchButtonPressed()
{
    m_model->removeRows(0, m_model->rowCount());
    m_broadcastSocket->writeDatagram(DiscoveryWorld.toUtf8(), QHostAddress::Broadcast, DiscoveryPort);
}

void ConnectionWidget::readPendingDatagrams()
{
    while (m_broadcastSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_broadcastSocket->pendingDatagramSize());
        QHostAddress senderHost;
        quint16 senderPort;

        m_broadcastSocket->readDatagram(datagram.data(), datagram.size(),
                                &senderHost, &senderPort);

        qDebug() << "->" << senderHost << senderPort << datagram;
        if(m_adresses.contains(senderHost))
            continue;

        processTheDatagram(datagram, senderHost);
    }
}

void ConnectionWidget::processTheDatagram(QByteArray &datagram, QHostAddress &senderHost)
{
    QString data = QString::fromUtf8(datagram);
    if(data == DiscoveryWorld) {
        QString answer = QString("%1\t%2\t%3")
                .arg(m_name)
                .arg(m_port)
                .arg(m_wallet);
        QUdpSocket sock;
        int writed = sock.writeDatagram(answer.toUtf8(), senderHost, DiscoveryPort);
        qDebug() << "<-" << senderHost << DiscoveryPort << "'" << answer << "'" << writed;
        //sock.waitForBytesWritten();
    }
    else {
        QStringList answer = data.split("\t", QString::KeepEmptyParts);
        if(answer.size() != 3)
            return;
        QList<QStandardItem*> row = QList<QStandardItem*>() << new QStandardItem(answer[0])
                                                          << new QStandardItem(senderHost.toString())
                                                          << new QStandardItem(answer[1])
                                                          << new QStandardItem(answer[2]);
        m_model->appendRow(row);
    }
}

void ConnectionWidget::onClicked(const QModelIndex& index)
{
    const QAbstractItemModel *model = index.model();
    const QModelIndex parent = index.parent();
    ui->lineEdit->setText(model->data(model->index(index.row(),1,parent)).toString());
    ui->spinBox->setValue(model->data(model->index(index.row(),2,parent)).toUInt());
}
