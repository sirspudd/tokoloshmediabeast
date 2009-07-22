Qt Animation Framework v2.3

This component adds a convenient API to do animations in Qt. Note
that this solution is experimental and work in progress: No
backwards compatibility (source nor binary) is provided.

The Animation Framework lets you define, run and control
animations, states and transitions. The foundation classes
QtPropertyAnimation, QtSequentialAnimationGroup and
QtParallelAnimationGroupare used to scoreboard both simple and
complex animations. A simple animation might change a widget's
geometry from one rectangle to another, over the course of 2
seconds, using a linear easing curve. You can daisy-chain
animations by adding them to a QtSequentialAnimationGroup, a class
that also gives you fine grained control over pauses and
combinations of parallel and sequential subanimations. In addition
to this, The State Machine Framework lets you define and run state
machines.

