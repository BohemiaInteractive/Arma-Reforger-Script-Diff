/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class AnimationControllerComponent: BaseAnimationControllerComponent
{
	proto external void EnableUpdate(notnull IEntity owner, bool enable);
	proto external bool IsUpdateEnabled(notnull IEntity owner);
	//! Manual update of underlying AnimationController. Evaluates animation graph and applies resulting pose.
	//! If automatic updates are enabled, there is no need to call this function, the controller would get updated twice.
	//! \return	update result flags indicating success or specific failure reasons
	proto external EAnimControllerUpdateFlags UpdateController(notnull IEntity owner, float timeSlice);
}

/*!
\}
*/
