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



#include "qtabstractstate.h"
#include "qtabstractstate_p.h"
#include "qtstatemachine.h"
#include "qtstatemachine_p.h"
#include "qtstate.h"

QT_BEGIN_NAMESPACE

/*!
  \class QtAbstractState

  \brief The QtAbstractState class is the base class of states of a QtStateMachine.

  
  

  The QtAbstractState class is the abstract base class of states that are part
  of a QtStateMachine. It defines the interface that all state objects have in
  common. QtAbstractState is part of \l{The State Machine Framework}.

  The assignProperty() function is used for defining property assignments that
  should be performed when a state is entered.

  The parentState() function returns the state's parent state.

  \section1 Subclassing

  The onEntry() function is called when the state is entered; reimplement this
  function to perform custom processing when the state is entered.

  The onExit() function is called when the state is exited; reimplement this
  function to perform custom processing when the state is exited.
*/

/*!
   \enum QtAbstractState::RestorePolicy

   This enum specifies the restore policy type for a state. The restore policy
   takes effect when the machine enters a state which sets one or more
   properties. If the restore policy of the state is set to RestoreProperties,
   the state machine will save the original value of the property before the
   new value is set.

   Later, when the machine either enters a state which has its restore policy
   set to DoNotRestoreProperties or when it enters a state which does not set
   a value for the given property, the property will automatically be restored
   to its initial value.

   Only one initial value will be saved for any given property. If a value for a property has 
   already been saved by the state machine, it will not be overwritten until the property has been
   successfully restored. Once the property has been restored, the state machine will clear the 
   initial value until it enters a new state which sets the property and which has RestoreProperties
   as its restore policy.

   \value GlobalRestorePolicy The restore policy for the state should be retrieved using 
          QtStateMachine::globalRestorePolicy()
   \value DoNotRestoreProperties The state machine should not save the initial values of properties 
          set in the state and restore them later.
   \value RestoreProperties The state machine should save the initial values of properties 
          set in the state and restore them later.


   \sa setRestorePolicy(), restorePolicy(), QtAbstractState::assignProperty()
*/

/*!
   \property QtAbstractState::restorePolicy

    \brief the restore policy of this state
*/

QtAbstractStatePrivate::QtAbstractStatePrivate()
    : restorePolicy(QtAbstractState::GlobalRestorePolicy)
{
}

QtAbstractStatePrivate *QtAbstractStatePrivate::get(QtAbstractState *q)
{
    return q->d_func();
}

const QtAbstractStatePrivate *QtAbstractStatePrivate::get(const QtAbstractState *q)
{
    return q->d_func();
}

QtStateMachine *QtAbstractStatePrivate::machine() const
{
    Q_Q(const QtAbstractState);
    QObject *par = q->parent();
    while (par != 0) {
        if (QtStateMachine *mach = qobject_cast<QtStateMachine*>(par))
            return mach;
        par = par->parent();
    }
    return 0;
}

void QtAbstractStatePrivate::callOnEntry()
{
    Q_Q(QtAbstractState);
    q->onEntry();
}

void QtAbstractStatePrivate::callOnExit()
{
    Q_Q(QtAbstractState);
    q->onExit();
}

/*!
  Constructs a new state with the given \a parent state.
*/
QtAbstractState::QtAbstractState(QtState *parent)
    : QObject(
#ifndef QT_STATEMACHINE_SOLUTION
        *new QtAbstractStatePrivate,
#endif
        parent)
#ifdef QT_STATEMACHINE_SOLUTION
    , d_ptr(new QtAbstractStatePrivate)
#endif
{
#ifdef QT_STATEMACHINE_SOLUTION
    d_ptr->q_ptr = this;
#endif
}

/*!
  \internal
*/
QtAbstractState::QtAbstractState(QtAbstractStatePrivate &dd, QtState *parent)
    : QObject(
#ifndef QT_STATEMACHINE_SOLUTION
        dd,
#endif
        parent)
#ifdef QT_STATEMACHINE_SOLUTION
    , d_ptr(&dd)
#endif
{
#ifdef QT_STATEMACHINE_SOLUTION
    d_ptr->q_ptr = this;
#endif
}

/*!
  Destroys this state.
*/
QtAbstractState::~QtAbstractState()
{
#ifdef QT_STATEMACHINE_SOLUTION
    delete d_ptr;
#endif
}

/*!
  Returns this state's parent state, or 0 if the state has no parent state.
*/
QtState *QtAbstractState::parentState() const
{
    return qobject_cast<QtState*>(parent());
}

/*!
  Instructs this state to set the property with the given \a name of the given
  \a object to the given \a value when the state is entered.
*/
void QtAbstractState::assignProperty(QObject *object, const char *name,
                                    const QVariant &value)
{
    Q_D(QtAbstractState);
    for (int i = 0; i < d->propertyAssignments.size(); ++i) {
        QPropertyAssignment &assn = d->propertyAssignments[i];
        if ((assn.object == object) && (assn.propertyName == name)) {
            assn.value = value;
            return;
        }
    }
    d->propertyAssignments.append(QPropertyAssignment(object, name, value));
}

/*!
  Sets the restore policy of this state to \a restorePolicy. 
  
  The default restore policy is QtAbstractState::GlobalRestorePolicy.
*/
void QtAbstractState::setRestorePolicy(RestorePolicy restorePolicy)
{
    Q_D(QtAbstractState);
    d->restorePolicy = restorePolicy;
}

/*!
  Returns the restore policy for this state.
*/
QtAbstractState::RestorePolicy QtAbstractState::restorePolicy() const
{
    Q_D(const QtAbstractState);
    return d->restorePolicy;
}

/*!
  \fn QtAbstractState::onExit()

  This function is called when the state is exited.  Reimplement this function
  to perform custom processing when the state is exited.
*/

/*!
  \fn QtAbstractState::onEntry()

  This function is called when the state is entered. Reimplement this function
  to perform custom processing when the state is entered.
*/

/*!
  \reimp
*/
bool QtAbstractState::event(QEvent *e)
{
    return QObject::event(e);
}

QT_END_NAMESPACE
