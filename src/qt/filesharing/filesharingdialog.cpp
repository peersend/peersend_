#include "filesharingdialog.h"
#include "ui_filesharingdialog.h"

#include <QUdpSocket>
#include "TcpSocket.h"
#include "transferwidger.h"

FileSharingDialog::FileSharingDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileSharingDialog)
{
    ui->setupUi(this);
    ui->tabBar->addTab(tr("Connection"));
    ui->tabBar->addTab(tr("Server"));
    ui->tabBar->setTabsClosable(true);

    ui->tabBar->setTabButton(0, QTabBar::RightSide, 0);
    ui->tabBar->setTabButton(0, QTabBar::LeftSide, 0);

    ui->tabBar->setTabButton(1, QTabBar::RightSide, 0);
    ui->tabBar->setTabButton(1, QTabBar::LeftSide, 0);


    connect (ui->tabBar, SIGNAL(currentChanged(int)), ui->stackedWidget, SLOT(setCurrentIndex(int)));

    connect(ui->serverWidget, SIGNAL(nameChanged(QString)), ui->connectionWidget, SLOT(onNameChanged(QString)));
    connect(ui->serverWidget, SIGNAL(portChanged(int)), ui->connectionWidget, SLOT(onPortChanged(int)));
    connect(ui->serverWidget, SIGNAL(walletChanged(QString)), ui->connectionWidget, SLOT(onWalletChanged(QString)));
    connect(ui->connectionWidget, SIGNAL(connectEstablished(QTcpSocket*)), this, SLOT(onConnectEstablished(QTcpSocket*)));
    connect(ui->serverWidget, SIGNAL(connectEstablished(QTcpSocket*)), this, SLOT(onConnectEstablished(QTcpSocket*)));

    connect(ui->tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(onTabCloseRequested(int)));
    connect(ui->stackedWidget, SIGNAL(widgetRemoved(int)), this, SLOT(onTabDestroyed(int)));
    // fill defauelt values

    ui->connectionWidget->onNameChanged(ui->serverWidget->getName());
    ui->connectionWidget->onWalletChanged(ui->serverWidget->getWallet());
    ui->connectionWidget->onPortChanged(ui->serverWidget->getPort());
}

FileSharingDialog::~FileSharingDialog()
{
    for(int i=ui->stackedWidget->count()-1; i>1;--i)
        delete ui->stackedWidget->widget(i);
    delete ui;
}

void FileSharingDialog::setModel(WalletModel* walletModel)
{
    this->walletModel = walletModel;
}

void FileSharingDialog::onConnectEstablished(QTcpSocket *socket)
{
    static int cnt = 1;
    TransferWidger* newTab = new TransferWidger(NULL, socket);
    connect(newTab, SIGNAL(clientName(QString)),
            this, SLOT(onClientName(QString)));
    ui->tabBar->addTab(QString("NewTab%1").arg(cnt++));
    ui->stackedWidget->addWidget(newTab);
    ui->tabBar->setCurrentIndex(ui->tabBar->count()-1);
}

void FileSharingDialog::onTabCloseRequested(int index)
{
    TransferWidger * widget = qobject_cast<TransferWidger*>(ui->stackedWidget->widget(index));
    ui->stackedWidget->removeWidget(widget);
    //widget->disconnect(0,0,0,0);
    //widget->deleteLater();
    widget->terminateTranswer();
    //ui->tabBar->removeTab(index);
}

void FileSharingDialog::onTabDestroyed(int index)
{
    ui->tabBar->removeTab(index);
}

void FileSharingDialog::onClientName(const QString &name)
{

    TransferWidger* widget = qobject_cast<TransferWidger*>(sender());
    if(widget) {
        int cnt = ui->stackedWidget->count();
        for(int i=0; i<cnt; ++i) {
            if(ui->stackedWidget->widget(i) == widget) {
                ui->tabBar->setTabText(i, name);
                return;
            }
        }
    }
}
