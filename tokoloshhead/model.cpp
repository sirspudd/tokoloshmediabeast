#include "model.h"

TrackModel::TrackModel(QDBusInterface *interface, QObject *parent)
    : QAbstractTableModel(parent)
{
    d.interface = interface;
    d.rowCount = 0;
    QDBusPendingReply<int> reply = interface->call("count");
    reply.waitForFinished();
    d.rowCount = reply.value();
    qDebug() << "rowCount" << d.rowCount;

    interface->callWithCallback("count", QList<QVariant>(), this, SLOT(onTrackCountChanged(int)));

    d.columns.append(PlaylistIndex);
    d.columns.append(Title);
    d.columns.append(FileName);
    connect(interface, SIGNAL(tracksInserted(int, int)), this, SLOT(onTracksInserted(int, int)));
    connect(interface, SIGNAL(tracksRemoved(int, int)), this, SLOT(onTracksRemoved(int, int)));
    connect(interface, SIGNAL(trackMoved(int, int)), this, SLOT(onTrackMoved(int, int)));
    connect(interface, SIGNAL(tracksSwapped(int, int)), this, SLOT(onTracksSwapped(int, int)));
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
    if (role != Qt::DisplayRole && role != Qt::EditRole) { // Decoration role, album art?
        return QVariant();
    }

    const TrackInfo info = d.columns.at(index.column());
    TrackData &data = d.data[index.row()];
    static const QString fetchMessage = tr("Fetching data...");
    qDebug() << info << data.fields;

    if (!(data.fields & info)) {
        qDebug("%s %d: if (!(data.fields & info)) {", __FILE__, __LINE__);
        QList<QVariant> args;
        args.append(index.row());
        args.append(All);
        QDBusPendingReply<QVariant> reply = d.interface->call("trackData", args);
        reply.waitForFinished();
        qDebug() << int(reply.value().type());
        const_cast<TrackModel*>(this)->onTrackDataReceived(reply.value());
        d.interface->callWithCallback("trackData", args, const_cast<TrackModel*>(this),
                                      SLOT(onTrackDataReceived(QVariant)));
        return fetchMessage;
    }
    return data.data(info);
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
    if (row > d.rowCount || count < 1 || row < 0 || parent.isValid())
        return false;

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
    const QMap<int, TrackData>::iterator end = d.data.upperBound(row + count);
    while (it != end) {
        d.data.erase(it++);
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

void TrackModel::onTrackDataReceived(const QVariant &variant)
{
    const TrackData data = qVariantValue<TrackData>(variant);
    qDebug() << "got some stuff" << data.playlistIndex << data.path << data.fields;
    Q_ASSERT((data.fields & (PlaylistIndex|FilePath)) == (PlaylistIndex|FilePath));
    const int track = data.playlistIndex;
    Q_ASSERT(track < d.rowCount);
    d.data[track] = data;
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
    qDebug() << "count changed" << count << d.rowCount;
    if (d.rowCount > count) {
        removeRows(count, d.rowCount - 1);
    } else if (d.rowCount < count) {
        insertRows(d.rowCount, count - 1);
    }
}
