#include "backend.h"

Backend *Backend::inst = 0;
Backend *Backend::instance()
{
    return inst;
}

Backend::Backend(BackendPrivate &dd, QObject *parent)
    : Playlist(dd, parent), d(&dd)
{
    Q_ASSERT(!inst);
    inst = this;
}
