Copyright (c) 2010, Anders Bakken, Donald Carr
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
Neither the name of any associated organizations nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "skinselectiondialog.h"

#include "player.h"

#include <QDebug>
#include <QListView>
#include <QPointer>
#include <QDirModel>
#include <QVBoxLayout>
#include <QCoreApplication>
#include <QFileSystemWatcher>

class SkinSelectionModel : public QAbstractListModel
{
    Q_OBJECT
public:
    SkinSelectionModel(const QString & path, QObject * parent = 0);
    QVariant data(const QModelIndex & index, int role) const;
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
public slots:
    void updateAvailableSkins(const QString &skinPath);
private:
    QFileInfoList availableDirs;
    QFileSystemWatcher watchedDir;
};

SkinSelectionModel::SkinSelectionModel(const QString & path, QObject *parent)
    : QAbstractListModel(parent),
      watchedDir(QStringList(path))
{
    connect(&watchedDir,
            SIGNAL(directoryChanged(const QString &)),
            SLOT(updateAvailableSkins(const QString &)));

    updateAvailableSkins(path);

}

QVariant SkinSelectionModel::data(const QModelIndex & index, int role) const
{
    if (index.row() < 0 || index.row() >= availableDirs.size())
        return QVariant();

    switch (role) {
    case Qt::UserRole:
        return availableDirs.at(index.row()).absoluteFilePath();
    case Qt::DisplayRole:
        return availableDirs.at(index.row()).baseName();
    default:
        return QVariant();
    }
}

int SkinSelectionModel::rowCount(const QModelIndex & parent) const
{
    if (parent.isValid())
        return 0;
    return availableDirs.length();
}

void SkinSelectionModel::updateAvailableSkins(const QString &skinPath)
{
    availableDirs = QDir(skinPath).entryInfoList(QStringList(),
                                                 QDir::Dirs|QDir::NoDotAndDotDot|QDir::Readable);
    reset();
    int i = availableDirs.size() - 1;
    while (i >= 0) {
        if (!Player::verifySkin(availableDirs.at(i).absoluteFilePath())) {
            availableDirs.removeAt(i);
        }
        --i;
    }
}

class SkinSelectionDialog::Private : public QObject
{
    Q_OBJECT
public:
    Private(SkinSelectionDialog *pSSD);
    QListView *skinView;
    SkinSelectionModel skinDirModel;
    static QPointer<SkinSelectionDialog> instance;
public slots:
    void handleSelectionChanged(const QModelIndex & current);
private:
    SkinSelectionDialog *pSelf;
};

#include "skinselectiondialog.moc"
QPointer<SkinSelectionDialog> SkinSelectionDialog::Private::instance = 0;

SkinSelectionDialog::Private::Private(SkinSelectionDialog *pSSD)
    : skinDirModel(QCoreApplication::applicationDirPath() + "/skins"),
      pSelf(pSSD)
{
    skinView = new QListView(pSSD);
    skinView->setModel(&skinDirModel);
    connect(skinView, SIGNAL(clicked(QModelIndex)),
            this, SLOT(handleSelectionChanged(QModelIndex)));
    connect(skinView, SIGNAL(doubleClicked(QModelIndex)),
            pSSD, SLOT(close()));

}

void SkinSelectionDialog::Private::handleSelectionChanged(const QModelIndex &current)
{
    emit pSelf->skinSelected(current.data(Qt::UserRole).toString());
}

SkinSelectionDialog* SkinSelectionDialog::instance(Player *player)
{
    if (!Private::instance) {
        Private::instance = new SkinSelectionDialog();

        connect(Private::instance,
                SIGNAL(skinSelected(const QString &)),
                player,
                SLOT(setSkin(const QString &)));
    }
    return Private::instance;
}

SkinSelectionDialog::SkinSelectionDialog(QWidget * parent, Qt::WindowFlags f)
    : QDialog(parent, f),
      d(new Private(this))
{
    setAttribute(Qt::WA_QuitOnClose, false);
    setAttribute(Qt::WA_DeleteOnClose);
    setModal(false);
    setWindowTitle(tr("Select skin"));
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(d->skinView);
    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, this);
    connect(box, SIGNAL(clicked(QAbstractButton*)), this, SLOT(close()));
    layout->addWidget(box);
}

SkinSelectionDialog::~SkinSelectionDialog()
{
    delete d;
    d = 0;
}

