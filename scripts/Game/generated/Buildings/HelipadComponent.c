/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Buildings
\{
*/

class HelipadComponent: GameComponent
{
	proto external bool IsOccupied();
	proto external void SetOccupant(IEntity heli);
	proto external float GetLandingRadius();
	proto external float GetLandingAngle();
	proto external void GetLandingTransform(out vector outWorldMatrix[4]);
}

/*!
\}
*/
