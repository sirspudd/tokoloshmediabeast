#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QtGui>
#include <model.h>
#include "../shared/config.h"

class PlaylistWidget : public QWidget
{
    Q_OBJECT
public:
    PlaylistWidget(QDBusInterface *interface, TrackModel *model, QWidget *parent = 0);
    void closeEvent(QCloseEvent *e);
    void showEvent(QShowEvent *e);
public slots:
    void removeSongs();
    void onActivated(const QModelIndex &idx);
    void setCurrentTrack(int i);
signals:
    void visibilityChanged(bool visible);
private:
    struct Data {
        QDBusInterface *interface;
        TrackModel *model;
        QTableView *tableView;
    } d;
};


#endif
