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



/*!
    \class QtAbstractAnimation
    
    \brief The QtAbstractAnimation class provides an abstract base class for animations.
    
    \preliminary

    This class is part of \l{The Animation Framework}. It serves as a base class
    for standard animations and groups, with functions for shared
    functionality, and it also makes it easy for you to define custom
    animations that plug into the rest of the animation framework.

    If you want to create an animation, you should look at the two subclasses,
    QtVariantAnimation and QtAnimationGroup, instead.

    QtAbstractAnimation provides an interface for the current time and
    duration, the loop count, and the state of an animation. These properties
    define the base functionality common to all animations in Qt. The virtual
    duration() function returns the local duration of the animation; i.e., for
    how long the animation should update the current time before
    looping. Subclasses can implement this function differently; for example,
    QtVariantAnimation returns the duration of a simple animated property, whereas
    QtAnimationGroup returns the duration of a set or sequence of
    animations. You can also set a loop count by calling setLoopCount(); a
    loop count of 2 will let the animation run twice (the default value is
    1).

    Like QTimeLine, QtAbstractAnimation also provides an interface for starting
    and stopping an animation, and for tracking its progress. You can call the
    start() slot to start the animation. When the animation starts, the
    stateChanged() signal is emitted, and state() returns Running. If you call the
    stop() slot, the stateChanged() signal is emitted, and state() returns
    Stopped. If you call the pause() slot, the stateChanged() signal is emitted
    and state() returns Paused. If the animation reaches the end, the finished()
    signal is emitted. You can check the current state by calling state().

    QtAbstractAnimation provides two functions that are pure virtual, and must
    be reimplemented in a subclass: duration(), and updateCurrentTime(). The
    duration() function lets you report a duration for the animation (a return
    value of -1 signals that the animation runs forever until explicitly
    stopped). The current time is delivered by the framework through calls to
    updateCurrentTime(). By reimplementing this function, you can track the
    animation progress and update your target objects accordingly. By
    reimplementing updateState(), you can track the animation's state
    changes, which is particularily useful for animations that are not driven
    by time.

    \sa QtVariantAnimation, QtAnimationGroup, {The Animation Framework}
*/

/*!
    \enum QtAbstractAnimation::DeletionPolicy

    \value KeepWhenStopped The animation will not be deleted when stopped.
    \value DeleteWhenStopped The animation will be automatically deleted when
    stopped.
*/

/*!
    \fn QtAbstractAnimation::finished()

    QtAbstractAnimation emits this signal after the animation has stopped and
    has reached the end.

    This signal is emitted after stateChanged().

    \sa stateChanged()
*/

/*!
    \fn QtAbstractAnimation::stateChanged(QtAbstractAnimation::State oldState, QtAbstractAnimation::State newState)

    QtAbstractAnimation emits this signal whenever the state of the animation has
    changed from \a oldState to \a newState. This signal is emitted after the virtual
    updateState() function is called.

    \sa updateState()
*/

/*!
    \fn QtAbstractAnimation::currentLoopChanged(int currentLoop)

    QtAbstractAnimation emits this signal whenever the current loop
    changes. \a currentLoop is the current loop.

    \sa currentLoop(), loopCount()
*/

/*!
    \fn QtAbstractAnimation::directionChanged(QtAbstractAnimation::Direction newDirection);

    QtAbstractAnimation emits this signal whenever the direction has been
    changed. \a newDirection is the new direction.

    \sa direction
*/

#ifndef QT_NO_ANIMATION

#include "qtabstractanimation.h"
#include "qtanimationgroup.h"
#include <QtCore/qdebug.h>

#include "qtabstractanimation_p.h"

#include <QtCore/qmath.h>
#include <QtCore/qthreadstorage.h>
#include <QtCore/qcoreevent.h>
#include <QtCore/qpointer.h>

#define TIMER_INTERVAL 16

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(QThreadStorage<QUnifiedTimer *>, unifiedTimer);

QUnifiedTimer::QUnifiedTimer() : QObject(), lastTick(0)
{
}

QUnifiedTimer *QUnifiedTimer::instance()
{
    QUnifiedTimer *inst;
    if (!unifiedTimer()->hasLocalData()) {
        inst = new QUnifiedTimer;
        unifiedTimer()->setLocalData(inst);
    } else {
        inst = unifiedTimer()->localData();
    }
    return inst;
}

