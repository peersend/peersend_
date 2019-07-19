#include "filetransfermainform.h"
#include "ui_filetransfermainform.h"

FileTransferMainForm::FileTransferMainForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileTransferMainForm)
{
    ui->setupUi(this);
}

FileTransferMainForm::~FileTransferMainForm()
{
    delete ui;
}

QString FileTransferMainForm::displaySpeed(quint64 bytes)
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
