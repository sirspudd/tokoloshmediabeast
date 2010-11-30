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
        case Artist: ids << ID3FID_LEADARTIST << ID3FID_ORIGARTIST << ID3FID_ORIGARTIST << ID3FID_BAND; break;
        case Album: ids << ID3FID_ALBUM; break;
        case Year: ids << ID3FID_YEAR << ID3FID_ORIGYEAR << ID3FID_DATE; break;
        case AlbumIndex: ids << ID3FID_TRACKNUM; break;
        }
        return ids;
    }

    virtual uint trackData(TrackData *data, const QUrl &path, int types = All) const
    {
        QFileInfo fi = path.toLocalFile();
        if (!fi.exists())
            return false;

        ID3_Tag tag;
        tag.Link(qPrintable(fi.absoluteFilePath()));
        uint ret = 0;
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
                            break;
                        }
                    }
                    delete it;
                    if (found)
                        break;
                }
            }
            if (found) {
                ret |= trackInfos[i];
                data->setData(trackInfos[i], variant);
            }
        }

#if 0
        static const ID3_FrameID ids[] = {
            ID3FID_NOFRAME, ID3FID_AUDIOCRYPTO, ID3FID_PICTURE, ID3FID_AUDIOSEEKPOINT, ID3FID_COMMENT, ID3FID_COMMERCIAL,
            ID3FID_CRYPTOREG, ID3FID_EQUALIZATION2, ID3FID_EQUALIZATION, ID3FID_EVENTTIMING, ID3FID_GENERALOBJECT, ID3FID_GROUPINGREG,
            ID3FID_INVOLVEDPEOPLE, ID3FID_LINKEDINFO, ID3FID_CDID, ID3FID_MPEGLOOKUP, ID3FID_OWNERSHIP, ID3FID_PRIVATE,
            ID3FID_PLAYCOUNTER, ID3FID_POPULARIMETER, ID3FID_POSITIONSYNC, ID3FID_BUFFERSIZE, ID3FID_VOLUMEADJ2,
            ID3FID_VOLUMEADJ, ID3FID_REVERB, ID3FID_SEEKFRAME, ID3FID_SIGNATURE, ID3FID_SYNCEDLYRICS, ID3FID_SYNCEDTEMPO,
            ID3FID_ALBUM, ID3FID_BPM, ID3FID_COMPOSER, ID3FID_CONTENTTYPE, ID3FID_COPYRIGHT, ID3FID_DATE,
            ID3FID_ENCODINGTIME, ID3FID_PLAYLISTDELAY, ID3FID_ORIGRELEASETIME, ID3FID_RECORDINGTIME, ID3FID_RELEASETIME,
            ID3FID_TAGGINGTIME, ID3FID_INVOLVEDPEOPLE2, ID3FID_ENCODEDBY, ID3FID_LYRICIST, ID3FID_FILETYPE, ID3FID_TIME, ID3FID_CONTENTGROUP,
            ID3FID_TITLE, ID3FID_SUBTITLE, ID3FID_INITIALKEY, ID3FID_LANGUAGE, ID3FID_SONGLEN, ID3FID_MUSICIANCREDITLIST, ID3FID_MEDIATYPE,
            ID3FID_MOOD, ID3FID_ORIGALBUM, ID3FID_ORIGFILENAME, ID3FID_ORIGLYRICIST, ID3FID_ORIGARTIST, ID3FID_ORIGYEAR, ID3FID_FILEOWNER,
            ID3FID_LEADARTIST, ID3FID_BAND, ID3FID_CONDUCTOR, ID3FID_MIXARTIST, ID3FID_PARTINSET, ID3FID_PRODUCEDNOTICE, ID3FID_PUBLISHER,
            ID3FID_TRACKNUM, ID3FID_RECORDINGDATES, ID3FID_NETRADIOSTATION, ID3FID_NETRADIOOWNER, ID3FID_SIZE, ID3FID_ALBUMSORTORDER,
            ID3FID_PERFORMERSORTORDER, ID3FID_TITLESORTORDER, ID3FID_ISRC, ID3FID_ENCODERSETTINGS, ID3FID_SETSUBTITLE, ID3FID_USERTEXT, ID3FID_YEAR,
            ID3FID_UNIQUEFILEID, ID3FID_TERMSOFUSE, ID3FID_UNSYNCEDLYRICS, ID3FID_WWWCOMMERCIALINFO, ID3FID_WWWCOPYRIGHT, ID3FID_WWWAUDIOFILE,
            ID3FID_WWWARTIST, ID3FID_WWWAUDIOSOURCE, ID3FID_WWWRADIOPAGE, ID3FID_WWWPAYMENT, ID3FID_WWWPUBLISHER, ID3FID_WWWUSER, ID3FID_METACRYPTO,
            ID3FID_METACOMPRESSION, ID3FID_LASTFRAMEID
        };
        for (int i=0; ids[i] != ID3FID_LASTFRAMEID; ++i) {
            if (ID3_Frame *frame = tag.Find(ids[i])) {
                ID3_Frame::Iterator *it = frame->CreateIterator();
                forever {
                    const ID3_Field *fld = it->GetNext();
                    if (!fld)
                        break;
                    QByteArray byteArray(fld->Size(), '\0');
                    const int ret = fld->Get(byteArray.data(), byteArray.size());
                    byteArray.resize(ret);
                    if (ret > 0) {
                        qDebug() << ids[i] << byteArray;
                    }
                }
                delete it;
            }
        }
#endif

        return ret;
    }
};

#endif
