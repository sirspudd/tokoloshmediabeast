#include "controlpanel.h"

#include "player.h"

#include <QDebug>
#include <QListView>
#include <QPointer>
#include <QDirModel>
#include <QVBoxLayout>
#include <QCoreApplication>
#include <QFileSystemWatcher>

class ControlPanelItem : public QWidget
{
    public:
        ControlPanelItem(QWidget *p = 0);
    private:
        QLabel *text;
        QWidget *control;
};

ControlPanelItem::ControlPanelItem(QWidget *p)
    : QWidget(p)
{
    QHBoxLayout *hBox = new QHBoxLayout(this);
    text = new QLabel("foo");
    control = new QCheckBox();
    hBox->addWidget(text);
    hBox->addWidget(control);
}

class ControlPanel::Private : public QObject
{
    Q_OBJECT
public:
    Private(ControlPanel *pSSD);
    static QPointer<ControlPanel> instance;
private:
    ControlPanel *pSelf;
};

#include "controlpanel.moc"
QPointer<ControlPanel> ControlPanel::Private::instance = 0;

ControlPanel::Private::Private(ControlPanel *pSSD)
      : pSelf(pSSD)
{
}

ControlPanel* ControlPanel::instance(Player *player)
{
    if (!Private::instance) {
        Private::instance = new ControlPanel();
   }
    return Private::instance;
}

ControlPanel::ControlPanel(QWidget * parent, Qt::WindowFlags f)
    : QWidget(parent, f),
      d(new Private(this))
{
    setAttribute(Qt::WA_QuitOnClose, false);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Control panel"));
    QVBoxLayout *layout = new QVBoxLayout(this);
    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, this);
    connect(box, SIGNAL(clicked(QAbstractButton*)), this, SLOT(close()));
    layout->addWidget(new ControlPanelItem());
    layout->addWidget(new ControlPanelItem());
    layout->addWidget(box);
}

ControlPanel::~ControlPanel()
{
    delete d;
    d = 0;
}