void QUnifiedTimer::updateRecentlyStartedAnimations()
{
    if (animationsToStart.isEmpty())
        return;

    animations += animationsToStart;
    updateTimer(); //we make sure we start the timer there

    animationsToStart.clear();
}

void QUnifiedTimer::timerEvent(QTimerEvent *event)
{
    //this is simply the time we last received a tick
    int oldLastTick = lastTick;
    if (time.isValid())
        lastTick = time.elapsed();

    //we transfer the waiting animations into the "really running" state
    updateRecentlyStartedAnimations();

    if (event->timerId() == startStopAnimationTimer.timerId()) {
        startStopAnimationTimer.stop();
        if (animations.isEmpty()) {
            animationTimer.stop();
            time = QTime();
        } else {
            animationTimer.start(TIMER_INTERVAL, this);
            lastTick = 0;
            time.start();
        }
    } else if (event->timerId() == animationTimer.timerId()) {
        const int delta = lastTick - oldLastTick;
        for (int i = 0; i < animations.count(); ++i) {
            QtAbstractAnimation *animation = animations.at(i);
            int elapsed = QtAbstractAnimationPrivate::get(animation)->totalCurrentTime
                + (animation->direction() == QtAbstractAnimation::Forward ? delta : -delta);
            animation->setCurrentTime(elapsed);
        }
    }
}

void QUnifiedTimer::updateTimer()
{
    //we delay the call to start and stop for the animation timer so that if you
    //stop and start animations in batch you don't stop/start the timer too often.
    if (!startStopAnimationTimer.isActive())
        startStopAnimationTimer.start(0, this); // we delay the actual start of the animation
}

void QUnifiedTimer::registerAnimation(QtAbstractAnimation *animation)
{
    if (animations.contains(animation) ||animationsToStart.contains(animation))
        return;
    animationsToStart << animation;
    updateTimer();
}

void QUnifiedTimer::unregisterAnimation(QtAbstractAnimation *animation)
{
    animations.removeAll(animation);
    animationsToStart.removeAll(animation);
    updateTimer();
}


void QtAbstractAnimationPrivate::setState(QtAbstractAnimation::State newState)
{
    Q_Q(QtAbstractAnimation);
    if (state == newState)
        return;

    QtAbstractAnimation::State oldState = state;
    int oldCurrentTime = currentTime;
    int oldCurrentLoop = currentLoop;
    QtAbstractAnimation::Direction oldDirection = direction;

    state = newState;

    QPointer<QtAbstractAnimation> guard(q);

    guard->updateState(oldState, newState);

    //this is to be safe if updateState changes the state
    if (state == oldState)
        return;

    // Notify state change
    if (guard)
        emit guard->stateChanged(oldState, newState);

    // Enter running state.
    switch (state)
    {
    case QtAbstractAnimation::Paused:
    case QtAbstractAnimation::Running:
        {
            // Rewind
            if (oldState == QtAbstractAnimation::Stopped) {
                if (guard) {
                    if (direction == QtAbstractAnimation::Forward)
                        q->setCurrentTime(0);
                    else
                        q->setCurrentTime(loopCount == -1 ? q->duration() : q->totalDuration());
                }

                // Check if the setCurrentTime() function called stop().
                // This can happen for a 0-duration animation
                if (state == QtAbstractAnimation::Stopped)
                    return;
            }

            // Register timer if our parent is not running.
            if (state == QtAbstractAnimation::Running && guard) {
                if (!group || group->state() == QtAbstractAnimation::Stopped) {
                    QUnifiedTimer::instance()->registerAnimation(q);
                }
            } else {
                //new state is paused
                QUnifiedTimer::instance()->unregisterAnimation(q);
            }
        }
        break;
    case QtAbstractAnimation::Stopped:
        // Leave running state.
        int dura = q->duration();
        if (deleteWhenStopped && guard)
            q->deleteLater();

        QUnifiedTimer::instance()->unregisterAnimation(q);

        if (dura == -1 || loopCount < 0
            || (oldDirection == QtAbstractAnimation::Forward && (oldCurrentTime * (oldCurrentLoop + 1)) == (dura * loopCount))
            || (oldDirection == QtAbstractAnimation::Backward && oldCurrentTime == 0)) {
                if (guard)
                    emit q->finished();
        }
        break;
    }

}

