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



#include "qtstate.h"
#include "qtstate_p.h"
#include "qthistorystate.h"
#include "qthistorystate_p.h"
#include "qtabstracttransition.h"
#include "qtabstracttransition_p.h"
#include "qtsignaltransition.h"
#include "qtstatefinishedtransition.h"
#include "qtstatemachine.h"
#include "qtstatemachine_p.h"

QT_BEGIN_NAMESPACE

/*!
  \class QtState

  \brief The QtState class provides a general-purpose state for QtStateMachine.

  
  

  QtState objects can have child states, and can have transitions to other
  states. QtState is part of \l{The State Machine Framework}.

  The addTransition() function adds a transition. The removeTransition()
  function removes a transition.

  \section1 States with Child States

  For non-parallel state groups, the setInitialState() function must be called
  to set the initial state. The child states are mutually exclusive states,
  and the state machine needs to know which child state to enter when the
  parent state is the target of a transition.

  The addHistoryState() function adds a history state.

  The addFinishedTransition() function creates and adds a transition that's
  triggered when a final child state is entered.

  The setErrorState() sets the state's error state. The error state is the
  state that the state machine will transition to if an error is detected when
  attempting to enter the state (e.g. because no initial state has been set).
*/

/*!
  \enum QtState::Type

  This enum specifies the type of a state.

  \value Normal A normal state. If the state has no child states, it is an
  atomic state; otherwise, the child states are mutually exclusive and an
  initial state must be set by calling QtState::setInitialState().

  \value ParallelGroup The state is a parallel group state. When a parallel
  group state is entered, all its child states are entered in parallel.
*/

/*!
  \enum QtState::HistoryType

  This enum specifies the type of history that a QtHistoryState records.

  \value ShallowHistory Only the immediate child states of the parent state
  are recorded. In this case a transition with the history state as its
  target will end up in the immediate child state that the parent was in the
  last time it was exited. This is the default.

  \value DeepHistory Nested states are recorded. In this case a transition
  with the history state as its target will end up in the most deeply nested
  descendant state the parent was in the last time it was exited.
*/

QtStatePrivate::QtStatePrivate()
    : errorState(0), isParallelGroup(false), initialState(0)
{
}

QtStatePrivate::~QtStatePrivate()
{
}

QtStatePrivate *QtStatePrivate::get(QtState *q)
{
    if (!q)
        return 0;
    return q->d_func();
}

const QtStatePrivate *QtStatePrivate::get(const QtState *q)
{
    if (!q)
        return 0;
    return q->d_func();
}

/*!
  Constructs a new state with the given \a parent state.
*/
QtState::QtState(QtState *parent)
    : QtActionState(*new QtStatePrivate, parent)
{
}

/*!
  Constructs a new state of the given \a type with the given \a parent state.
*/
QtState::QtState(Type type, QtState *parent)
    : QtActionState(*new QtStatePrivate, parent)
{
    Q_D(QtState);
    d->isParallelGroup = (type == ParallelGroup);
}

/*!
  \internal
*/
QtState::QtState(QtStatePrivate &dd, QtState *parent)
    : QtActionState(dd, parent)
{
}

/*!
  Destroys this state.
*/
QtState::~QtState()
{
}

QList<QtAbstractState*> QtStatePrivate::childStates() const
{
    QList<QtAbstractState*> result;
    QList<QObject*>::const_iterator it;
#ifdef QT_STATEMACHINE_SOLUTION
    const QObjectList &children = q_func()->children();
#endif
    for (it = children.constBegin(); it != children.constEnd(); ++it) {
        QtAbstractState *s = qobject_cast<QtAbstractState*>(*it);
        if (!s || qobject_cast<QtHistoryState*>(s))
            continue;
        result.append(s);
    }
    return result;
}

QList<QtHistoryState*> QtStatePrivate::historyStates() const
{
    QList<QtHistoryState*> result;
    QList<QObject*>::const_iterator it;
#ifdef QT_STATEMACHINE_SOLUTION
    const QObjectList &children = q_func()->children();
#endif
    for (it = children.constBegin(); it != children.constEnd(); ++it) {
        QtHistoryState *h = qobject_cast<QtHistoryState*>(*it);
        if (h)
            result.append(h);
    }
    return result;
}

QList<QtAbstractTransition*> QtStatePrivate::transitions() const
{
    QList<QtAbstractTransition*> result;
    QList<QObject*>::const_iterator it;
#ifdef QT_STATEMACHINE_SOLUTION
    const QObjectList &children = q_func()->children();
#endif
    for (it = children.constBegin(); it != children.constEnd(); ++it) {
        QtAbstractTransition *t = qobject_cast<QtAbstractTransition*>(*it);
        if (t)
            result.append(t);
    }
    return result;
}

/*!
  Returns this state group's error state. 

  \sa QtStateMachine::errorState(), QtStateMachine::setErrorState()
*/
QtAbstractState *QtState::errorState() const
{
    Q_D(const QtState);
    return d->errorState;
}

/*!
  Sets this state's error state to be the given \a state. If the error state
  is not set, or if it is set to 0, the state will inherit its parent's error
  state recursively.

  \sa QtStateMachine::setErrorState(), QtStateMachine::errorState()
*/
void QtState::setErrorState(QtAbstractState *state)
{
    Q_D(QtState);
    if (state != 0 && QtAbstractStatePrivate::get(state)->machine() != d->machine()) {
        qWarning("QtState::setErrorState: error state cannot belong "
                 "to a different state machine");
        return;
    }

    d->errorState = state;
}

