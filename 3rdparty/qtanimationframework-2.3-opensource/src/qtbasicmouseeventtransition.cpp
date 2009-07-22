/****************************************************************************
**
** This file is part of a Qt Solutions component.
** 
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
** 
** Contact:  Qt Software Information (qt-info@nokia.com)
** 
** Commercial Usage  
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Solutions Commercial License Agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and Nokia.
** 
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
** 
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
** 
** GNU General Public License Usage 
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
** 
** Please note Third Party Software included with Qt Solutions may impose
** additional restrictions and it is the user's responsibility to ensure
** that they have met the licensing requirements of the GPL, LGPL, or Qt
** Solutions Commercial license and the relevant license of the Third
** Party Software they are using.
** 
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
** 
****************************************************************************/



#include "qtbasicmouseeventtransition_p.h"
#include <QtGui/qevent.h>
#include <QtGui/qpainterpath.h>
#include <qdebug.h>

#if defined(QT_EXPERIMENTAL_SOLUTION)
# include "qtabstracttransition_p.h"
#else
# include <private/qabstracttransition_p.h>
#endif

QT_BEGIN_NAMESPACE

/*!
  \internal
  \class QtBasicMouseEventTransition

  \brief The QtBasicMouseEventTransition class provides a transition for Qt mouse events.
*/

class QtBasicMouseEventTransitionPrivate : public QtAbstractTransitionPrivate
{
    Q_DECLARE_PUBLIC(QtBasicMouseEventTransition)
public:
    QtBasicMouseEventTransitionPrivate();

    static QtBasicMouseEventTransitionPrivate *get(QtBasicMouseEventTransition *q);

    QEvent::Type eventType;
    Qt::MouseButton button;
    QPainterPath path;
};

QtBasicMouseEventTransitionPrivate::QtBasicMouseEventTransitionPrivate()
{
    eventType = QEvent::None;
    button = Qt::NoButton;
}

QtBasicMouseEventTransitionPrivate *QtBasicMouseEventTransitionPrivate::get(QtBasicMouseEventTransition *q)
{
    return q->d_func();
}

/*!
  Constructs a new mouse event transition with the given \a sourceState.
*/
QtBasicMouseEventTransition::QtBasicMouseEventTransition(QtState *sourceState)
    : QtAbstractTransition(*new QtBasicMouseEventTransitionPrivate, sourceState)
{
}

/*!
  Constructs a new mouse event transition for events of the given \a type.
*/
QtBasicMouseEventTransition::QtBasicMouseEventTransition(QEvent::Type type,
                                                       Qt::MouseButton button,
                                                       QtState *sourceState)
    : QtAbstractTransition(*new QtBasicMouseEventTransitionPrivate, sourceState)
{
    Q_D(QtBasicMouseEventTransition);
    d->eventType = type;
    d->button = button;
}

/*!
  Destroys this mouse event transition.
*/
QtBasicMouseEventTransition::~QtBasicMouseEventTransition()
{
}

/*!
  Returns the event type that this mouse event transition is associated with.
*/
QEvent::Type QtBasicMouseEventTransition::eventType() const
{
    Q_D(const QtBasicMouseEventTransition);
    return d->eventType;
}

/*!
  Sets the event \a type that this mouse event transition is associated with.
*/
void QtBasicMouseEventTransition::setEventType(QEvent::Type type)
{
    Q_D(QtBasicMouseEventTransition);
    d->eventType = type;
}

/*!
  Returns the button that this mouse event transition checks for.
*/
Qt::MouseButton QtBasicMouseEventTransition::button() const
{
    Q_D(const QtBasicMouseEventTransition);
    return d->button;
}

/*!
  Sets the button that this mouse event transition will check for.
*/
void QtBasicMouseEventTransition::setButton(Qt::MouseButton button)
{
    Q_D(QtBasicMouseEventTransition);
    d->button = button;
}

/*!
  Returns the path for this mouse event transition.
*/
QPainterPath QtBasicMouseEventTransition::path() const
{
    Q_D(const QtBasicMouseEventTransition);
    return d->path;
}

/*!
  Sets the path for this mouse event transition.
*/
void QtBasicMouseEventTransition::setPath(const QPainterPath &path)
{
    Q_D(QtBasicMouseEventTransition);
    d->path = path;
}

/*!
  \reimp
*/
bool QtBasicMouseEventTransition::eventTest(QEvent *event) const
{
    Q_D(const QtBasicMouseEventTransition);
    if (event->type() == d->eventType) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        return (me->button() == d->button)
            && (d->path.isEmpty() || d->path.contains(me->pos()));
    }
    return false;
}

/*!
  \reimp
*/
void QtBasicMouseEventTransition::onTransition()
{
}

QT_END_NAMESPACE
