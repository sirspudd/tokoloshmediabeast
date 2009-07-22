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
    \class QtSequentialAnimationGroup
    \brief The QtSequentialAnimationGroup class provides a sequential group of animations.
    
    
    \preliminary

    The first animation in the group is started first, and when it finishes, the next animation
    is started, and so on. The animation group finishes when the last animation has finished.

    At each moment there is at most one animation that is active in the group, called currentAnimation.
    An empty group has no current animation.

    You can call addPause() or insertPause() to add a pause to a sequential animation group.
*/

#ifndef QT_NO_ANIMATION

#include "qtsequentialanimationgroup.h"
#include "qtsequentialanimationgroup_p.h"

#include "qtpauseanimation.h"

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE



bool QtSequentialAnimationGroupPrivate::atEnd() const
{
    // we try to detect if we're at the end of the group
    //this is true if the following conditions are true:
    // 1. we're in the last loop
    // 2. the direction is forward
    // 3. the current animation is the last one
    // 4. the current animation has reached its end
    const int animTotalCurrentTime = QtAbstractAnimationPrivate::get(currentAnimation)->totalCurrentTime;
    return (currentLoop == loopCount - 1
        && direction == QtAbstractAnimation::Forward
        && currentAnimation == animations.last()
        && animTotalCurrentTime == animationActualTotalDuration(currentAnimationIndex));
}

int QtSequentialAnimationGroupPrivate::animationActualTotalDuration(int index) const
{
    QtAbstractAnimation *anim = animations.at(index);
    int ret = anim->totalDuration();
    if (ret == -1 && actualDuration.size() > index)
        ret = actualDuration.at(index); //we can try the actual duration there
    return ret;
}

QtSequentialAnimationGroupPrivate::AnimationIndex QtSequentialAnimationGroupPrivate::indexForTime(int msecs) const
{
    Q_Q(const QtSequentialAnimationGroup);
    Q_ASSERT(!animations.isEmpty());

    AnimationIndex ret;
    int duration = 0;

    // in case duration is -1, currentLoop will always be 0
    ret.timeOffset = currentLoop * q->duration();

    for (int i = 0; i < animations.size(); ++i) {
        duration = animationActualTotalDuration(i);

        // 'animation' is the current animation if one of these reasons is true:
        // 1. it's duration is undefined
        // 2. it ends after msecs
        // 3. it is the last animation (this can happen in case there is at least 1 uncontrolled animation)
        // 4. it ends exactly in msecs and the direction is backwards
        if (duration == -1 || msecs < (ret.timeOffset + duration)
            || (msecs == (ret.timeOffset + duration) && direction == QtAbstractAnimation::Backward)) {
            ret.index = i;
            return ret;
        }

        // 'animation' has a non-null defined duration and is not the one at time 'msecs'.
        ret.timeOffset += duration;
    }

    // this can only happen when one of those conditions is true:
    // 1. the duration of the group is undefined and we passed its actual duration
    // 2. there are only 0-duration animations in the group
    ret.timeOffset -= duration;
    ret.index = animations.size() - 1;
    return ret;
}

void QtSequentialAnimationGroupPrivate::restart()
{
    // restarting the group by making the first/last animation the current one
    if (direction == QtAbstractAnimation::Forward) {
        lastLoop = 0;
        if (currentAnimationIndex == 0)
            activateCurrentAnimation();
        else
            setCurrentAnimation(0);
    } else { // direction == QtAbstractAnimation::Backward
        lastLoop = loopCount - 1;
        int index = animations.size() - 1;
        if (currentAnimationIndex == index)
            activateCurrentAnimation();
        else
            setCurrentAnimation(index);
    }
}

/*!
    \internal
    This manages advancing the execution of a group running forwards (time has gone forward),
    which is the same behaviour for rewinding the execution of a group running backwards
    (time has gone backward).
*/
void QtSequentialAnimationGroupPrivate::advanceForwards(const AnimationIndex &newAnimationIndex)
{
    if (lastLoop < currentLoop) {
        // we need to fast forward to the end
        for (int i = currentAnimationIndex; i < animations.size(); ++i) {
            QtAbstractAnimation *anim = animations.at(i);
            setCurrentAnimation(i, true);
            anim->setCurrentTime(animationActualTotalDuration(i));
        }
        // this will make sure the current animation is reset to the beginning
        if (animations.size() == 1)
            // we need to force activation because setCurrentAnimation will have no effect
            activateCurrentAnimation();
        else
            setCurrentAnimation(0, true);
    }

    // and now we need to fast forward from the current position to
    for (int i = currentAnimationIndex; i < newAnimationIndex.index; ++i) {     //### WRONG,
        QtAbstractAnimation *anim = animations.at(i);
        setCurrentAnimation(i, true);
        anim->setCurrentTime(animationActualTotalDuration(i));
    }
    // setting the new current animation will happen later
}

