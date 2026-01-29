[BaseContainerProps(), SCR_BaseContainerCustomTitleFields({"m_sAlias", "m_sFactionKey"}, "%1 - %2")]
class SCR_ScenarioFrameworkFactionAliasConfig
{
	[Attribute(desc: "Alias of faction referenced by slots. Examples: Invaders, Occupants, Rebels")]
	string m_sAlias;

	[Attribute(desc: "Faction key used to select units from an entity catelog or inline entity catalog")]
	FactionKey m_sFactionKey;
}
