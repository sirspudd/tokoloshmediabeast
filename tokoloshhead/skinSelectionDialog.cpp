#include "skinSelectionDialog.h"

#include <QDebug>
#include <QListView>
#include <QPointer>
#include <QDirModel>
#include <QVBoxLayout>
#include <QCoreApplication>

struct SkinSelectionDialog::Private
{
    static QPointer<SkinSelectionDialog> instance;
};

QPointer<SkinSelectionDialog> SkinSelectionDialog::Private::instance = 0;

SkinSelectionDialog* SkinSelectionDialog::instance()
{
    if(!Private::instance) {
        Private::instance = new SkinSelectionDialog();
    }
    return Private::instance;
}

    SkinSelectionDialog::SkinSelectionDialog( QWidget * parent, Qt::WindowFlags f)
    : QDialog(parent, f),
      d( new Private )
{
    setAttribute(Qt::WA_DeleteOnClose);
    setModal(false);
    setWindowTitle(tr("Select skin"));
    //Not going to use this ludicrous model, will create custom model from QDir listing, returning
    //#1 skin name as DisplayRole
    //#2 full path as UserRole
    QDirModel *skinDirModel = new QDirModel(this);
    QListView *skinView = new QListView(this);
    skinView->setModel(skinDirModel);
    skinView->setRootIndex(skinDirModel->index(QCoreApplication::applicationDirPath() + "/skins"));
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(skinView);
}

SkinSelectionDialog::~SkinSelectionDialog()
{
    delete d;
    d = 0;
}