/*!
    \internal
    This manages rewinding the execution of a group running forwards (time has gone forward),
    which is the same behaviour for advancing the execution of a group running backwards
    (time has gone backward).
*/
void QtSequentialAnimationGroupPrivate::rewindForwards(const AnimationIndex &newAnimationIndex)
{
    if (lastLoop > currentLoop) {
        // we need to fast rewind to the beginning
        for (int i = currentAnimationIndex; i >= 0 ; --i) {
            QtAbstractAnimation *anim = animations.at(i);
            setCurrentAnimation(i, true);
            anim->setCurrentTime(0);
        }
        // this will make sure the current animation is reset to the end
        if (animations.size() == 1)
            // we need to force activation because setCurrentAnimation will have no effect
            activateCurrentAnimation();
        else
            setCurrentAnimation(animations.count() - 1, true);
    }

    // and now we need to fast rewind from the current position to
    for (int i = currentAnimationIndex; i > newAnimationIndex.index; --i) {
        QtAbstractAnimation *anim = animations.at(i);
        setCurrentAnimation(i, true);
        anim->setCurrentTime(0);
    }
    // setting the new current animation will happen later
}

/*!
    \fn QtSequentialAnimationGroup::currentAnimationChanged(QtAbstractAnimation *current)

    QtSequentialAnimationGroup emits this signal when currentAnimation
    has been changed. \a current is the current animation.

    \sa currentAnimation()
*/


/*!
    Constructs a QtSequentialAnimationGroup.
    \a parent is passed to QObject's constructor.
*/
QtSequentialAnimationGroup::QtSequentialAnimationGroup(QObject *parent)
    : QtAnimationGroup(*new QtSequentialAnimationGroupPrivate, parent)
{
}

/*!
    \internal
*/
QtSequentialAnimationGroup::QtSequentialAnimationGroup(QtSequentialAnimationGroupPrivate &dd,
                                                     QObject *parent)
    : QtAnimationGroup(dd, parent)
{
}

/*!
    Destroys the animation group. It will also destroy all its animations.
*/
QtSequentialAnimationGroup::~QtSequentialAnimationGroup()
{
}

/*!
    Adds a pause of \a msecs to this animation group.
    The pause is considered as a special type of animation, thus count() will be
    increased by one.
    \sa insertPauseAt(), QtAnimationGroup::addAnimation()
*/
QtPauseAnimation *QtSequentialAnimationGroup::addPause(int msecs)
{
    QtPauseAnimation *pause = new QtPauseAnimation(msecs);
    addAnimation(pause);
    return pause;
}

/*!
    Inserts a pause of \a msecs milliseconds at \a index in this animation
    group.

    \sa addPause(), QtAnimationGroup::insertAnimationAt()
*/
QtPauseAnimation *QtSequentialAnimationGroup::insertPauseAt(int index, int msecs)
{
    Q_D(const QtSequentialAnimationGroup);

    if (index < 0 || index > d->animations.size()) {
        qWarning("QtSequentialAnimationGroup::insertPauseAt: index is out of bounds");
        return 0;
    }

    QtPauseAnimation *pause = new QtPauseAnimation(msecs);
    insertAnimationAt(index, pause);
    return pause;
}


/*!
    \property QtSequentialAnimationGroup::currentAnimation
    Returns the animation in the current time.

    \sa currentAnimationChanged()
*/
QtAbstractAnimation *QtSequentialAnimationGroup::currentAnimation() const
{
    Q_D(const QtSequentialAnimationGroup);
    return d->currentAnimation;
}

