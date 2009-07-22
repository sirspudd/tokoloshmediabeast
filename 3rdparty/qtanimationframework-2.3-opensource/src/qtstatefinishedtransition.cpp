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



#include "qtstatefinishedtransition.h"
#include "qtstatefinishedevent.h"
#include "qttransition_p.h"

QT_BEGIN_NAMESPACE

/*!
  \class QtStateFinishedTransition

  \brief The QtStateFinishedTransition class provides a transition that triggers when a state is finished.

  

  A state is finished when one of its final child states (a QtFinalState) is
  entered; this will cause a QtStateFinishedEvent to be generated. The
  QtStateFinishedTransition class provides a way of associating a transition
  with such an event. QtStateFinishedTransition is part of \l{The State Machine
  Framework}.

  \code
    QtStateMachine machine;
    QtState *s1 = new QtState(machine.rootState());
    QtState *s11 = new QtState(s1);
    QtFinalState *s12 = new QtFinalState(s1);
    s11->addTransition(s12);

    QtState *s2 = new QtState(machine.rootState());
    QtStateFinishedTransition *finishedTransition = new QtStateFinishedTransition(s1);
    finishedTransition->setTargetState(s2);
    s1->addTransition(finishedTransition);
  \endcode

  \sa QtState::addFinishedTransition(), QtStateFinishedEvent
*/

/*!
    \property QtStateFinishedTransition::state

    \brief the state whose QtStateFinishedEvent this transition is associated with
*/

class QtStateFinishedTransitionPrivate : public QtTransitionPrivate
{
    Q_DECLARE_PUBLIC(QtStateFinishedTransition)
public:
    QtStateFinishedTransitionPrivate();

    static QtStateFinishedTransitionPrivate *get(QtStateFinishedTransition *q);

    QtState *state;
};

QtStateFinishedTransitionPrivate::QtStateFinishedTransitionPrivate()
{
    state = 0;
}

QtStateFinishedTransitionPrivate *QtStateFinishedTransitionPrivate::get(QtStateFinishedTransition *q)
{
    return q->d_func();
}

/*!
  Constructs a new QtStateFinishedTransition object that has the given \a
  sourceState.
*/
QtStateFinishedTransition::QtStateFinishedTransition(QtState *sourceState)
    : QtTransition(*new QtStateFinishedTransitionPrivate, sourceState)
{
}

/*!
  Constructs a new QtStateFinishedTransition object associated with the given
  \a state, and that has the given \a targets and \a sourceState.
*/
QtStateFinishedTransition::QtStateFinishedTransition(
    QtState *state, const QList<QtAbstractState*> &targets, QtState *sourceState)
    : QtTransition(*new QtStateFinishedTransitionPrivate, targets, sourceState)
{
    Q_D(QtStateFinishedTransition);
    d->state = state;
}

/*!
  Destroys this QtStateFinishedTransition.
*/
QtStateFinishedTransition::~QtStateFinishedTransition()
{
}

/*!
  Returns the state associated with this QtStateFinishedTransition.
*/
QtState *QtStateFinishedTransition::state() const
{
    Q_D(const QtStateFinishedTransition);
    return d->state;
}

/*!
  Sets the \a state associated with this QtStateFinishedTransition.
*/
void QtStateFinishedTransition::setState(QtState *state)
{
    Q_D(QtStateFinishedTransition);
    d->state = state;
}

/*!
  \reimp
*/
bool QtStateFinishedTransition::eventTest(QEvent *event) const
{
    Q_D(const QtStateFinishedTransition);
#ifndef QT_STATEMACHINE_SOLUTION
    if (event->type() == QEvent::StateFinished) {
#else
    if (event->type() == QEvent::Type(QEvent::User-2)) {
#endif
        QtStateFinishedEvent *sfe = static_cast<QtStateFinishedEvent*>(event);
        return (sfe->state() == d->state);
    }
    return false;
}

/*!
  \reimp
*/
bool QtStateFinishedTransition::event(QEvent *e)
{
    return QtTransition::event(e);
}

QT_END_NAMESPACE
