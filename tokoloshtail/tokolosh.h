#ifndef TOKOLOSH_MEDIA_BEAST_BACKEND
#define TOKOLOSH_MEDIA_BEAST_BACKEND

#include <QObject>
#include <QString>
#include <QtDBus>

class Tokolosh : public QObject
{
    Q_OBJECT

public:
    Tokolosh(QObject *parent);
    ~Tokolosh();

public Q_SLOTS:
    void load(const QString &fileName);
    void stop();
    void play();
    void pause();
    void next();
    void prev();
    void volume();
    void setVolume(int);
    void toggleMute();
    void toggleShuffle();
    void toggleRepeat();
    int getStatus();
    void syncClients();
    int speed();
    void setSpeed(int);
    void clearLibraryPaths();
    void addLibraryPath(const QString &path);
    void removeLibraryPath(const QString &path);
    void populateLibrary();
    void searchedLibraryPaths();
    QDBusVariant playlistWindow(int window = 5);
    QDBusVariant libraryPaths();
    QDBusVariant libraryContents();

Q_SIGNALS: // SIGNALS
    void crashed();
    void trackChanged(QString trackPath);
    void repeat(bool);
    void shuffle(bool);
private:
    class Private;
    Private *d;
};

#endif
/*
  Donald Carr(sirspudd_at_gmail.com) plays songs occasionally
  Copyright(C) 2007 Donald Car

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
