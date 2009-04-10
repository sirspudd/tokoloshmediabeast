#ifndef SKINWIDGET
#define SKINWIDGET

#include <QWidget>

enum GUI
{
    Designer, Winamp, Glory
};

class SkinWidget : public QWidget
{
    Q_OBJECT
public:
    SkinWidget(GUI interface, QWidget* parent);
    ~SkinWidget();
protected:
    void paintEvent(QPaintEvent *e);
    QSize sizeHint() const;
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
public slots:
    void setTrackName(QString);
    void setSkinPath(QString path);
    void setShuffleState(bool shuffleState);
    void setRepeatState(bool repeatState);
signals:
    void play();
    void pause();
    void prev();
    void next();
    void stop();
    void open();
    void openSkin();
    void skinChange();

    void shuffle();
    void repeat();

    void skinChanged();
private:
    class Private;
    Private *d;
};

#endif

/*
    Donald Carr (sirspudd_at_gmail.com) plays songs occasionally
    Copyright (C) 2007 Donald Car

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNES FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
