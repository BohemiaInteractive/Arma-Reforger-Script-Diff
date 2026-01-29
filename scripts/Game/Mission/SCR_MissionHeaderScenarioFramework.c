class SCR_MissionHeaderScenarioFramework : SCR_MissionHeader
{
	[Attribute(defvalue: "-1", desc: "Maximal number of tasks that can be generated (global override, -1 for default)", params: "-1 inf")]
	int m_iMaxNumberOfTasks;
	
	[Attribute("Available Tasks for the Scenario (global override, leave empty for default)")];
	ref array<ref SCR_ScenarioFrameworkTaskType> m_aTaskTypesAvailable;
	
	[Attribute(desc: "List of Areas that will be spawned (optionally with desired Layer Task or further optionally with Slot Task as well) as opposed to leaving it to random generation (global override, leave empty for default)")];
	ref array<ref SCR_ScenarioFrameworkDebugArea> m_aDebugAreas;
	
	[Attribute(defvalue: "-1", desc: "Should the dynamic Spawn/Despawn based on distance from player characters be enabled for the whole GameMode? (global override, -1 for default, 0 for disabled, 1 for enabled)", params: "-1 1 1")];
	int m_iDynamicDespawn;
	
	[Attribute(defvalue: "-1", UIWidgets.Slider, params: "-1 600 1", desc: "How frequently is dynamic spawn/despawn being checked in seconds (global override, -1 for default)")]
	int m_iUpdateRate;
	
	[Attribute(desc: "Global faction aliases to allow quick switching of factions for slots.")]
	ref array<ref SCR_ScenarioFrameworkFactionAliasConfig> m_aFactionAliases;
};