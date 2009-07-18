#ifndef TAGINTERFACE_H
#define TAGINTERFACE_H

#include <QUrl>
struct TrackData;
class TagInterface
{
public:
    virtual ~TagInterface() {}
    virtual bool trackData(TrackData *data, const QUrl &path, int types = All) const = 0;
};

#endif
