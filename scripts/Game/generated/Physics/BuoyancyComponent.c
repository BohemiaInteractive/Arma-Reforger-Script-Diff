/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Physics
\{
*/

class BuoyancyComponent: GenericComponent
{
	proto external IEntity GetOwner();
	proto external bool IsInWater();
	proto external bool IsCenterOfMassInWater();

	// callbacks

	event void OnEnterWater();
	event void OnExitWater();
}

/*!
\}
*/
