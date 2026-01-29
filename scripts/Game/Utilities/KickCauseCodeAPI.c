/*!
\addtogroup GameStateTransitions
\{
*/

// \deprecated
sealed class KickCauseCodeAPI
{
	private void KickCauseCodeAPI();
	private void ~KickCauseCodeAPI();

	static KickCauseCode NewCode(KickCauseGroup2 group, int reason)
	{
		return KickCauseCode.NewCode(group, reason);
	}

	static KickCauseGroup2 GetGroup(KickCauseCode code)
	{
		return code.GetGroup();
	}

	static int GetReason(KickCauseCode code)
	{
		return code.GetReason();
	}
}

/*!
\}
*/
