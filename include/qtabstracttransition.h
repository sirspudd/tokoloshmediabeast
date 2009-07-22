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



#ifndef QABSTRACTTRANSITION_H
#define QABSTRACTTRANSITION_H

#include <QtCore/qobject.h>

#include <QtCore/qlist.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Core)

class QEvent;
class QtAbstractState;
class QtState;

#ifndef QT_NO_ANIMATION
class QtAbstractAnimation;
#endif

class QtAbstractTransitionPrivate;
class Q_ANIMATION_EXPORT QtAbstractTransition : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QtState* source READ sourceState)
    Q_PROPERTY(QtAbstractState* target READ targetState WRITE setTargetState)
    Q_PROPERTY(QList<QtAbstractState*> targets READ targetStates WRITE setTargetStates)
public:
    QtAbstractTransition(QtState *sourceState = 0);
    QtAbstractTransition(const QList<QtAbstractState*> &targets, QtState *sourceState = 0);
    virtual ~QtAbstractTransition();

    QtState *sourceState() const;
    QtAbstractState *targetState() const;
    void setTargetState(QtAbstractState* target);
    QList<QtAbstractState*> targetStates() const;
    void setTargetStates(const QList<QtAbstractState*> &targets);

#ifndef QT_NO_ANIMATION
    void addAnimation(QtAbstractAnimation *animation);
    void removeAnimation(QtAbstractAnimation *animation);
    QList<QtAbstractAnimation*> animations() const;
#endif

protected:
    virtual bool eventTest(QEvent *event) const = 0;

    virtual void onTransition() = 0;

    bool event(QEvent *e);

protected:
#ifdef QT_STATEMACHINE_SOLUTION
    QtAbstractTransitionPrivate *d_ptr;
#endif
    QtAbstractTransition(QtAbstractTransitionPrivate &dd, QtState *parent);
    QtAbstractTransition(QtAbstractTransitionPrivate &dd,
                        const QList<QtAbstractState*> &targets, QtState *parent);

private:
    Q_DISABLE_COPY(QtAbstractTransition)
    Q_DECLARE_PRIVATE(QtAbstractTransition)
};

QT_END_NAMESPACE

QT_END_HEADER

#endif
