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



#include "qteventtransition.h"
#include "qteventtransition_p.h"
#include "qtboundevent_p.h"
#include "qtstate.h"
#include "qtstate_p.h"
#include "qtstatemachine.h"
#include "qtstatemachine_p.h"
#include <qdebug.h>

QT_BEGIN_NAMESPACE

/*!
  \class QtEventTransition

  \brief The QtEventTransition class provides a QObject-specific transition for Qt events.

  
  

  A QtEventTransition object binds an event to a particular QObject.
  QtEventTransition is part of \l{The State Machine Framework}.

  Example:

  \code
  QPushButton *button = ...;
  QtState *s1 = ...;
  QtState *s2 = ...;
  // If in s1 and the button receives an Enter event, transition to s2
  QtEventTransition *enterTransition = new QtEventTransition(button, QEvent::Enter);
  enterTransition->setTargetState(s2);
  s1->addTransition(enterTransition);
  // If in s2 and the button receives an Exit event, transition back to s1
  QtEventTransition *leaveTransition = new QtEventTransition(button, QEvent::Leave);
  leaveTransition->setTargetState(s1);
  s2->addTransition(leaveTransition);
  \endcode

  \section1 Subclassing

  Many event classes have attributes in addition to the event type itself.
  The testEventCondition() function can be reimplemented to check attributes
  of an event instance in order to determine whether the transition should be
  triggered or not.

  \sa QtState::addTransition()
*/

/*!
    \property QtEventTransition::object

    \brief the event source that this event transition is associated with
*/

/*!
    \property QtEventTransition::eventType

    \brief the type of event that this event transition is associated with
*/
QtEventTransitionPrivate::QtEventTransitionPrivate()
{
    object = 0;
    eventType = QEvent::None;
    registered = false;
}

QtEventTransitionPrivate *QtEventTransitionPrivate::get(QtEventTransition *q)
{
    return q->d_func();
}

void QtEventTransitionPrivate::invalidate()
{
    Q_Q(QtEventTransition);
    if (registered) {
        QtState *source = sourceState();
        QtStatePrivate *source_d = QtStatePrivate::get(source);
        QtStateMachinePrivate *mach = QtStateMachinePrivate::get(source_d->machine());
        if (mach) {
            mach->unregisterEventTransition(q);
            if (mach->configuration.contains(source))
                mach->registerEventTransition(q);
        }
    }
}

/*!
  Constructs a new QtEventTransition object with the given \a sourceState.
*/
QtEventTransition::QtEventTransition(QtState *sourceState)
    : QtTransition(*new QtEventTransitionPrivate, sourceState)
{
}

/*!
  Constructs a new QtEventTransition object associated with events of the given
  \a type for the given \a object, and with the given \a sourceState.
*/
QtEventTransition::QtEventTransition(QObject *object, QEvent::Type type,
                                   QtState *sourceState)
    : QtTransition(*new QtEventTransitionPrivate, sourceState)
{
    Q_D(QtEventTransition);
    d->registered = false;
    d->object = object;
    d->eventType = type;
}

/*!
  Constructs a new QtEventTransition object associated with events of the given
  \a type for the given \a object. The transition has the given \a targets and
  \a sourceState.
*/
QtEventTransition::QtEventTransition(QObject *object, QEvent::Type type,
                                   const QList<QtAbstractState*> &targets,
                                   QtState *sourceState)
    : QtTransition(*new QtEventTransitionPrivate, targets, sourceState)
{
    Q_D(QtEventTransition);
    d->registered = false;
    d->object = object;
    d->eventType = type;
}

/*!
  \internal
*/
QtEventTransition::QtEventTransition(QtEventTransitionPrivate &dd, QtState *parent)
    : QtTransition(dd, parent)
{
}

/*!
  \internal
*/
QtEventTransition::QtEventTransition(QtEventTransitionPrivate &dd, QObject *object,
                                   QEvent::Type type, QtState *parent)
    : QtTransition(dd, parent)
{
    Q_D(QtEventTransition);
    d->registered = false;
    d->object = object;
    d->eventType = type;
}

/*!
  \internal
*/
QtEventTransition::QtEventTransition(QtEventTransitionPrivate &dd, QObject *object,
                                   QEvent::Type type, const QList<QtAbstractState*> &targets,
                                   QtState *parent)
    : QtTransition(dd, targets, parent)
{
    Q_D(QtEventTransition);
    d->registered = false;
    d->object = object;
    d->eventType = type;
}

/*!
  Destroys this QObject event transition.
*/
QtEventTransition::~QtEventTransition()
{
}

/*!
  Returns the event type that this event transition is associated with.
*/
QEvent::Type QtEventTransition::eventType() const
{
    Q_D(const QtEventTransition);
    return d->eventType;
}

/*!
  Sets the event \a type that this event transition is associated with.
*/
void QtEventTransition::setEventType(QEvent::Type type)
{
    Q_D(QtEventTransition);
    if (d->eventType == type)
        return;
    d->eventType = type;
    d->invalidate();
}

/*!
  Returns the event source associated with this event transition.
*/
QObject *QtEventTransition::eventSource() const
{
    Q_D(const QtEventTransition);
    return d->object;
}

/*!
  Sets the event source associated with this event transition to be the given
  \a object.
*/
void QtEventTransition::setEventSource(QObject *object)
{
    Q_D(QtEventTransition);
    if (d->object == object)
        return;
    d->object = object;
    d->invalidate();
}

/*!
  \reimp
*/
bool QtEventTransition::eventTest(QEvent *event) const
{
    Q_D(const QtEventTransition);
#ifdef QT_STATEMACHINE_SOLUTION
    if (event->type() == QEvent::Type(QEvent::User-3)) {
#else
    if (event->type() == QEvent::Bound) {
#endif
        QtBoundEvent *oe = static_cast<QtBoundEvent*>(event);
        return (oe->object() == d->object)
            && (oe->event()->type() == d->eventType)
            && testEventCondition(oe->event());
    }
    return false;
}

/*!
  Tests an instance of an event associated with this event transition and
  returns true if the transition should be taken, otherwise returns false.
  The type of the given \a event will be eventType().

  Reimplement this function if you have custom conditions associated with
  the transition. The default implementation always returns true.
*/
bool QtEventTransition::testEventCondition(QEvent *event) const
{
    Q_UNUSED(event);
    return true;
}

/*!
  \reimp
*/
bool QtEventTransition::event(QEvent *e)
{
    return QtTransition::event(e);
}

QT_END_NAMESPACE
