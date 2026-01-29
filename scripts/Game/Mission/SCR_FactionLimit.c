class SCR_FactionLimit : ScriptAndConfig
{
	[Attribute("", UIWidgets.EditBox, "Faction key or ID")]
	string m_sFactionKey;

	[Attribute("-1", UIWidgets.Slider, "Faction player limit (-1 = unlimited)", "-1 inf")]
	int m_iFactionLimit;
}