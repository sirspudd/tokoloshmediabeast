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



#ifndef QABSTRACTSTATE_H
#define QABSTRACTSTATE_H

#include <QtCore/qobject.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Core)

class QtState;

class QtAbstractStatePrivate;
class Q_ANIMATION_EXPORT QtAbstractState : public QObject
{
    Q_OBJECT
    Q_ENUMS(RestorePolicy)
    Q_PROPERTY(RestorePolicy restorePolicy READ restorePolicy WRITE setRestorePolicy)
public:
    enum RestorePolicy {
        GlobalRestorePolicy,
        DoNotRestoreProperties,
        RestoreProperties
    };

    ~QtAbstractState();

    QtState *parentState() const;

    void assignProperty(QObject *object, const char *name,
                        const QVariant &value);

    void setRestorePolicy(RestorePolicy restorePolicy);
    RestorePolicy restorePolicy() const;

protected:
    QtAbstractState(QtState *parent = 0);

    virtual void onEntry() = 0;
    virtual void onExit() = 0;

    bool event(QEvent *e);

protected:
#ifdef QT_STATEMACHINE_SOLUTION
     QtAbstractStatePrivate *d_ptr;
#endif
    QtAbstractState(QtAbstractStatePrivate &dd, QtState *parent);

private:
    Q_DISABLE_COPY(QtAbstractState)
    Q_DECLARE_PRIVATE(QtAbstractState)
};

QT_END_NAMESPACE

QT_END_HEADER

#endif
