#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QtGui>
#include <model.h>
#include "../shared/config.h"
class PlaylistWidget : public QWidget
{
    Q_OBJECT
public:
    PlaylistWidget(TrackModel *model, QWidget *parent = 0)
        : QWidget(parent)
    {
        setAttribute(Qt::WA_DeleteOnClose, false);
        d.model = model;
        QVBoxLayout *l = new QVBoxLayout(this);
        l->addWidget(d.tableView = new QTableView);
        d.tableView->setModel(model);
        d.tableView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    }

    void closeEvent(QCloseEvent *e)
    {
        Config::setValue("playlist/geometry", saveGeometry());
        QWidget::closeEvent(e);
    }

    void showEvent(QShowEvent *e)
    {
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
private:
    struct Data {
        TrackModel *model;
        QTableView *tableView;
    } d;
};


#endif
