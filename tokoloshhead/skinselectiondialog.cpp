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
    void handleSelectionChanged(const QModelIndex & current, const QModelIndex & previous);
private:
    SkinSelectionDialog *pSelf;
};

QPointer<SkinSelectionDialog> SkinSelectionDialog::Private::instance = 0;

SkinSelectionDialog::Private::Private(SkinSelectionDialog *pSSD)
    : skinDirModel(QCoreApplication::applicationDirPath() + "/skins"),
      pSelf(pSSD)
{
    skinView = new QListView(pSSD);
    skinView->setModel(&skinDirModel);
    connect(skinView->selectionModel(),
            SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            SLOT(handleSelectionChanged(QModelIndex, QModelIndex)));
}

void SkinSelectionDialog::Private::handleSelectionChanged(const QModelIndex &current, const QModelIndex &/*previous*/)
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

#include "skinSelectionDialog.moc"
