/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Projectile
\{
*/

class BaseProjectileEffect: ScriptAndConfig
{
	/*!
	Ask if the effect has finished or not.
	\return Returns true when a TICKABLE projectile effect has ended
	*/
	proto external bool HasFinished();
	//! Set if the effect is finished or not.
	proto external void SetHasFinished(bool value);
	//! Ask if the effect is enabled or not.
	proto external bool IsEnabled();
	//! Set if the effect is finished or not.
	proto external void SetIsEnabled(bool value);

	// callbacks

	/*!
	Called during EOnInit.
	\param owner Entity this component is attached to.
	*/
	event void OnInit(IEntity owner);
	/*!
	Called when the effect is being applied.
	\param pHitEntity Damaged entity
	\param outMat Hit position/direction/normal
	\param damageSource Projectile
	\param Instigator Instigator of the damage
	\param colliderName Collider name if exist otherwise empty
	\param speed Projectile velocity in m/s
	*/
	event void OnEffect(IEntity pHitEntity, inout vector outMat[3], IEntity damageSource, notnull Instigator instigator, string colliderName, float speed);
	/*!
	Called during EOnFrame.
	\param owner Entity this component is attached to.
	\param timeSlice Delta time since last update.
	*/
	event void OnFrame(IEntity owner, float timeSlice);
	/*!
	OnFrame function should be called
	\return Returns true if OnFrame function should be called.
	*/
	event bool ShouldTick();
}

/*!
\}
*/
