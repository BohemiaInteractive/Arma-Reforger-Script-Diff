/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Character
\{
*/

class CharacterCommandLadder: CharacterCommand
{
	//! returns true if ladder can be exited
	proto external bool CanExit();
	//!
	proto external void Exit();
	//! returns flags that represent a possibility to exit ladder to the right (0x1) and/or to the left (0x2)
	proto external int CanExitLR();
	static proto LadderComponent DetectLadder(ChimeraCharacter pCharacter, int pLadderPhxLayer);
}

/*!
\}
*/
