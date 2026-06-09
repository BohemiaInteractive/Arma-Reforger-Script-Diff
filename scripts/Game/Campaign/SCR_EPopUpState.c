//! State of a popup message so we can easily determine what to display
[EnumLinear()]
enum SCR_EPopUpState
{
	NONE,
	FRIENDLY_CAPTURE,
	ENEMY_CAPTURE,
	CONTESTED,
}