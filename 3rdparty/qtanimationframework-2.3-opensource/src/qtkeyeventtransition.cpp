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



#include "qtkeyeventtransition.h"
#include "qtbasickeyeventtransition_p.h"

#if defined(QT_EXPERIMENTAL_SOLUTION)
# include "qteventtransition_p.h"
#else
# include <private/qeventtransition_p.h>
#endif

QT_BEGIN_NAMESPACE

/*!
  \class QtKeyEventTransition

  \brief The QtKeyEventTransition class provides a transition for key events.

  
  

  QtKeyEventTransition is part of \l{The State Machine Framework}.

  \sa QtState::addTransition()
*/

/*!
    \property QtKeyEventTransition::key

    \brief the key that this key event transition is associated with
*/

class QtKeyEventTransitionPrivate : public QtEventTransitionPrivate
{
    Q_DECLARE_PUBLIC(QtKeyEventTransition)
public:
    QtKeyEventTransitionPrivate() {}

    QtBasicKeyEventTransition *transition;
};

/*!
  Constructs a new key event transition with the given \a sourceState.
*/
QtKeyEventTransition::QtKeyEventTransition(QtState *sourceState)
    : QtEventTransition(*new QtKeyEventTransitionPrivate, sourceState)
{
    Q_D(QtKeyEventTransition);
    d->transition = new QtBasicKeyEventTransition();
}

/*!
  Constructs a new key event transition for events of the given \a type for
  the given \a object, with the given \a key and \a sourceState.
*/
QtKeyEventTransition::QtKeyEventTransition(QObject *object, QEvent::Type type,
                                         int key, QtState *sourceState)
    : QtEventTransition(*new QtKeyEventTransitionPrivate, object, type, sourceState)
{
    Q_D(QtKeyEventTransition);
    d->transition = new QtBasicKeyEventTransition(type, key);
}

/*!
  Constructs a new key event transition for events of the given \a type for
  the given \a object, with the given \a key, \a targets and \a sourceState.
*/
QtKeyEventTransition::QtKeyEventTransition(QObject *object, QEvent::Type type,
                                         int key, const QList<QtAbstractState*> &targets,
                                         QtState *sourceState)
    : QtEventTransition(*new QtKeyEventTransitionPrivate, object, type, targets, sourceState)
{
    Q_D(QtKeyEventTransition);
    d->transition = new QtBasicKeyEventTransition(type, key);
}

/*!
  Destroys this key event transition.
*/
QtKeyEventTransition::~QtKeyEventTransition()
{
    Q_D(QtKeyEventTransition);
    delete d->transition;
}

/*!
  Returns the key that this key event transition checks for.
*/
int QtKeyEventTransition::key() const
{
    Q_D(const QtKeyEventTransition);
    return d->transition->key();
}

/*!
  Sets the key that this key event transition will check for.
*/
void QtKeyEventTransition::setKey(int key)
{
    Q_D(QtKeyEventTransition);
    d->transition->setKey(key);
}

/*!
  Returns the keyboard modifiers that this key event transition checks for.
*/
Qt::KeyboardModifiers QtKeyEventTransition::modifiers() const
{
    Q_D(const QtKeyEventTransition);
    return d->transition->modifiers();
}

/*!
  Sets the keyboard \a modifiers that this key event transition will check
  for.
*/
void QtKeyEventTransition::setModifiers(Qt::KeyboardModifiers modifiers)
{
    Q_D(QtKeyEventTransition);
    d->transition->setModifiers(modifiers);
}

/*!
  \reimp
*/
bool QtKeyEventTransition::testEventCondition(QEvent *event) const
{
    Q_D(const QtKeyEventTransition);
    d->transition->setEventType(event->type());
    return QtAbstractTransitionPrivate::get(d->transition)->callEventTest(event);
}

/*!
  \reimp
*/
bool QtKeyEventTransition::eventTest(QEvent *event) const
{
    return QtEventTransition::eventTest(event);
}

/*!
  \reimp
*/
void QtKeyEventTransition::onTransition()
{
    QtEventTransition::onTransition();
}

QT_END_NAMESPACE
