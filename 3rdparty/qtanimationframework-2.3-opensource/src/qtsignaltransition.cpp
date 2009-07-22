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



#include "qtsignaltransition.h"
#include "qtsignaltransition_p.h"
#include "qtsignalevent.h"
#include "qtstate.h"
#include "qtstate_p.h"
#include "qtstatemachine.h"
#include "qtstatemachine_p.h"
#include <qdebug.h>

QT_BEGIN_NAMESPACE

/*!
  \class QtSignalTransition

  \brief The QtSignalTransition class provides a transition based on a Qt signal.

  
  

  Typically you would use the overload of QtState::addTransition() that takes a
  sender and signal as arguments, rather than creating QtSignalTransition
  objects directly. QtSignalTransition is part of \l{The State Machine
  Framework}.

  You can subclass QtSignalTransition and reimplement eventTest() to make a
  signal transition conditional; the event object passed to eventTest() will
  be a QtSignalEvent object. Example:

  \code
  class CheckedTransition : public QtSignalTransition
  {
  public:
      CheckedTransition(QCheckBox *check)
          : QtSignalTransition(check, SIGNAL(stateChanged(int))) {}
  protected:
      bool eventTest(QEvent *e) const {
          if (!QtSignalTransition::eventTest(e))
              return false;
          QtSignalEvent *se = static_cast<QtSignalEvent*>(e);
          return (se->arguments().at(0).toInt() == Qt::Checked);
      }
  };

  ...

  QCheckBox *check = new QCheckBox();
  check->setTristate(true);

  QtState *s1 = new QtState();
  QtState *s2 = new QtState();
  CheckedTransition *t1 = new CheckedTransition(check);
  t1->setTargetState(s2);
  s1->addTransition(t1);
  \endcode
*/

/*!
    \property QtSignalTransition::object

    \brief the sender object that this signal transition is associated with
*/

/*!
    \property QtSignalTransition::signal

    \brief the signal that this signal transition is associated with
*/

QtSignalTransitionPrivate::QtSignalTransitionPrivate()
{
    sender = 0;
    signalIndex = -1;
}

QtSignalTransitionPrivate *QtSignalTransitionPrivate::get(QtSignalTransition *q)
{
    return q->d_func();
}

void QtSignalTransitionPrivate::invalidate()
{
    Q_Q(QtSignalTransition);
    if (signalIndex != -1) {
        QtState *source = sourceState();
        QtStatePrivate *source_d = QtStatePrivate::get(source);
        QtStateMachinePrivate *mach = QtStateMachinePrivate::get(source_d->machine());
        if (mach) {
            mach->unregisterSignalTransition(q);
            if (mach->configuration.contains(source))
                mach->registerSignalTransition(q);
        }
    }
}

/*!
  Constructs a new signal transition with the given \a sourceState.
*/
QtSignalTransition::QtSignalTransition(QtState *sourceState)
    : QtTransition(*new QtSignalTransitionPrivate, sourceState)
{
}

/*!
  Constructs a new signal transition associated with the given \a signal of
  the given \a sender, and with the given \a sourceState.
*/
QtSignalTransition::QtSignalTransition(QObject *sender, const char *signal,
                                     QtState *sourceState)
    : QtTransition(*new QtSignalTransitionPrivate, sourceState)
{
    Q_D(QtSignalTransition);
    d->sender = sender;
    d->signal = signal;
}

/*!
  Constructs a new signal transition associated with the given \a signal of
  the given \a sender. The transition has the given \a targets and \a
  sourceState.
*/
QtSignalTransition::QtSignalTransition(QObject *sender, const char *signal,
                                     const QList<QtAbstractState*> &targets,
                                     QtState *sourceState)
    : QtTransition(*new QtSignalTransitionPrivate, targets, sourceState)
{
    Q_D(QtSignalTransition);
    d->sender = sender;
    d->signal = signal;
}

/*!
  Destroys this signal transition.
*/
QtSignalTransition::~QtSignalTransition()
{
}

/*!
  Returns the sender object associated with this signal transition.
*/
QObject *QtSignalTransition::senderObject() const
{
    Q_D(const QtSignalTransition);
    return d->sender;
}

/*!
  Sets the \a sender object associated with this signal transition.
*/
void QtSignalTransition::setSenderObject(QObject *sender)
{
    Q_D(QtSignalTransition);
    if (sender == d->sender)
        return;
    d->sender = sender;
    d->invalidate();
}

/*!
  Returns the signal associated with this signal transition.
*/
QByteArray QtSignalTransition::signal() const
{
    Q_D(const QtSignalTransition);
    return d->signal;
}

/*!
  Sets the \a signal associated with this signal transition.
*/
void QtSignalTransition::setSignal(const QByteArray &signal)
{
    Q_D(QtSignalTransition);
    if (signal == d->signal)
        return;
    d->signal = signal;
    d->invalidate();
}

/*!
  \reimp

  The \a event is a QtSignalEvent object.  The default implementation returns
  true if the event's sender and signal index match this transition, and
  returns false otherwise.
*/
bool QtSignalTransition::eventTest(QEvent *event) const
{
    Q_D(const QtSignalTransition);
#ifndef QT_STATEMACHINE_SOLUTION
    if (event->type() == QEvent::Signal) {
#else
    if (event->type() == QEvent::Type(QEvent::User-1)) {
#endif
        if (d->signalIndex == -1)
            return false;
        QtSignalEvent *se = static_cast<QtSignalEvent*>(event);
        return (se->sender() == d->sender)
            && (se->signalIndex() == d->signalIndex);
    }
    return false;
}

/*!
  \reimp
*/
bool QtSignalTransition::event(QEvent *e)
{
    return QtTransition::event(e);
}

QT_END_NAMESPACE