/*!
    Constructs the QtAbstractAnimation base class, and passes \a parent to
    QObject's constructor.

    \sa QtVariantAnimation, QtAnimationGroup
*/
#ifdef QT_EXPERIMENTAL_SOLUTION
QtAbstractAnimation::QtAbstractAnimation(QObject *parent)
    : d_ptr(new QtAbstractAnimationPrivate)
{
    // Allow auto-add on reparent
    setParent(parent);
    d_ptr->q_ptr = this;
}
#else
QtAbstractAnimation::QtAbstractAnimation(QObject *parent)
    : QObject(*new QtAbstractAnimationPrivate, 0)
{
    // Allow auto-add on reparent
    setParent(parent);
}
#endif

/*!
    \internal
*/
#ifdef QT_EXPERIMENTAL_SOLUTION
QtAbstractAnimation::QtAbstractAnimation(QtAbstractAnimationPrivate &dd, QObject *parent)
    : d_ptr(&dd)
{
    // Allow auto-add on reparent
   setParent(parent);
   d_ptr->q_ptr = this;
}
#else
QtAbstractAnimation::QtAbstractAnimation(QtAbstractAnimationPrivate &dd, QObject *parent)
    : QObject(dd, 0)
{
    // Allow auto-add on reparent
   setParent(parent);
}
#endif

/*!
    Stops the animation if it's running, then destroys the
    QtAbstractAnimation. If the animation is part of a QtAnimationGroup, it is
    automatically removed before it's destroyed.
*/
QtAbstractAnimation::~QtAbstractAnimation()
{
    Q_D(QtAbstractAnimation);
    //we can't call stop here. Otherwise we get pure virtual calls
    if (d->state != Stopped) {
        QtAbstractAnimation::State oldState = d->state;
        d->state = Stopped;
        emit stateChanged(oldState, d->state);
        QUnifiedTimer::instance()->unregisterAnimation(this);
    }
}

/*!
    \property QtAbstractAnimation::state
    \brief state of the animation.

    This property describes the current state of the animation. When the
    animation state changes, QtAbstractAnimation emits the stateChanged()
    signal.
*/
QtAbstractAnimation::State QtAbstractAnimation::state() const
{
    Q_D(const QtAbstractAnimation);
    return d->state;
}

/*!
    If this animation is part of a QtAnimationGroup, this function returns a
    pointer to the group; otherwise, it returns 0.

    \sa QtAnimationGroup::addAnimation()
*/
QtAnimationGroup *QtAbstractAnimation::group() const
{
    Q_D(const QtAbstractAnimation);
    return d->group;
}

/*!
    \enum QtAbstractAnimation::State

    This enum describes the state of the animation.

    \value Stopped The animation is not running. This is the initial state
    of QtAbstractAnimation, and the state QtAbstractAnimation reenters when finished. The current
    time remain unchanged until either setCurrentTime() is
    called, or the animation is started by calling start().

    \value Paused The animation is paused (i.e., temporarily
    suspended). Calling resume() will resume animation activity.

    \value Running The animation is running. While control is in the event
    loop, QtAbstractAnimation will update its current time at regular intervals,
    calling updateCurrentTime() when appropriate.

    \sa state(), stateChanged()
*/

/*!
    \enum QtAbstractAnimation::Direction

    This enum describes the direction of the animation when in \l Running state.

    \value Forward The current time of the animation increases with time (i.e.,
    moves from 0 and towards the end / duration).

    \value Backward The current time of the animation decreases with time (i.e.,
    moves from the end / duration and towards 0).

    \sa direction
*/

/*!
    \property QtAbstractAnimation::direction
    \brief the direction of the animation when it is in \l Running
    state.

    This direction indicates whether the time moves from 0 towards the
    animation duration, or from the value of the duration and towards 0 after
    start() has been called.

    By default, this property is set to \l Forward.
*/
QtAbstractAnimation::Direction QtAbstractAnimation::direction() const
{
    Q_D(const QtAbstractAnimation);
    return d->direction;
}
void QtAbstractAnimation::setDirection(Direction direction)
{
    Q_D(QtAbstractAnimation);
    if (d->direction == direction)
        return;

    d->direction = direction;
    if (state() == Stopped) {
        if (direction == Backward) {
            d->currentTime = duration();
            d->currentLoop = d->loopCount - 1;
        } else {
            d->currentTime = 0;
            d->currentLoop = 0;
        }
    }
    updateDirection(direction);
    emit directionChanged(direction);
}

