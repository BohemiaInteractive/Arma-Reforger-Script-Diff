/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Effects
\{
*/

class MultiBarrelMuzzleEffectComponentClass: BaseEffectComponentClass
{
}

class MultiBarrelMuzzleEffectComponent: BaseEffectComponent
{
	proto external bool IsOverridenByAttachment();

	// callbacks

	/*!
	Called during EOnInit.
	\param owner Entity this component is attached to.
	*/
	event void OnInit(IEntity owner);
	event void OnFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity);
}

/*!
\}
*/
