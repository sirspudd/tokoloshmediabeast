#ifndef ID3TAGINTERFACE_H
#define ID3TAGINTERFACE_H

#include <QUrl>
#include <global.h>
#include "taginterface.h"
#include <id3/tag.h>

class ID3TagInterface : public TagInterface
{
public:
    static QList<ID3_FrameID> ids(TrackInfo info)
    {
        QList<ID3_FrameID> ids;
        switch (info) {
        case None:
        case All:
        case URL:
        case PlaylistIndex:
        case FileName:
        case Genre:
            break;
        case Title: ids << ID3FID_TITLE; break;
        case TrackLength: ids << ID3FID_SONGLEN; break;
        case Artist: ids << ID3FID_ORIGARTIST << ID3FID_ORIGARTIST << ID3FID_BAND; break;
        case Album: ids << ID3FID_ALBUM; break;
        case Year: ids << ID3FID_YEAR << ID3FID_ORIGYEAR << ID3FID_DATE; break;
        case AlbumIndex: ids << ID3FID_TRACKNUM; break;
        }
        return ids;
    }

    virtual bool trackData(TrackData *data, const QUrl &path, int types = All) const
    {
        QFileInfo fi = path.toLocalFile();
        if (!fi.exists())
            return false;

        ID3_Tag tag;
        tag.Link(qPrintable(fi.absoluteFilePath()));
        bool success = false;
        for (int i=0; ::trackInfos[i] != None; ++i) {
            if (!(types & ::trackInfos[i]))
                continue;
            const QList<ID3_FrameID> list = ids(::trackInfos[i]);
            if (list.isEmpty())
                continue;

            QVariant variant;

            bool found = false;
            foreach(const ID3_FrameID id, list) {
                if (ID3_Frame *frame = tag.Find(id)) {
                    qDebug() << id << trackInfoToString(trackInfos[i]) << fi.fileName();
                    ID3_Frame::Iterator *it = frame->CreateIterator();
                    forever {
                        const ID3_Field *fld = it->GetNext();
                        if (!fld)
                            break;
                        QByteArray byteArray(fld->Size(), '\0');
                        const int ret = fld->Get(byteArray.data(), byteArray.size());
                        byteArray.resize(ret);
                        if (ret > 0) {
                            variant = byteArray;
                            found = true;
                            success = true;
                            break;
                        }
                    }
                    delete it;
                    if (found)
                        break;
                } else {
                    qDebug() << "nothing here" << id << trackInfoToString(trackInfos[i]) << fi.fileName();
                }
            }
            if (found) {
                data->setData(trackInfos[i], variant);
            }
        }
        return success;
    }
};

#endif
