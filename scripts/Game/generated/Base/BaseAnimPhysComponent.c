/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Base
\{
*/

class BaseAnimPhysComponent: GameComponent
{
	/*!
	-----------------------------------------------------------------------------
	 commands
	-----------------------------------------------------------------------------
	*/
	proto external void SetCurrentCommand(AnimPhysCommandScripted pCommand);
	proto external AnimPhysCommandScripted GetCommandScripted();
	/*!
	-----------------------------------------------------------------------------
	 script binding
	-----------------------------------------------------------------------------
	*/
	proto external TAnimGraphCommand	BindCommand(string pCommandName);
	//! binds variable
	proto external TAnimGraphVariable	BindVariableFloat(string pVariableName);
	proto external TAnimGraphVariable	BindVariableInt(string pVariableName);
	proto external TAnimGraphVariable	BindVariableBool(string pVariableName);
	//! binds tag
	proto external TAnimGraphTag		BindTag(string pTagName);
	//! binds event
	proto external TAnimGraphEvent		BindEvent(string pEventName);
	//! binds prediction
	proto external TAnimGraphPrediction	BindPrediction(string pPredictionName);
	//! Functions for setting animation variables and calling animation commands. It is recommended to, instead of
	//! binding and setting the variables directly, use a ScriptedCommand object with a static table,
	//! and to set those variables in the PreAnimUpdate function using the functions provideed by ScriptedCommand instead.
	//! For an example on how to use those, see SCR_CharacterCommandSwim.
	//! Animation variables can also be overwritten by the character commands, as they are evaluated
	//! right before animations, leading to manually set values being discarded.
	proto external void SetVariableFloat(TAnimGraphVariable varIdx, float value);
	proto external void SetVariableInt(TAnimGraphVariable varIdx, int value);
	proto external void SetVariableBool(TAnimGraphVariable varIdx, bool value);
	proto external void CallCommand(TAnimGraphCommand pCmdIndex, int intParam, float floatParam);
	proto external void CallCommand4I(TAnimGraphCommand pCmdIndex, int intParam1, int intParam2, int intParam3, int intParam4, float floatParam);
	//! returns true when character is physically falling
	proto external bool PhysicsIsFalling();
	//! enables physics
	proto external void PhysicsEnableGravity(bool pState);
	proto external void PhysicsSetStance(int index);
	proto external void PhysicsSetCollisionOffset(vector pOff);
	/*!
	//-----------------------------------------------------------------------------
	// physics transform API
	//-----------------------------------------------------------------------------

	// This new transform API is needed for physically linking with another entity.
	// Linking with another entity means that we are making the another entity our parent, but not in entity hierarchy terms, only in physical terms.
	// The physical movement and transform will work as if the linked entity would be parent in entity hierarchy, but we have more control and don't have the overhead of entity hierarchy.
	// If there is a linked entity, it doesn't mean that owner has a parent in entity hierarchy.
	// All of the methods of this physics transform api were done to work even if the linked entity is not streamed in or is deleted.
	// If the entity is not found, but we are still linked, we use the last known linked entity transformation to prevent distrupting transform changes when streaming in/out.
	// Physical parent transform doesn't have to be the same as the parent's or linked entity transform.
	// We do that to prevent quick character orientation change when linking to an entity. It does make linking transition much simpler for systems that use owner's local transformation.
	// This API should be used if you care about LS of the owner when its linked. If you only care about world transform of the owner, you can use owner's entity API.
	// Use cases of this API should be camera, commands or theoretically any code that works with position/orientation of owner and needs to work in the space of the linked entity.

	*/
	//! Returns physical transform in world space.
	proto external void PhysicsGetTransformWS(out vector mat[4]);
	//! Returns physical transform in local space of parent or linked entity.
	proto external void PhysicsGetTransformLS(out vector mat[4]);
	//! Returns physical transform of parent or linked entity.
	proto external void PhysicsGetParentTransform(out vector mat[4]);
	//! Returns true if the character is physically linked to another entity.
	proto external bool PhysicsIsLinked();
	//! Returns linked entity if its found, null if its not found.
	//! If this returns a null, it doesn't mean we are not linked, so this shouldn't be used as a check if its linked
	proto external IEntity GetLinkedEntity();
	//! Returns velocity in local space of parent or linked entity.
	//! Returned velocity is relative to the velocity of parent or linked enitity.
	proto external vector PhysicsGetLocalVelocity();
	//! Same as method above, y coordinate is zero.
	proto external vector PhysicsGetLocalVelocityXZ();
	//! Returns (yaw pitch roll) vector in local space of parent or linked entity
	//! In degrees.
	proto external vector PhysicsGetLocalYawPitchRoll();
}

/*!
\}
*/
