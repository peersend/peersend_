#ifndef TRANSFERMAINWIDGET_H
#define TRANSFERMAINWIDGET_H

#include <QWidget>
#include <QTcpSocket>

#include "walletmodel.h"

namespace Ui {
class FileSharingDialog;
}

class FileSharingDialog : public QWidget
{
    Q_OBJECT

public:
    explicit FileSharingDialog(QWidget *parent = 0);
    ~FileSharingDialog();

    void setModel(WalletModel* model);

private slots:
    void onConnectEstablished(QTcpSocket*);
    void onTabCloseRequested(int);
    void onTabDestroyed(int);
    void onClientName(const QString&);

private:
    Ui::FileSharingDialog *ui;
    WalletModel* walletModel;
};

#endif // TRANSFERMAINWIDGET_H