/*!
    \property QtAbstractAnimation::duration
    \brief the duration of the animation.

    If the duration is -1, it means that the duration is undefined.
    In this case, loopCount is ignored.
*/

/*!
    \property QtAbstractAnimation::loopCount
    \brief the loop count of the animation

    This property describes the loop count of the animation as an integer.
    By default this value is 1, indicating that the animation
    should run once only, and then stop. By changing it you can let the
    animation loop several times. With a value of 0, the animation will not
    run at all, and with a value of -1, the animation will loop forever
    until stopped.
    It is not supported to have loop on an animation that has an undefined
    duration. It will only run once.
*/
int QtAbstractAnimation::loopCount() const
{
    Q_D(const QtAbstractAnimation);
    return d->loopCount;
}
void QtAbstractAnimation::setLoopCount(int loopCount)
{
    Q_D(QtAbstractAnimation);
    d->loopCount = loopCount;
}

/*!
    \property QtAbstractAnimation::currentLoop
    \brief the current loop of the animation

    This property describes the current loop of the animation. By default,
    the animation's loop count is 1, and so the current loop will
    always be 0. If the loop count is 2 and the animation runs past its
    duration, it will automatically rewind and restart at current time 0, and
    current loop 1, and so on.

    When the current loop changes, QtAbstractAnimation emits the
    currentLoopChanged() signal.
*/
int QtAbstractAnimation::currentLoop() const
{
    Q_D(const QtAbstractAnimation);
    return d->currentLoop;
}

/*!
    \fn virtual int QtAbstractAnimation::duration() const = 0

    This pure virtual function returns the duration of the animation, and
    defines for how long QtAbstractAnimation should update the current
    time. This duration is local, and does not include the loop count.

    A return value of -1 indicates that the animation has no defined duration;
    the animation should run forever until stopped. This is useful for
    animations that are not time driven, or where you cannot easily predict
    its duration (e.g., event driven audio playback in a game).

    If the animation is a parallel QtAnimationGroup, the duration will be the longest
    duration of all its animations. If the animation is a sequential QtAnimationGroup,
    the duration will be the sum of the duration of all its animations.
    \sa loopCount
*/

/*!
    Returns the total and effective duration of the animation, including the
    loop count.

    \sa duration(), currentTime
*/
int QtAbstractAnimation::totalDuration() const
{
    Q_D(const QtAbstractAnimation);
    if (d->loopCount < 0)
        return -1;
    int dura = duration();
    if (dura == -1)
        return -1;
    return dura * d->loopCount;
}

/*!
    \property QtAbstractAnimation::currentTime
    \brief the current time and progress of the animation

    This property describes the animation's current time. You can change the
    current time by calling setCurrentTime, or you can call start() and let
    the animation run, setting the current time automatically as the animation
    progresses.

    The animation's current time starts at 0, and ends at duration(). If the
    animation's loopCount is larger than 1, the current time will rewind and
    start at 0 again for the consecutive loops. If the animation has a pause.
    currentTime will also include the duration of the pause.

    \sa loopCount
 */
