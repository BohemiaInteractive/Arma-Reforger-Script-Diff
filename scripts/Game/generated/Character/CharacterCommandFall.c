/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Character
\{
*/

class CharacterCommandFall: CharacterCommand
{
	/*!
	end fall with land
	*/
	proto external void Land(int pLandType, float pFallHeight);
	//! returns true if fall is in landing state
	proto external bool IsLanding();
	//! this is true when fall has physically landed - need to call Land after this is true
	proto external bool PhysicsLanded();
}

/*!
\}
*/