/*!
  Adds the given \a transition. The transition has this state as the source.
  This state takes ownership of the transition.
*/
void QtState::addTransition(QtAbstractTransition *transition)
{
    Q_D(QtState);
    if (!transition) {
        qWarning("QtState::addTransition: cannot add null transition");
        return;
    }
    const QList<QtAbstractState*> &targets = QtAbstractTransitionPrivate::get(transition)->targetStates;
    for (int i = 0; i < targets.size(); ++i) {
        QtAbstractState *t = targets.at(i);
        if (!t) {
            qWarning("QtState::addTransition: cannot add transition to null state");
            return;
        }
        if ((QtAbstractStatePrivate::get(t)->machine() != d->machine())
            && QtAbstractStatePrivate::get(t)->machine() && d->machine()) {
            qWarning("QtState::addTransition: cannot add transition "
                     "to a state in a different state machine");
            return;
        }
    }
    transition->setParent(this);
}

/*!
  Adds a transition associated with the given \a signal of the given \a sender
  object, and returns the new QtSignalTransition object. The transition has
  this state as the source, and the given \a target as the target state.
*/
QtSignalTransition *QtState::addTransition(QObject *sender, const char *signal,
                                         QtAbstractState *target)
{
    if (!sender) {
        qWarning("QtState::addTransition: sender cannot be null");
        return 0;
    }
    if (!signal) {
        qWarning("QtState::addTransition: signal cannot be null");
        return 0;
    }
    QtSignalTransition *trans = new QtSignalTransition(sender, signal, QList<QtAbstractState*>() << target);
    addTransition(trans);
    return trans;
}

/*!
  Adds a transition that's triggered by the finished event of this state, and
  returns the new QtStateFinishedTransition object. The transition has the
  given \a target state.

  \sa QtStateFinishedEvent
*/
QtStateFinishedTransition *QtState::addFinishedTransition(QtAbstractState *target)
{
    QtStateFinishedTransition *trans = new QtStateFinishedTransition(this, QList<QtAbstractState*>() << target);
    addTransition(trans);
    return trans;
}

namespace {

// ### Make public?
class UnconditionalTransition : public QtAbstractTransition
{
public:
    UnconditionalTransition(QtAbstractState *target)
        : QtAbstractTransition(QList<QtAbstractState*>() << target) {}
protected:
    void onTransition() {}
    bool eventTest(QEvent *) const { return true; }
};

} // namespace

/*!
  Adds an unconditional transition from this state to the given \a target
  state, and returns then new transition object.
*/
QtAbstractTransition *QtState::addTransition(QtAbstractState *target)
{
    UnconditionalTransition *trans = new UnconditionalTransition(target);
    addTransition(trans);
    return trans;
}

/*!
  Removes the given \a transition from this state.  The state releases
  ownership of the transition.

  \sa addTransition()
*/
void QtState::removeTransition(QtAbstractTransition *transition)
{
    Q_D(QtState);
    if (!transition) {
        qWarning("QtState::removeTransition: cannot remove null transition");
        return;
    }
    if (transition->sourceState() != this) {
        qWarning("QtState::removeTransition: transition %p's source state (%p)"
                 " is different from this state (%p)",
                 transition, transition->sourceState(), this);
        return;
    }
    QtStateMachinePrivate *mach = QtStateMachinePrivate::get(d->machine());
    if (mach)
        mach->unregisterTransition(transition);
    transition->setParent(0);
}

/*!
  Returns the list of transitions from this state, or an empty list if there
  are no transitions from this state.

  \sa addTransition(), removeTransition()
*/
QList<QtAbstractTransition*> QtState::transitions() const
{
    Q_D(const QtState);
    return d->transitions();
}

/*!
  Creates a history state of the given \a type for this state and returns the
  new state.  The history state becomes a child of this state.
*/
QtHistoryState *QtState::addHistoryState(HistoryType type)
{
    return QtHistoryStatePrivate::create(type, this);
}

/*!
  \reimp
*/
void QtState::onEntry()
{
    QtActionState::onEntry();
}

/*!
  \reimp
*/
void QtState::onExit()
{
    QtActionState::onExit();
}

/*!
  Returns this state's initial state, or 0 if the state has no initial state.
*/
QtAbstractState *QtState::initialState() const
{
    Q_D(const QtState);
    return d->initialState;
}

/*!
  Sets this state's initial state to be the given \a state.
  \a state has to be a child of this state.
*/
void QtState::setInitialState(QtAbstractState *state)
{
    Q_D(QtState);
    if (d->isParallelGroup) {
        qWarning("QtState::setInitialState: ignoring attempt to set initial state "
                 "of parallel state group %p", this);
        return;
    }
    if (state && (state->parentState() != this)) {
        qWarning("QtState::setInitialState: state %p is not a child of this state (%p)",
                 state, this);
        return;
    }
    d->initialState = state;
}

/*!
  \reimp
*/
bool QtState::event(QEvent *e)
{
    return QtActionState::event(e);
}

QT_END_NAMESPACE
