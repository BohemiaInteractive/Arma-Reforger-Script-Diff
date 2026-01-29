class SCR_ScenarioFrameworkCinematicEntityComponentClass : ScriptComponentClass
{
}

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkCinematicEntityComponent : ScriptComponent
{
	[Attribute(desc: "Actions available for cinematic.")]
	ref array<ref SCR_ScenarioFrameworkDebugAction> m_aDebugActions;
	
	//------------------------------------------------------------------------------------------------
	void ActivateAction(string actionName)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(actionName))
			return;
		
		SCR_ScenarioFrameworkDebugAction activatedAction;
		foreach (SCR_ScenarioFrameworkDebugAction action : m_aDebugActions)
		{
			if (!action || action.m_sDebugActionName != actionName)
				continue;
			
			activatedAction = action;
			break;
		}
		
		foreach (SCR_ScenarioFrameworkActionBase action : activatedAction.m_aDebugActions)
		{
			action.Init(null);
			action.OnActivate(null);
		}
	}
}