int QtAbstractAnimation::currentTime() const
{
    Q_D(const QtAbstractAnimation);
    return d->currentTime;
}
void QtAbstractAnimation::setCurrentTime(int msecs)
{
    Q_D(QtAbstractAnimation);
    msecs = qMax(msecs, 0);

    // Calculate new time and loop.
    int dura = duration();
    int totalDura = (d->loopCount < 0 || dura == -1) ? -1 : dura * d->loopCount;
    if (totalDura != -1)
        msecs = qMin(totalDura, msecs);
    d->totalCurrentTime = msecs;

    // Update new values.
    int oldLoop = d->currentLoop;
    d->currentLoop = ((dura <= 0) ? 0 : (msecs / dura));
    if (d->currentLoop == d->loopCount) {
        //we're at the end
        d->currentTime = qMax(0, dura);
        d->currentLoop = qMax(0, d->loopCount - 1);
    } else {
        if (d->direction == Forward) {
            d->currentTime = (dura <= 0) ? msecs : (msecs % dura);
        } else {
            d->currentTime = (dura <= 0) ? msecs : ((msecs - 1) % dura) + 1;
            if (d->currentTime == dura)
                --d->currentLoop;
        }
    }

    updateCurrentTime(msecs);
    if (d->currentLoop != oldLoop)
        emit currentLoopChanged(d->currentLoop);

    // All animations are responsible for stopping the animation when their
    // own end state is reached; in this case the animation is time driven,
    // and has reached the end.
    if ((d->direction == Forward && d->totalCurrentTime == totalDura)
        || (d->direction == Backward && d->totalCurrentTime == 0)) {
        stop();
    }
}

/*!
    Starts the animation. The \a policy argument says whether or not the
    animation should be deleted when it's done. When the animation starts, the
    stateChanged() signal is emitted, and state() returns Running. When control
    reaches the event loop, the animation will run by itself, periodically
    calling updateCurrentTime() as the animation progresses.

    If the animation is currently stopped or has already reached the end,
    calling start() will rewind the animation and start again from the beginning.
    When the animation reaches the end, the animation will either stop, or
    if the loop level is more than 1, it will rewind and continue from the beginning.

    If the animation is already running, this function does nothing.

    \sa stop(), state()
*/
void QtAbstractAnimation::start(DeletionPolicy policy)
{
    Q_D(QtAbstractAnimation);
    if (d->state == Running)
        return;
    d->setState(Running);
    d->deleteWhenStopped = policy;
}

/*!
    Stops the animation. When the animation is stopped, it emits the stateChanged()
    signal, and state() returns Stopped. The current time is not changed.

    If the animation stops by itself after reaching the end (i.e.,
    currentTime() == duration() and currentLoop() > loopCount() - 1), the
    finished() signal is emitted.

    \sa start(), state()
 */
void QtAbstractAnimation::stop()
{
    Q_D(QtAbstractAnimation);

    d->setState(Stopped);
}

/*!
    Pauses the animation. When the animation is paused, state() returns Paused.
    The currenttime will remain unchanged until resume() or start() is called.
    If you want to continue from the current time, call resume().


    \sa start(), state(), resume()
 */
void QtAbstractAnimation::pause()
{
    Q_D(QtAbstractAnimation);
    if (d->state == Stopped) {
        qWarning("QtAbstractAnimation::pause: Cannot pause a stopped animation");
        return;
    }

    d->setState(Paused);
}

/*!
    Resumes the animation after it was paused. When the animation is resumed,
    it emits the resumed() and stateChanged() signals. The currenttime is not
    changed.

    \sa start(), pause(), state()
 */
void QtAbstractAnimation::resume()
{
    Q_D(QtAbstractAnimation);
    if (d->state != Paused) {
        qWarning("QtAbstractAnimation::resume: "
                 "Cannot resume an animation that is not paused");
        return;
    }

    d->setState(Running);
}

/*!
    \reimp
*/
bool QtAbstractAnimation::event(QEvent *event)
{
    return QObject::event(event);
}

/*!
    \fn virtual void QtAbstractAnimation::updateCurrentTime(int msecs) = 0;

    This pure virtual function is called every time the animation's current
    time changes. The \a msecs argument is the current time.

    \sa updateState()
*/

/*!
    This virtual function is called by QtAbstractAnimation when the state
    of the animation is changed from \a oldState to \a newState.

    \sa start(), stop(), pause(), resume()
*/
void QtAbstractAnimation::updateState(QtAbstractAnimation::State oldState,
                                     QtAbstractAnimation::State newState)
{
    Q_UNUSED(oldState);
    Q_UNUSED(newState);
}

/*!
    This virtual function is called by QtAbstractAnimation when the direction
    of the animation is changed. The \a direction argument is the new direction.

    \sa setDirection(), direction()
*/
void QtAbstractAnimation::updateDirection(QtAbstractAnimation::Direction direction)
{
    Q_UNUSED(direction);
}


QT_END_NAMESPACE

#include "moc_qtabstractanimation.cpp"

#endif //QT_NO_ANIMATION
