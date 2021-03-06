/*
    Copyright (c) 2010 Anders Bakken
    Copyright (c) 2010 Donald Carr
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer. Redistributions in binary
    form must reproduce the above copyright notice, this list of conditions and
    the following disclaimer in the documentation and/or other materials
    provided with the distribution. Neither the name of any associated
    organizations nor the names of its contributors may be used to endorse or
    promote products derived from this software without specific prior written
    permission. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
    CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
    NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
    OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
    OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
    OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/

#include "model.h"

TrackModel::TrackModel(QDBusInterface *interface, QObject *parent)
    : QAbstractTableModel(parent)
{
    d.interface = interface;
    d.rowCount = 0;
    d.current = -1;

//    d.columns.append(PlaylistIndex);
    d.columns.append(FileName);
    d.columns.append(Title);
    d.columns.append(Artist);

    interface->connection().connect(SERVICE_NAME, "/", QString(), "currentTrackChanged", this, SLOT(onCurrentTrackChanged(int)));
    interface->connection().connect(SERVICE_NAME, "/", QString(), "tracksInserted", this, SLOT(onTracksInserted(int, int)));
    interface->connection().connect(SERVICE_NAME, "/", QString(), "tracksRemoved", this, SLOT(onTracksRemoved(int, int)));
    interface->connection().connect(SERVICE_NAME, "/", QString(), "tracksMoved", this, SLOT(onTracksMoved(int, int)));
    interface->connection().connect(SERVICE_NAME, "/", QString(), "tracksSwapped", this, SLOT(onTracksSwapped(int, int)));

    interface->callWithCallback("count", QList<QVariant>(), this, SLOT(onTrackCountChanged(int)));
    interface->callWithCallback("currentTrackIndex", QList<QVariant>(), this, SLOT(onCurrentTrackChanged(int)));
}

QModelIndex TrackModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() || row < 0 || row >= d.rowCount || column < 0 || column >= d.columns.size()) {
        return QModelIndex();
    }
    return createIndex(row, column);
}

int TrackModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : d.rowCount;
}

int TrackModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : d.columns.size();
}

QVariant TrackModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    switch (role) {
    case Qt::DisplayRole:
        break;
    case Qt::FontRole: {
        QFont f;
        if (index.row() == d.current)
            f.setBold(true);
        return f; }
    default:
        return QVariant();
    }

    const TrackInfo info = d.columns.at(index.column());
    TrackData &data = d.data[index.row()];
    static const QString fetchMessage = tr("Fetching data...");

    if (!(data.fields & info)) {
        int &val = d.pendingFields[index.row()];
//        qDebug() << "going to fetch" << index.row() << "because" << data.fields << info;
        if ((val & info) != info) {
            QList<QVariant> args;
            const int fields = All;
            args << index.row() << fields; // do I want all?
#if 1
            d.interface->callWithCallback("trackData", args, const_cast<TrackModel*>(this),
                                          SLOT(onTrackDataReceived(TrackData)));
//            qDebug() << "calling trackdata" << args;
#else
            const TrackData trackData = QDBusReply<TrackData>(d.interface->call("trackData", index.row(), fields)).value();
            qDebug() << d.interface->lastError() << trackData.fields << trackData.title;
            if (trackData.fields != 0) {
                const_cast<TrackModel*>(this)->onTrackDataReceived(trackData);
            }
#endif
            val |= fields;
        }
        return fetchMessage;
    }
    return data.data(info);
}

QVariant TrackModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        Q_ASSERT(section < d.columns.size());
        switch (d.columns.at(section)) {
        case URL: return tr("URL");
        case Title: return tr("Title");
        case TrackLength: return tr("Track length");
        case Artist: return tr("Artist");
        case Album: return tr("Album");
        case Year: return tr("Year");
        case Genre: return tr("Genre");
        case AlbumIndex: return tr("Album index");
        case PlaylistIndex: return tr("Playlist index");
        case FileName: return tr("File name");
        default: Q_ASSERT(0); break;
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}


bool TrackModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(index);
    Q_UNUSED(value);
    Q_UNUSED(role);
    // ### should we allow Tokolosh to be used for editing tags?
    return false;
}

bool TrackModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (row > d.rowCount || count < 1 || row < 0 || parent.isValid()) {
        return false;
    }

    beginInsertRows(parent, row, row + count - 1);

    QMap<int, TrackData> &data = d.data;
    QMap<int, TrackData>::iterator it = data.lowerBound(row);
    const QMap<int, TrackData>::iterator end = data.end();
    QMap<int, TrackData> moved;
    while (it != end) {
        const TrackData value = it.value();
        moved[it.key() + 1] = value;
        data.erase(it);
        ++it;
    }
    d.rowCount += count;
    data.unite(moved);
    endInsertRows();
    return true;
}

bool TrackModel::insertColumns(int, int, const QModelIndex &)
{
    Q_ASSERT(0);
    return false; // done through separate API
}

bool TrackModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || row < 0 || count < 1 || row + count > d.rowCount) {
        return false;
    }
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    d.rowCount -= count;
    QMap<int, TrackData>::iterator it = d.data.lowerBound(row);
    QMap<int, TrackData>::const_iterator end = d.data.upperBound(row + count);
    while (it != end) {
        d.data.erase(it++);
    }
    end = d.data.end();
    QMap<int, TrackData> moved;
    while (it != end) {
        moved[it.key() - count] = it.value();
        moved.erase(it);
        ++it;
    }
    endRemoveRows();
    return true;
}

bool TrackModel::removeColumns(int, int, const QModelIndex &)
{
    Q_ASSERT(0);
    return false; // done through separate API
}

void TrackModel::onTracksInserted(int from, int count)
{
    insertRows(from, count, QModelIndex());
}

void TrackModel::onTracksRemoved(int from, int count)
{
    removeRows(from, count, QModelIndex());
}

void TrackModel::onTrackDataReceived(const TrackData &data)
{
//    qDebug() << "receiving trackData" << data.playlistIndex << data.title << data.fields;
    if (!(data.fields)) {
        qWarning("Somehow receiving empty trackdata");
        return;
    }
    Q_ASSERT(data.fields & PlaylistIndex);
    const int track = data.playlistIndex;
    int &ref = d.pendingFields[track];
    ref &= ~data.fields;
    if (!ref) {
        d.pendingFields.remove(track);
    }
    if (track >= d.rowCount)
        return;
    d.data[track] |= data;
//    qDebug() << "receiving" << d.data[track].fields << track << data.fields;
    emit dataChanged(index(track, 0), index(track, d.columns.size() - 1));
}

void TrackModel::onTracksSwapped(int from, int to)
{
    QMap<int, TrackData> &data = d.data;
    const TrackData fromData = data.take(from);
    const TrackData toData = data.take(to);
    if (fromData.fields)
        data[to] = fromData;
    if (toData.fields)
        data[from] = toData;

    QModelIndexList fromList, toList;
    for (int i=0; i<d.columns.size(); ++i) {
        const QModelIndex fromIdx = index(from, i);
        const QModelIndex toIdx = index(to, i);
        fromList.append(fromIdx);
        toList.append(toList);
        fromList.append(toIdx);
        toList.append(fromIdx);
    }
    changePersistentIndexList(fromList, toList);
    emitDataChanged(from);
    emitDataChanged(to);
}

void TrackModel::clearCache()
{
    d.data.clear();
}

void TrackModel::onTrackMoved(int from, int to)
{
    // ### need to change persistent indexes
    TrackData data = d.data.take(from);
    removeRow(from);
    insertRow(to);
    data.fields |= PlaylistIndex;
    data.playlistIndex = to;
    d.data[to] = data;
}

void TrackModel::emitDataChanged(int row)
{
    const QModelIndex from = index(row, 0);
    const QModelIndex to = (d.columns.size() > 1 ? index(row, d.columns.size() - 1) : from);
    emit dataChanged(from, to);
}

void TrackModel::onTrackCountChanged(int count)
{
    if (d.rowCount > count) {
        removeRows(count, d.rowCount - 1);
    } else if (d.rowCount < count) {
        insertRows(d.rowCount, count - 1);
    }
}

void TrackModel::onTracksChanged(int from, int count)
{
    emit dataChanged(index(from, 0), index(from + count, d.columns.size() - 1));
}

void TrackModel::onCurrentTrackChanged(int c)
{
//    qDebug() << "onCurrentTrackChanged" << c;
    if (c != d.current) {
        const int old = d.current;
        d.current = c;
        if (old >= 0)
            emitDataChanged(old);
        if (d.current != -1)
            emitDataChanged(d.current);
    }
}