/*!
    \reimp
*/
int QtSequentialAnimationGroup::duration() const
{
    Q_D(const QtSequentialAnimationGroup);
    int ret = 0;

    for (int i = 0; i < d->animations.size(); ++i) {
        QtAbstractAnimation *animation = d->animations.at(i);
        const int currentDuration = animation->totalDuration();
        if (currentDuration == -1)
            return -1; // Undetermined length

        ret += currentDuration;
    }

    return ret;
}

/*!
    \reimp
*/
void QtSequentialAnimationGroup::updateCurrentTime(int msecs)
{
    Q_D(QtSequentialAnimationGroup);
    if (!d->currentAnimation)
        return;

    const QtSequentialAnimationGroupPrivate::AnimationIndex newAnimationIndex = d->indexForTime(msecs);

    // remove unneeded animations from actualDuration list
    while (newAnimationIndex.index < d->actualDuration.size())
        d->actualDuration.removeLast();

    // newAnimationIndex.index is the new current animation
    if (d->lastLoop < d->currentLoop
        || (d->lastLoop == d->currentLoop && d->currentAnimationIndex < newAnimationIndex.index)) {
            // advancing with forward direction is the same as rewinding with backwards direction
            d->advanceForwards(newAnimationIndex);
    } else if (d->lastLoop > d->currentLoop
        || (d->lastLoop == d->currentLoop && d->currentAnimationIndex > newAnimationIndex.index)) {
            // rewinding with forward direction is the same as advancing with backwards direction
            d->rewindForwards(newAnimationIndex);
    }

    d->setCurrentAnimation(newAnimationIndex.index);

    const int newCurrentTime = msecs - newAnimationIndex.timeOffset;

    if (d->currentAnimation) {
        d->currentAnimation->setCurrentTime(newCurrentTime);
        if (d->atEnd()) {
            //we make sure that we don't exceed the duration here
            d->currentTime += QtAbstractAnimationPrivate::get(d->currentAnimation)->totalCurrentTime - newCurrentTime;
            stop();
        }
    } else {
        //the only case where currentAnimation could be null
        //is when all animations have been removed
        Q_ASSERT(d->animations.isEmpty());
        d->currentTime = 0;
        stop();
    }

    d->lastLoop = d->currentLoop;
}

/*!
    \reimp
*/
void QtSequentialAnimationGroup::updateState(QtAbstractAnimation::State oldState,
                                            QtAbstractAnimation::State newState)
{
    Q_D(QtSequentialAnimationGroup);
    QtAnimationGroup::updateState(oldState, newState);

    if (!d->currentAnimation)
        return;

    switch (newState) {
    case Stopped:
        d->currentAnimation->stop();
        break;
    case Paused:
        if (oldState == d->currentAnimation->state()
            && oldState == QtSequentialAnimationGroup::Running) {
                d->currentAnimation->pause();
            }
        else
            d->restart();
        break;
    case Running:
        if (oldState == d->currentAnimation->state()
            && oldState == QtSequentialAnimationGroup::Paused)
            d->currentAnimation->start();
        else
            d->restart();
        break;
    }
}

/*!
    \reimp
*/
void QtSequentialAnimationGroup::updateDirection(QtAbstractAnimation::Direction direction)
{
    Q_D(QtSequentialAnimationGroup);
    // we need to update the direction of the current animation
    if (state() != Stopped && d->currentAnimation)
        d->currentAnimation->setDirection(direction);
}

/*!
    \reimp
*/
bool QtSequentialAnimationGroup::event(QEvent *event)
{
    return QtAnimationGroup::event(event);
}

void QtSequentialAnimationGroupPrivate::setCurrentAnimation(int index, bool intermediate)
{
    Q_Q(QtSequentialAnimationGroup);

    index = qMin(index, animations.count() - 1);

    if (index == -1) {
        Q_ASSERT(animations.isEmpty());
        currentAnimationIndex = -1;
        currentAnimation = 0;
        return;
    }

    // need these two checks below because this func can be called after the current animation
    // has been removed
    if (index == currentAnimationIndex && animations.at(index) == currentAnimation)
        return;

    // stop the old current animation
    if (currentAnimation)
        currentAnimation->stop();

    currentAnimation = animations.at(index);
    currentAnimationIndex = index;

    emit q->currentAnimationChanged(currentAnimation);

    activateCurrentAnimation(intermediate);
}

