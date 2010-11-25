Copyright (c) 2010, Anders Bakken, Donald Carr
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
Neither the name of any associated organizations nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
    void onTrackDataReceived(const TrackData &data);
    void onTrackCountChanged(int count);
    void onTracksInserted(int from, int count);
    void onTracksRemoved(int from, int count);
    void onTrackMoved(int from, int to);
    void onTracksSwapped(int from, int to);
    void onTracksChanged(int from, int size);
    void onCurrentTrackChanged(int c);
private:
    void emitDataChanged(int row);
    int column(TrackInfo info) const { return d.columns.indexOf(info); }
    struct Private {
        mutable QDBusInterface *interface;
        mutable QMap<int, TrackData> data; // sorted
        mutable QHash<int, int> pendingFields;
        QVector<TrackInfo> columns;
        int rowCount;
        int current;
        // bool blockIncomingTrackData; Do I need to make sure everything is in sync?
    } d;
};

#endif
