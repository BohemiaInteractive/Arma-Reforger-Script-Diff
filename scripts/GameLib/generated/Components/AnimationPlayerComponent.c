/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class AnimationPlayerComponentClass: GenericComponentClass
{
}

class AnimationPlayerComponent: GenericComponent
{
	//! Start playing the animation. Call 'Prepare' first if you need to change the setup of a component!
	proto external void Play();
	//! Stop the animation.
	proto external void Stop();
	//! Check whether the animation is being played at the moment.
	proto external bool IsPlaying();
	//! Prepare this component for playing, provide an animation, start time, playing speed and loop flag.
	proto external bool Prepare(ResourceName animation, float startTime, float speed, bool loop);
	//! Set custom bind pose from mesh object identified by rNameMeshObj.
	proto external bool SetCustomBindPose(ResourceName meshObj);
	proto external IEntity GetOwner();
	//! Get current animation
	proto external ResourceName GetAnimation();
	proto external bool GetLoop();
	proto external float GetPlaySpeed();
	proto external float GetStartTime();
}

/*!
\}
*/
