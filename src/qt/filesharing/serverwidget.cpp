#include "serverwidget.h"
#include "ui_serverwidget.h"

ServerWidget::ServerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ServerWidget)
{
    ui->setupUi(this);

    connect(ui->leUserName, SIGNAL(textEdited(const QString&)), this, SIGNAL(nameChanged(const QString&)));
    connect(ui->leWalletAddress, SIGNAL(textEdited(const QString&)), this, SIGNAL(walletChanged(const QString&)));
    connect(ui->sbPort, SIGNAL(valueChanged(int)), this, SIGNAL(portChanged(int)));
    connect(ui->sbPort, SIGNAL(valueChanged(int)), this, SIGNAL(portChanged(int)));

    connect(&m_server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    m_server.listen(QHostAddress::Any, ui->sbPort->value());
}

ServerWidget::~ServerWidget()
{
    delete ui;
}

QString ServerWidget::getName() const
{
    return ui->leUserName->text();
}

QString ServerWidget::getWallet() const
{
    return ui->leWalletAddress->text();
}

int ServerWidget::getPort() const
{
    return ui->sbPort->value();
}

void ServerWidget::onNewConnection()
{
    QTcpSocket *sock;
    while(m_server.hasPendingConnections()) {
        sock = m_server.nextPendingConnection();
        if(sock && sock->peerAddress() != QHostAddress())
            emit connectEstablished(sock);
    }
}

void ServerWidget::onPortChaned()
{
    m_server.close();
    m_server.listen(QHostAddress::Any, ui->sbPort->value());
}
