/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup System
\{
*/

/*!
System for use in GameCode.
*/
class GameSystem: WorldSystem
{
	//! Is this system should be paused while the simulation is paused?
	event bool ShouldBePaused() { return false; };
	//! Is this system should be enabled inside the editor
	event bool ShouldBeEnabledInEditMode() { return false; };
}

/*!
\}
*/
