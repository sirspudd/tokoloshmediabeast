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



#include "qtactionstate.h"
#include "qtactionstate_p.h"
#include "qtstateaction.h"
#include "qtstateaction_p.h"

QT_BEGIN_NAMESPACE

/*!
  \class QtActionState

  \brief The QtActionState class provides an action-based state.

  
  

  QtActionState executes \l{QtStateAction}{state actions} when the state is
  entered and exited. QtActionState is part of \l{The State Machine Framework}.

  You can add actions to a state with the addEntryAction() and addExitAction()
  functions. The state executes the actions when the state is entered and
  exited, respectively.

  The invokeMethodOnEntry() and invokeMethodOnExit() functions are used for
  defining method invocations that should be performed when a state is entered
  and exited, respectively.

  \code
  QtState *s2 = new QtState();
  s2->invokeMethodOnEntry(&label, "showMaximized");
  machine.addState(s2);
  \endcode

  \sa QtStateAction
*/

QtActionStatePrivate::QtActionStatePrivate()
{
}

QtActionStatePrivate::~QtActionStatePrivate()
{
}

QtActionStatePrivate *QtActionStatePrivate::get(QtActionState *q)
{
    return q->d_func();
}

const QtActionStatePrivate *QtActionStatePrivate::get(const QtActionState *q)
{
    return q->d_func();
}

QList<QtStateAction*> QtActionStatePrivate::entryActions() const
{
    QList<QtStateAction*> result;
    QList<QObject*>::const_iterator it;
#ifdef QT_STATEMACHINE_SOLUTION
    const QObjectList &children = q_func()->children();
#endif
    for (it = children.constBegin(); it != children.constEnd(); ++it) {
        QtStateAction *act = qobject_cast<QtStateAction*>(*it);
        if (act && (QtStateActionPrivate::get(act)->when == QtStateActionPrivate::ExecuteOnEntry))
            result.append(act);
    }
    return result;
}

QList<QtStateAction*> QtActionStatePrivate::exitActions() const
{
    QList<QtStateAction*> result;
    QList<QObject*>::const_iterator it;
#ifdef QT_STATEMACHINE_SOLUTION
    const QObjectList &children = q_func()->children();
#endif
    for (it = children.constBegin(); it != children.constEnd(); ++it) {
        QtStateAction *act = qobject_cast<QtStateAction*>(*it);
        if (act && (QtStateActionPrivate::get(act)->when == QtStateActionPrivate::ExecuteOnExit))
            result.append(act);
    }
    return result;
}

/*!
  Constructs a new action state with the given \a parent state.
*/
QtActionState::QtActionState(QtState *parent)
    : QtAbstractState(*new QtActionStatePrivate, parent)
{
}

/*!
  \internal
*/
QtActionState::QtActionState(QtActionStatePrivate &dd,
                           QtState *parent)
    : QtAbstractState(dd, parent)
{
}

/*!
  Destroys this action state.
*/
QtActionState::~QtActionState()
{
}

/*!
  Instructs this state to invoke the given \a method of the given \a object
  with the given \a arguments when the state is entered. This function will
  create a QtStateInvokeMethodAction object and add it to the entry actions of
  the state.

  \sa invokeMethodOnExit(), addEntryAction()
*/
void QtActionState::invokeMethodOnEntry(QObject *object, const char *method,
                                       const QList<QVariant> &arguments)
{
    addEntryAction(new QtStateInvokeMethodAction(object, method, arguments));
}

/*!
  Instructs this state to invoke the given \a method of the given \a object
  with the given \a arguments when the state is exited. This function will
  create a QtStateInvokeMethodAction object and add it to the exit actions of
  the state.

  \sa invokeMethodOnEntry(), addExitAction()
*/
void QtActionState::invokeMethodOnExit(QObject *object, const char *method,
                                      const QList<QVariant> &arguments)
{
    addExitAction(new QtStateInvokeMethodAction(object, method, arguments));
}

/*!
  Adds the given \a action to this state. The action will be executed when
  this state is entered. The state takes ownership of the action.

  \sa addExitAction(), removeEntryAction()
*/
void QtActionState::addEntryAction(QtStateAction *action)
{
    if (!action) {
        qWarning("QtActionState::addEntryAction: cannot add null action");
        return;
    }
    action->setParent(this);
    QtStateActionPrivate::get(action)->when = QtStateActionPrivate::ExecuteOnEntry;
}

/*!
  Adds the given \a action to this state. The action will be executed when
  this state is exited. The state takes ownership of the action.

  \sa addEntryAction(), removeExitAction()
*/
void QtActionState::addExitAction(QtStateAction *action)
{
    if (!action) {
        qWarning("QtActionState::addExitAction: cannot add null action");
        return;
    }
    action->setParent(this);
    QtStateActionPrivate::get(action)->when = QtStateActionPrivate::ExecuteOnExit;
}

/*!
  Removes the given entry \a action from this state. The state releases
  ownership of the action.

  \sa addEntryAction()
*/
void QtActionState::removeEntryAction(QtStateAction *action)
{
    if (!action) {
        qWarning("QtActionState::removeEntryAction: cannot remove null action");
        return;
    }
    if (action->parent() == this)
        action->setParent(0);
}

/*!
  Removes the given exit \a action from this state. The state releases
  ownership of the action.

  \sa addExitAction()
*/
void QtActionState::removeExitAction(QtStateAction *action)
{
    if (!action) {
        qWarning("QtActionState::removeExitAction: cannot remove null action");
        return;
    }
    if (action->parent() == this)
        action->setParent(0);
}

/*!
  Returns this state's entry actions.

  \sa addEntryAction(), exitActions()
*/
QList<QtStateAction*> QtActionState::entryActions() const
{
    Q_D(const QtActionState);
    return d->entryActions();
}

/*!
  Returns this state's exit actions.

  \sa addExitAction(), entryActions()
*/
QList<QtStateAction*> QtActionState::exitActions() const
{
    Q_D(const QtActionState);
    return d->exitActions();
}

/*!
  \reimp
*/
void QtActionState::onEntry()
{
    Q_D(QtActionState);
    QList<QtStateAction*> actions = d->entryActions();
    for (int i = 0; i < actions.size(); ++i)
        QtStateActionPrivate::get(actions.at(i))->callExecute();
}

/*!
  \reimp
*/
void QtActionState::onExit()
{
    Q_D(QtActionState);
    QList<QtStateAction*> actions = d->exitActions();
    for (int i = 0; i < actions.size(); ++i)
        QtStateActionPrivate::get(actions.at(i))->callExecute();
}

/*!
  \reimp
*/
bool QtActionState::event(QEvent *e)
{
    return QtAbstractState::event(e);
}

QT_END_NAMESPACE
