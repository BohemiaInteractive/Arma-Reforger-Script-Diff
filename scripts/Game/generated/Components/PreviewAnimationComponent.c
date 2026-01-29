/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class PreviewAnimationComponentClass: AnimationControllerComponentClass
{
}

class PreviewAnimationComponent: AnimationControllerComponent
{
	//! Performs manual graph step (might be needed when owner/component is controlled outside of world simulation and visual state update of skeleton is required)
	proto external void UpdateFrameStep(IEntity owner, float ts);
	//! Changes used anim graph
	proto external void SetGraphResource(IEntity owner, ResourceName graphResource, ResourceName animInstanceResource, string startNode);
	//! Changes used anim instance
	proto external void UpdateAnimInstance(IEntity owner, ResourceName instanceResource);
	//! Changes used ik pose resource
	proto external void SetHandsIKPose(IEntity owner, ResourceName ikPoseResource);
	//! Changes IK state for weapon IKVariable (usually state set once upon creation)
	proto external void SetIkState(bool leftHand, bool rightHand);
}

/*!
\}
*/
