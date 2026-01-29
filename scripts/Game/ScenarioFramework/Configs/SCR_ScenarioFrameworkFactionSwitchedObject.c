[BaseContainerProps(), SCR_BaseContainerCustomTitleFields({"m_sFactionKey", "m_sObjectToSpawn"}, "%1 - %2")]
class SCR_ScenarioFrameworkFactionSwitchedObject {
	[Attribute(desc: "Required faction key to spawn this object.")]
	FactionKey m_sFactionKey;
	
	[Attribute(params: "et", desc: "Resource name of the object to be spawned")]
	ResourceName m_sObjectToSpawn;
}
