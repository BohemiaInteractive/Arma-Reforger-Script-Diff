class SCR_ScriptedCharacterInputContext
{
	int m_iLoiteringType = -1;
	bool m_iLoiteringShouldHolsterWeapon = false;
	bool m_bLoiteringShouldAlignCharacter = false;
	bool m_bLoiteringRootMotion = false;
	bool m_bLoiteringDisablePlayerInput = false;
	vector m_mLoiteringPosition[4] = { vector.Zero, vector.Zero, vector.Zero, vector.Zero };
	ref SCR_LoiterCustomAnimData m_CustomAnimData;
}
