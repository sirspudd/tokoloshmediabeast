class TrackModel : public QAbstractTableModel
{
    Q_OBJECT
public:

    void setColumns(const QList<Playlist::Field> &columns);
    TrackModel(QObject *parent = 0);
    virtual QModelIndex index(int row, int column,
                              const QModelIndex &parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
public slots:
    void onCountChanged(int count);
    void onTrackDataReceived(int track, TrackInfo info, const QVariant &data);
    void clearCache();
private:
    int column(TrackInfo info) const { return d.columns.indexOf(info); }
    struct Private {
        QHash<int, QSet<TrackInfo, QVariant> > data;
//        QHash<int, QList<QPair<TrackInfo, QVariant> > > data; // ### is this faster? Do we care here?
        QVector<TrackInfo> columns;
        int rowCount;
        // bool blockIncomingTrackData; Do I need to make sure everything is in sync?
    } d;
};
