Copyright (c) 2010 Anders Bakken
Copyright (c) 2010 Donald Carr
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
Neither the name of any associated organizations nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
