/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
Opaque code representing various reasons for ending gameplay. It consists of two
parts: category and reason. Categories differentiate various systems and each
system should provide it's own list of possible reasons that could cause
gameplay to end.
*/
sealed class KickCauseCode: handle64
{
	static KickCauseCode NONE = null;

	static proto KickCauseCode NewCode(KickCauseGroup group, int reason);
	proto external KickCauseGroup GetGroup();
	proto external int GetReason();
}
