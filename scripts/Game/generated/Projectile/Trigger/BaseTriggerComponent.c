/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Projectile_Trigger
\{
*/

class BaseTriggerComponent: BaseProjectileComponent
{
	proto external IEntity GetOwner();
	proto external void OnUserTrigger(notnull IEntity owner);
	//Only used when overriding the instigator is necessary. E.g.: Player A shoots explosive of player B. The instigator needs to be overriden with player A.
	proto external void OnUserTriggerOverrideInstigator(notnull IEntity owner, Instigator instigator);
	proto external bool WasTriggered();
	proto external void SetLive();
	//! Get projectile effects that are inherited from projectileType parameter.
	proto external void GetProjectileEffects(typename projectileType, out notnull array<BaseProjectileEffect> outProjectileEffects);
	/*!	Time to arm the fuse once the trigger is set live */
	proto external float GetArmingTime();

	// callbacks

	/*!
	Event after entity is allocated and initialized.
	\param owner The owner entity
	*/
	event protected void EOnInit(IEntity owner);
	/*!
	Event when physics engine registered contact with other RigidBody
	\param owner The owner entity
	\param other Other Entity who contacted us
	\param contact Structure describing the contact
	*/
	event protected void EOnContact(IEntity owner, IEntity other, Contact contact);
	/*!
	Called when trigger is triggered within safety distance.
	\param pHitEntity Damaged entity
	\param outMat Hit position/direction/normal
	\param damageSource Projectile
	\param Instigator Instigator of the damage
	\param colliderName Collider name if exist otherwise empty
	\param speed Projectile velocity in m/s
	*/
	event void TriggeredInSafetyDistance(IEntity pHitEntity, inout vector outMat[3], IEntity damageSource, notnull Instigator instigator, string colliderName, float speed);
}

/*!
\}
*/
