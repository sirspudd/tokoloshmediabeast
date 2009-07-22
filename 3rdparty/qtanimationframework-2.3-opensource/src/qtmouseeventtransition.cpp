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



#include "qtmouseeventtransition.h"
#include "qtbasicmouseeventtransition_p.h"
#include <QtGui/qpainterpath.h>

#if defined(QT_EXPERIMENTAL_SOLUTION)
# include "qteventtransition_p.h"
#else
# include <private/qeventtransition_p.h>
#endif

QT_BEGIN_NAMESPACE

/*!
  \class QtMouseEventTransition

  \brief The QtMouseEventTransition class provides a transition for mouse events.

  
  

  QtMouseEventTransition is part of \l{The State Machine Framework}.

  \sa QtState::addTransition()
*/

class QtMouseEventTransitionPrivate : public QtEventTransitionPrivate
{
    Q_DECLARE_PUBLIC(QtMouseEventTransition)
public:
    QtMouseEventTransitionPrivate();

    QtBasicMouseEventTransition *transition;
};

QtMouseEventTransitionPrivate::QtMouseEventTransitionPrivate()
{
}

/*!
  Constructs a new mouse event transition with the given \a sourceState.
*/
QtMouseEventTransition::QtMouseEventTransition(QtState *sourceState)
    : QtEventTransition(*new QtMouseEventTransitionPrivate, sourceState)
{
    Q_D(QtMouseEventTransition);
    d->transition = new QtBasicMouseEventTransition();
}

/*!
  Constructs a new mouse event transition for events of the given \a type for
  the given \a object, with the given \a button and \a sourceState.
*/
QtMouseEventTransition::QtMouseEventTransition(QObject *object, QEvent::Type type,
                                             Qt::MouseButton button,
                                             QtState *sourceState)
    : QtEventTransition(*new QtMouseEventTransitionPrivate, object, type, sourceState)
{
    Q_D(QtMouseEventTransition);
    d->transition = new QtBasicMouseEventTransition(type, button);
}

/*!
  Constructs a new mouse event transition for events of the given \a type for
  the given \a object, with the given \a button, \a targets and \a
  sourceState.
*/
QtMouseEventTransition::QtMouseEventTransition(QObject *object, QEvent::Type type,
                                             Qt::MouseButton button,
                                             const QList<QtAbstractState*> &targets,
                                             QtState *sourceState)
    : QtEventTransition(*new QtMouseEventTransitionPrivate, object, type, targets, sourceState)
{
    Q_D(QtMouseEventTransition);
    d->transition = new QtBasicMouseEventTransition(type, button);
}

/*!
  Destroys this mouse event transition.
*/
QtMouseEventTransition::~QtMouseEventTransition()
{
    Q_D(QtMouseEventTransition);
    delete d->transition;
}

/*!
  Returns the button that this mouse event transition checks for.
*/
Qt::MouseButton QtMouseEventTransition::button() const
{
    Q_D(const QtMouseEventTransition);
    return d->transition->button();
}

/*!
  Sets the \a button that this mouse event transition will check for.
*/
void QtMouseEventTransition::setButton(Qt::MouseButton button)
{
    Q_D(QtMouseEventTransition);
    d->transition->setButton(button);
}

/*!
  Returns the path for this mouse event transition.
*/
QPainterPath QtMouseEventTransition::path() const
{
    Q_D(const QtMouseEventTransition);
    return d->transition->path();
}

/*!
  Sets the \a path for this mouse event transition.
  If a valid path has been set, the transition will only trigger if the mouse
  event position (QMouseEvent::pos()) is inside the path.

  \sa QPainterPath::contains()
*/
void QtMouseEventTransition::setPath(const QPainterPath &path)
{
    Q_D(QtMouseEventTransition);
    d->transition->setPath(path);
}

/*!
  \reimp
*/
bool QtMouseEventTransition::testEventCondition(QEvent *event) const
{
    Q_D(const QtMouseEventTransition);
    d->transition->setEventType(event->type());
    return QtAbstractTransitionPrivate::get(d->transition)->callEventTest(event);
}

/*!
  \reimp
*/
bool QtMouseEventTransition::eventTest(QEvent *event) const
{
    return QtEventTransition::eventTest(event);
}

/*!
  \reimp
*/
void QtMouseEventTransition::onTransition()
{
    QtEventTransition::onTransition();
}

QT_END_NAMESPACE
