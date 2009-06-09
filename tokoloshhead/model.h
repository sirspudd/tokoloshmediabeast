#ifndef MODEL_H
#define MODEL_H

#include <QtCore>
#include <QtDBus>
#include "../shared/global.h"

// class PendingCall : public QDBusPendingCallWatcher
// {
//     Q_OBJECT
// public:
//     PendingCall(


// };
class TokoloshInterface;
class TrackModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    TrackModel(QDBusInterface *interface, QObject *parent = 0);
    void setColumns(const QList<TrackInfo> &columns);
    virtual QModelIndex index(int row, int column,
                              const QModelIndex &parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex());
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex());

    void clearCache();
public slots:
    void onTrackDataReceived(const QByteArray &data);
    void onTrackCountChanged(int count);
    void onTracksInserted(int from, int count);
    void onTracksRemoved(int from, int count);
    void onTrackMoved(int from, int to);
    void onTracksSwapped(int from, int to);
private:
    void emitDataChanged(int row);
    int column(TrackInfo info) const { return d.columns.indexOf(info); }
    struct Private {
        mutable QDBusInterface *interface;
        mutable QMap<int, TrackData> data; // sorted
        QVector<TrackInfo> columns;
        int rowCount;
        // bool blockIncomingTrackData; Do I need to make sure everything is in sync?
    } d;
};

#endif
