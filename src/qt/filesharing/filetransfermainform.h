#ifndef FILETRANSFERMAINFORM_H
#define FILETRANSFERMAINFORM_H

#include <QWidget>

namespace Ui {
class FileTransferMainForm;
}

class FileTransferMainForm : public QWidget
{
    Q_OBJECT

public:
    explicit FileTransferMainForm(QWidget *parent = 0);
    ~FileTransferMainForm();

    QString displaySpeed(quint64 bytes);

private:
    Ui::FileTransferMainForm *ui;
};

#endif // FILETRANSFERMAINFORM_H
