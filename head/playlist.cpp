#include "playlist.h"

PlaylistWidget::PlaylistWidget(QDBusInterface *interface, TrackModel *model, QWidget *parent)
    : QWidget(parent)
{
    d.interface = interface;
    setAttribute(Qt::WA_DeleteOnClose, false);
    d.model = model;
    QVBoxLayout *l = new QVBoxLayout(this);
    l->addWidget(d.tableView = new QTableView);
    d.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    d.tableView->setModel(model);
    d.tableView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    d.interface->connection().connect(SERVICE_NAME, "/", QString(), "currentTrackChanged",
                                      this, SLOT(onCurrentTrackChanged(int)));
    d.interface->callWithCallback("currentTrack", QList<QVariant>(), this, SLOT(setCurrentTrack(int)));
    connect(d.tableView, SIGNAL(activated(QModelIndex)), this, SLOT(onActivated(QModelIndex)));
    new QShortcut(QKeySequence(Qt::Key_Delete), this, SLOT(removeSongs())); // this should be properly mapped with shortcut system
}

void PlaylistWidget::closeEvent(QCloseEvent *e)
{
    emit visibilityChanged(false);
    Config::setValue("playlist/geometry", saveGeometry());
    QWidget::closeEvent(e);
}

void PlaylistWidget::showEvent(QShowEvent *e)
{
    emit visibilityChanged(true);
    if (!Config::isEnabled("ignorelastgeometry", false)) {
        const QByteArray g = Config::value<QByteArray>("playlist/geometry");
        if (!g.isEmpty()) {
            restoreGeometry(g);
            if (!QApplication::desktop()->availableGeometry(this).intersects(geometry())) {
                QRect r(QPoint(), sizeHint());
                r.moveCenter(QApplication::desktop()->availableGeometry(this).center());
                setGeometry(r);
            }
        }
    } else {
        QRect r(QPoint(), sizeHint());
        r.moveCenter(QApplication::desktop()->availableGeometry(this).center());
        setGeometry(r);
    }
    QWidget::showEvent(e);
}

void PlaylistWidget::removeSongs()
{
    // could do ranges
    QList<int> tracks;
    foreach(const QModelIndex &i, d.tableView->selectionModel()->selectedRows()) {
        tracks.append(i.row());
    }
    qSort(tracks);
    for (int i=tracks.size() - 1; i>=0; --i) {
        d.interface->asyncCall("removeTrack", tracks.at(i));
    }
}

void PlaylistWidget::onActivated(const QModelIndex &idx)
{
    d.interface->asyncCall("setCurrentTrackIndex", idx.row());
}

void PlaylistWidget::setCurrentTrack(int i)
{
    // sanity check filename?
    const QModelIndex idx = d.tableView->currentIndex();
    d.tableView->setCurrentIndex(idx.sibling(i, idx.column()));
}
