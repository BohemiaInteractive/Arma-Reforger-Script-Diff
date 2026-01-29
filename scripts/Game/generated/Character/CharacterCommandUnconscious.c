/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Character
\{
*/

class CharacterCommandUnconscious: CharacterCommand
{
	/*
	Starts/Stops unconscious Command
	Lying on your back - 1,
	On belly rolling over to the left - 2, (with arms up - 6)
	On belly rolling over to the right - 3, (with arms up - 7)
	Lying on left side transition - 4, (with arms up - 8)
	Lying on right side transition - 5, (with arms up - 9)
	Exit unconscious animation state (with anims - soft exit) - -1
	Reset unconscious animation state (without anims - hard reset) - -2
	*/
	proto external void StartCommandUnconscious(int commandI);
}

/*!
\}
*/
