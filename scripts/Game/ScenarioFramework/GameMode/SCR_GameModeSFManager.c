[EntityEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_GameModeSFManagerClass : SCR_BaseGameModeComponentClass
{
}

class SCR_GameModeSFManager : SCR_BaseGameModeComponent
{	
	[Attribute("Available Tasks for the Scenario", category: "Tasks")];
	ref array<ref SCR_ScenarioFrameworkTaskType> m_aTaskTypesAvailable;
	
	[Attribute( defvalue: "3", desc: "Maximal number of tasks that can be generated", category: "Tasks" )];
	int m_iMaxNumberOfTasks;
	
	[Attribute(UIWidgets.Auto, desc: "Actions that will be activated after tasks are initialized.", category: "Tasks")];
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aAfterTasksInitActions;
	
	[Attribute(desc: "List of Core Areas that are essential for the Scenario to spawn alongside Debug Areas", category: "Debug")];
	ref array<string> m_aCoreAreas;
	
	[Attribute(desc: "List of Areas that will be spawned (Optionally with desired Layer Task) as opposed to leaving it to random generation", category: "Debug")];
	ref array<ref SCR_ScenarioFrameworkDebugArea> m_aDebugAreas;
	
	[Attribute(desc: "Debug actions accessible from Debug Menu.", category: "Debug")]
	ref array<ref SCR_ScenarioFrameworkDebugAction> m_aDebugActions;
	
	[Attribute(desc: "Should the dynamic Spawn/Despawn based on distance from player characters be enabled for the whole GameMode?", category: "Dynamic Spawn/Despawn")];
	bool m_bDynamicDespawn;
	
	[Attribute(defvalue: "4", UIWidgets.Slider, params: "0 600 1", desc: "How frequently is dynamic spawn/despawn being checked in seconds", category: "Dynamic Spawn/Despawn")]
	int m_iUpdateRate;
	
	[Attribute(desc: "Config with voice over data for whole scenario", params: "conf class=SCR_VoiceoverData")]
	ResourceName m_sVoiceOverDataConfig;
}
