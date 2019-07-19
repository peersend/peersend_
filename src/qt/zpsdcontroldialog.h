// Copyright (c) 2017-2019 The peersend developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef zpsdCONTROLDIALOG_H
#define zpsdCONTROLDIALOG_H

#include <QDialog>
#include <QTreeWidgetItem>
#include "zpsd/zerocoin.h"
#include "privacydialog.h"

class CZerocoinMint;
class WalletModel;

namespace Ui {
class zpsdControlDialog;
}

class CzpsdControlWidgetItem : public QTreeWidgetItem
{
public:
    explicit CzpsdControlWidgetItem(QTreeWidget *parent, int type = Type) : QTreeWidgetItem(parent, type) {}
    explicit CzpsdControlWidgetItem(int type = Type) : QTreeWidgetItem(type) {}
    explicit CzpsdControlWidgetItem(QTreeWidgetItem *parent, int type = Type) : QTreeWidgetItem(parent, type) {}

    bool operator<(const QTreeWidgetItem &other) const;
};

class zpsdControlDialog : public QDialog
{
    Q_OBJECT

public:
    explicit zpsdControlDialog(QWidget *parent);
    ~zpsdControlDialog();

    void setModel(WalletModel* model);

    static std::set<std::string> setSelectedMints;
    static std::set<CMintMeta> setMints;
    static std::vector<CMintMeta> GetSelectedMints();

private:
    Ui::zpsdControlDialog *ui;
    WalletModel* model;
    PrivacyDialog* privacyDialog;

    void updateList();
    void updateLabels();

    enum {
        COLUMN_CHECKBOX,
        COLUMN_DENOMINATION,
        COLUMN_PUBCOIN,
        COLUMN_VERSION,
        COLUMN_PRECOMPUTE,
        COLUMN_CONFIRMATIONS,
        COLUMN_ISSPENDABLE
    };
    friend class CzpsdControlWidgetItem;

private slots:
    void updateSelection(QTreeWidgetItem* item, int column);
    void ButtonAllClicked();
};

#endif // zpsdCONTROLDIALOG_H
