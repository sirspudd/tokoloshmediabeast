#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QtGui>
#include <model.h>
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
        l->addWidget(d.treeView = new QTreeView);
        d.treeView->setModel(model);
    }
private:
    struct Data {
        TrackModel *model;
        QTreeView *treeView;
    } d;
};


#endif
