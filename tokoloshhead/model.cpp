#include "model.h"

TrackModel::TrackModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

QModelIndex TrackModel::index(int row, int column, const QModelIndex &parent) const
{
}

int TrackModel::rowCount(const QModelIndex &) const
{
    return d.row;
}

int TrackModel::columnCount(const QModelIndex &parent) const
{
    return d.columns.size();
}

QVariant TrackModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role != Qt::DisplayRole && role != Qt::EditRole) { // Decoration role, album art?
        return QVariant();
    }
    const TrackInfo info = d.columns.at(index.column());
    const QList<QPair<TrackInfo, QVariant> &list = d.data.value(index);

    for (QList<QPair<TrackInfo, QVariant> >::const_iterator it = list.begin(); it != list.end(); ++it) {
        if ((*it).first == == info) {
            return (*it).second;
        }
    }
    playlistInterface->requestAsyncTrackData(index.row(), info);
    return tr("Fetching data..."); // store this somewhere?
}

bool TrackModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(index);
    Q_UNUSED(value);
    Q_UNUSED(role);
    // ### should we allow Tokolosh to be used for editing tags?
    return false;
}

void TrackModel::onCountChanged(int count)
{
    if (count == d.count + 1) {
        beginInsertRows(QModelIndex(), d.count, d.count);
        d.count = count;
        endInsertRows();
    } else {
        reset();
        // is this right?
    }
}

void TrackModel::onTrackDataReceived(int track, TrackInfo info, const QVariant &data)
{
    Q_ASSERT(track < count);
    d.data[track][info] = data;
    const QModelIndex idx = index(track, column(info));
    emit dataChanged(idx, idx);

}
