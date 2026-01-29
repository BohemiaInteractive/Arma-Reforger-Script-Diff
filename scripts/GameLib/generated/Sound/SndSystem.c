/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Sound
\{
*/

class SndSystem: WorldSystem
{
	//! Returns actual listener position.
	proto external vector GetListenerPos();
	//! Current the sound world - may be nullptr if SndWorld is not present in the world.
	proto external SndWorld GetSndWorld();
	//! Returns scripted module by class.
	proto external SndBaseModule FindModule(typename type);
}

/*!
\}
*/
