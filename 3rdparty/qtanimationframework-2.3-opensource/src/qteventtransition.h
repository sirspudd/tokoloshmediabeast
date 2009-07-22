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



#ifndef QEVENTTRANSITION_H
#define QEVENTTRANSITION_H

#include "qttransition.h"
#include <QtCore/qcoreevent.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Core)

class QtEventTransitionPrivate;
class Q_ANIMATION_EXPORT QtEventTransition : public QtTransition
{
    Q_OBJECT
    Q_PROPERTY(QObject* object READ eventSource WRITE setEventSource)
#ifndef QT_STATEMACHINE_SOLUTION
    Q_PROPERTY(QEvent::Type eventType READ eventType WRITE setEventType)
#endif
public:
    QtEventTransition(QtState *sourceState = 0);
    QtEventTransition(QObject *object, QEvent::Type type, QtState *sourceState = 0);
    QtEventTransition(QObject *object, QEvent::Type type,
                     const QList<QtAbstractState*> &targets, QtState *sourceState = 0);
    ~QtEventTransition();

    QObject *eventSource() const;
    void setEventSource(QObject *object);

    QEvent::Type eventType() const;
    void setEventType(QEvent::Type type);

protected:
    virtual bool testEventCondition(QEvent *event) const; // ### name

    bool eventTest(QEvent *event) const;

    bool event(QEvent *e);

protected:
    QtEventTransition(QtEventTransitionPrivate &dd, QtState *parent);
    QtEventTransition(QtEventTransitionPrivate &dd, QObject *object,
                     QEvent::Type type, QtState *parent);
    QtEventTransition(QtEventTransitionPrivate &dd, QObject *object,
                     QEvent::Type type, const QList<QtAbstractState*> &targets,
                     QtState *parent);

private:
    Q_DISABLE_COPY(QtEventTransition)
    Q_DECLARE_PRIVATE(QtEventTransition)
};

QT_END_NAMESPACE

QT_END_HEADER

#endif
