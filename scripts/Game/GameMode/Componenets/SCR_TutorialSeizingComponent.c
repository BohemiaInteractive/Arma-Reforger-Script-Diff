class SCR_TutorialSeizingComponentClass : SCR_SeizingComponentClass
{
}

class SCR_TutorialSeizingComponent : SCR_SeizingComponent
{
	//------------------------------------------------------------------------------------------------
	void UpdateFlagsInHierarchyPublic()
	{
		UpdateFlagsInHierarchy(SCR_Faction.Cast(m_FactionControl.GetAffiliatedFaction()));
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDisabledAI(IEntity ent)
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_Faction EvaluateEntityFaction(IEntity ent)
	{
		SCR_TutorialGamemodeComponent comp = SCR_TutorialGamemodeComponent.GetInstance();
		
		if (comp && comp.GetCurrentStage().ClassName() != "SCR_Tutorial_Seizing_CaptureBase")
			return null;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(ent);
		
		if (playerId == 0)
			return null;
		
		return super.EvaluateEntityFaction(ent);
	}
}