void QtSequentialAnimationGroupPrivate::activateCurrentAnimation(bool intermediate)
{
    Q_Q(QtSequentialAnimationGroup);

    if (!currentAnimation)
        return;

    if (state == QtSequentialAnimationGroup::Stopped)
        return;

    currentAnimation->stop();

    // we ensure the direction is consistent with the group's direction
    currentAnimation->setDirection(direction);

    // connects to the finish signal of uncontrolled animations
    if (currentAnimation->totalDuration() == -1)
        QObject::connect(currentAnimation, SIGNAL(finished()), q, SLOT(_q_uncontrolledAnimationFinished()));

    currentAnimation->start();
    if (!intermediate && state == QtSequentialAnimationGroup::Paused)
        currentAnimation->pause();
}

void QtSequentialAnimationGroupPrivate::_q_uncontrolledAnimationFinished()
{
    Q_Q(QtSequentialAnimationGroup);
    Q_ASSERT(qobject_cast<QtAbstractAnimation *>(q->sender()) == currentAnimation);

    // we trust the duration returned by the animation
    while (actualDuration.size() < (currentAnimationIndex + 1))
        actualDuration.append(-1);
    actualDuration[currentAnimationIndex] = currentAnimation->currentTime();

    QObject::disconnect(currentAnimation, SIGNAL(finished()), q, SLOT(_q_uncontrolledAnimationFinished()));

    if ((direction == QtAbstractAnimation::Forward && currentAnimation == animations.last())
        || (direction == QtAbstractAnimation::Backward && currentAnimationIndex == 0)) {
        // we don't handle looping of a group with undefined duration
        q->stop();
    } else if (direction == QtAbstractAnimation::Forward) {
        // set the current animation to be the next one
        setCurrentAnimation(currentAnimationIndex + 1);
    } else {
        // set the current animation to be the previous one
        setCurrentAnimation(currentAnimationIndex - 1);
    }
}

/*!
    \internal
    This method is called whenever an animation is added to
    the group at index \a index.
    Note: We only support insertion after the current animation
*/
void QtSequentialAnimationGroupPrivate::animationInsertedAt(int index)
{
    if (currentAnimation == 0)
        setCurrentAnimation(0); // initialize the current animation

    if (currentAnimationIndex == index
        && currentAnimation->currentTime() == 0 && currentAnimation->currentLoop() == 0) {
            //in this case we simply insert an animation before the current one has actually started
            setCurrentAnimation(index);
    }

    //we update currentAnimationIndex in case it has changed (the animation pointer is still valid)
    currentAnimationIndex = animations.indexOf(currentAnimation);

    if (index < currentAnimationIndex || currentLoop != 0) {
        qWarning("QSequentialGroup::insertAnimationAt only supports to add animations after the current one.");
        return; //we're not affected because it is added after the current one
    }
}

/*!
    \internal
    This method is called whenever an animation is removed from
    the group at index \a index. The animation is no more listed when this
    method is called.
*/
void QtSequentialAnimationGroupPrivate::animationRemovedAt(int index)
{
    Q_Q(QtSequentialAnimationGroup);
    QtAnimationGroupPrivate::animationRemovedAt(index);

    Q_ASSERT(currentAnimation); // currentAnimation should always be set

    if (actualDuration.size() > index)
        actualDuration.removeAt(index);

    const int currentIndex = animations.indexOf(currentAnimation);
    if (currentIndex == -1) {
        //we're removing the current animation, let's update it to another one
        if (index < animations.count())
            setCurrentAnimation(index); //let's try to take the next one
        else if (index > 0)
            setCurrentAnimation(index - 1);
        else// case all animations were removed
            setCurrentAnimation(-1);
    } else if (currentAnimationIndex > index) {
        currentAnimationIndex--;
    }

    // duration of the previous animations up to the current animation
    currentTime = 0;
    for (int i = 0; i < currentAnimationIndex; ++i) {
        const int current = animationActualTotalDuration(i);
        currentTime += current;
    }

    if (currentIndex != -1) {
        //the current animation is not the one being removed
        //so we add its current time to the current time of this group
        currentTime += QtAbstractAnimationPrivate::get(currentAnimation)->totalCurrentTime;
    }

    //let's also update the total current time
    totalCurrentTime = currentTime + loopCount * q->duration();
}

QT_END_NAMESPACE

#include "moc_qtsequentialanimationgroup.cpp"

#endif //QT_NO_ANIMATION
