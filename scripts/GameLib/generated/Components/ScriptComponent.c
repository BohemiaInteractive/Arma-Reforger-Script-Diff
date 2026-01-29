/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

/*!
* Parent class for all components created in script.
* Every ScriptComponent is being created in Entity's constructor and may receive following events
* 1. OnComponentInsert is being called when component is created. This is last event Workbench sends in World Editor edit mode.
* 2. EOnInit is being called after all components have been inserted and if the component registered event mask EV_INIT
* 3. EOnActivate is being called if the entity was flagged as TFL_ACTIVE and if the component is active. The component is active by default.
* 4. EOnXXXs are being called base on event mask component registered
* 5. EOnDeactivate is being called when Deactivate is called or when the component is being to be removed. Component must be active to be deactivated.
* 6. OnComponentRemove is being called after a component is removed.
* 7. EOnDelete is being called after entity is going to be destroyed.
*/
class ScriptComponent: GenericComponent
{
	/*!
	Called when Item is initialized from replication stream. Carries the data from Master.
	*/
	event protected bool RplLoad(ScriptBitReader reader);
	/*!
	Called when Item is getting replicated from Master to Slave connection. The data will be
	delivered to Slave using RplInit method.
	*/
	event protected bool RplSave(ScriptBitWriter writer);

	//! Get owner entity
	proto external GenericEntity GetOwner();

	// callbacks

	/*!
	Event when touched by other entity.
	\note You need to have TouchComponent in entity to receive this event.

	\param owner The owner entity
	\param other Entity who touched us
	\param touchTypesMask Bitmask of touch types TouchEvent
	*/
	event protected void EOnTouch(IEntity owner, IEntity other, int touchTypesMask);
	/*!
	Event after entity is allocated and initialized.
	\param owner The owner entity
	*/
	event protected void EOnInit(IEntity owner);
	/*!
	Event when we are visible
	\param owner The owner entity
	\param frameNumber Frame number
	*/
	event protected void EOnVisible(IEntity owner, int frameNumber);
	/*!
	Event every frame
	\param owner The owner entity
	\param timeSlice Time passed since last frame
	*/
	event protected void EOnFrame(IEntity owner, float timeSlice);
	/*!
	Event after physics update.
	\param owner The owner entity
	\param timeSlice Time passed since last frame
	*/
	event protected void EOnPostFrame(IEntity owner, float timeSlice);
	/*!
	Event from animation system
	\param owner The owner entity
	\param type
	\param slot
	*/
	event protected void EOnAnimEvent(IEntity owner, int type, int slot);
	/*!
	Event before every physics fixed step (can be multiple calls per engine update)
	\param owner The owner entity
	\param timeSlice Time slice of physics fixed step
	*/
	event protected void EOnSimulate(IEntity owner, float timeSlice);
	/*!
	Event after every physics fixed step (can be multiple calls per engine update)
	\param owner The owner entity
	\param timeSlice Time slice of physics fixed step
	*/
	event protected void EOnPostSimulate(IEntity owner, float timeSlice);
	/*!
	Event when joint attached to RigidBody of this entity is broken
	\param owner The owner entity
	\param other Other Entity attached to the joint
	*/
	event protected void EOnJointBreak(IEntity owner, IEntity other);
	/*!
	Event when physics engine has moved with this Entity
	\param owner The owner entity
	*/
	event protected void EOnPhysicsMove(IEntity owner);
	/*!
	Event when physics engine registered contact with other RigidBody
	\param owner The owner entity
	\param other Other Entity who contacted us
	\param contact Structure describing the contact
	*/
	event protected void EOnContact(IEntity owner, IEntity other, Contact contact);
	/*!
	Event when a RigidBody active state is changed between consecutive fixed steps
	\param owner The owner entity
	*/
	event protected void EOnPhysicsActive(IEntity owner, bool activeState);
	/*!
	Event every frame after EOnFrame, when "Entity diag" is enabled from the debug menu
	\param owner The owner entity
	\param timeSlice Time slice of simulation step
	*/
	event protected void EOnDiag(IEntity owner, float timeSlice);
	/*!
	Event every fixed frame.
	\param owner The owner entity
	\param timeSlice Fixed time step
	*/
	event protected void EOnFixedFrame(IEntity owner, float timeSlice);
	/*!
	Event after physics update on every fixed frame.
	\param owner The owner entity
	\param timeSlice Fixed time step
	*/
	event protected void EOnPostFixedFrame(IEntity owner, float timeSlice);
	/*!
	* Event when component is activated.
	*/
	event protected void EOnActivate(IEntity owner);
	/*!
	* Event when component is deactivated.
	*/
	event protected void EOnDeactivate(IEntity owner);
	/*!
	Event called after init when all components are initialized.
	\param owner Entity into which component is added
	*/
	event protected void OnPostInit(IEntity owner);
	/*!
	Called when Entity is being to be destroyed (deleted) or component to be deleted (see Game::DeleteScriptComponent).
	\param owner Entity which owns the component
	*/
	event protected void OnDelete(IEntity owner);
	//!Called on child entity when it is added into hierarchy
	event protected void OnAddedToParent(IEntity child, IEntity parent);
	//!Called on child entity when it is removed from hierarchy
	event protected void OnRemovedFromParent(IEntity child, IEntity parent);
	//!Called on parent entity when child entity is added into hierarchy
	event protected void OnChildAdded(IEntity parent, IEntity child);
	//!Called on parent entity when child entity is removed from hierarchy
	event protected void OnChildRemoved(IEntity parent, IEntity child);
}

/*!
\}
*/
