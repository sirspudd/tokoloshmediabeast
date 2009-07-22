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



#include "qtbasickeyeventtransition_p.h"
#include <QtGui/qevent.h>
#include <qdebug.h>

#if defined(QT_EXPERIMENTAL_SOLUTION)
# include "qtabstracttransition_p.h"
#else
# include <private/qabstracttransition_p.h>
#endif


QT_BEGIN_NAMESPACE

/*!
  \internal
  \class QtBasicKeyEventTransition

  \brief The QtBasicKeyEventTransition class provides a transition for Qt key events.
*/

class QtBasicKeyEventTransitionPrivate : public QtAbstractTransitionPrivate
{
    Q_DECLARE_PUBLIC(QtBasicKeyEventTransition)
public:
    QtBasicKeyEventTransitionPrivate();

    static QtBasicKeyEventTransitionPrivate *get(QtBasicKeyEventTransition *q);

    QEvent::Type eventType;
    int key;
    Qt::KeyboardModifiers modifiers;
};

QtBasicKeyEventTransitionPrivate::QtBasicKeyEventTransitionPrivate()
{
    eventType = QEvent::None;
    key = 0;
    modifiers = Qt::NoModifier;
}

QtBasicKeyEventTransitionPrivate *QtBasicKeyEventTransitionPrivate::get(QtBasicKeyEventTransition *q)
{
    return q->d_func();
}

/*!
  Constructs a new key event transition with the given \a sourceState.
*/
QtBasicKeyEventTransition::QtBasicKeyEventTransition(QtState *sourceState)
    : QtAbstractTransition(*new QtBasicKeyEventTransitionPrivate, sourceState)
{
}

/*!
  Constructs a new event transition for events of the given \a type for the
  given \a key, with the given \a sourceState.
*/
QtBasicKeyEventTransition::QtBasicKeyEventTransition(QEvent::Type type, int key,
                                                   QtState *sourceState)
    : QtAbstractTransition(*new QtBasicKeyEventTransitionPrivate, sourceState)
{
    Q_D(QtBasicKeyEventTransition);
    d->eventType = type;
    d->key = key;
}

/*!
  Constructs a new event transition for events of the given \a type for the
  given \a key, with the given \a modifiers and \a sourceState.
*/
QtBasicKeyEventTransition::QtBasicKeyEventTransition(QEvent::Type type, int key,
                                                   Qt::KeyboardModifiers modifiers,
                                                   QtState *sourceState)
    : QtAbstractTransition(*new QtBasicKeyEventTransitionPrivate, sourceState)
{
    Q_D(QtBasicKeyEventTransition);
    d->eventType = type;
    d->key = key;
    d->modifiers = modifiers;
}

/*!
  Destroys this event transition.
*/
QtBasicKeyEventTransition::~QtBasicKeyEventTransition()
{
}

/*!
  Returns the event type that this key event transition is associated with.
*/
QEvent::Type QtBasicKeyEventTransition::eventType() const
{
    Q_D(const QtBasicKeyEventTransition);
    return d->eventType;
}

/*!
  Sets the event \a type that this key event transition is associated with.
*/
void QtBasicKeyEventTransition::setEventType(QEvent::Type type)
{
    Q_D(QtBasicKeyEventTransition);
    d->eventType = type;
}

/*!
  Returns the key that this key event transition checks for.
*/
int QtBasicKeyEventTransition::key() const
{
    Q_D(const QtBasicKeyEventTransition);
    return d->key;
}

/*!
  Sets the key that this key event transition will check for.
*/
void QtBasicKeyEventTransition::setKey(int key)
{
    Q_D(QtBasicKeyEventTransition);
    d->key = key;
}

/*!
  Returns the keyboard modifiers that this key event transition checks for.
*/
Qt::KeyboardModifiers QtBasicKeyEventTransition::modifiers() const
{
    Q_D(const QtBasicKeyEventTransition);
    return d->modifiers;
}

/*!
  Sets the keyboard modifiers that this key event transition will check for.
*/
void QtBasicKeyEventTransition::setModifiers(Qt::KeyboardModifiers modifiers)
{
    Q_D(QtBasicKeyEventTransition);
    d->modifiers = modifiers;
}

/*!
  \reimp
*/
bool QtBasicKeyEventTransition::eventTest(QEvent *event) const
{
    Q_D(const QtBasicKeyEventTransition);
    if (event->type() == d->eventType) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(event);
        return (ke->key() == d->key) && (ke->modifiers() == d->modifiers);
    }
    return false;
}

/*!
  \reimp
*/
void QtBasicKeyEventTransition::onTransition()
{
}

QT_END_NAMESPACE
