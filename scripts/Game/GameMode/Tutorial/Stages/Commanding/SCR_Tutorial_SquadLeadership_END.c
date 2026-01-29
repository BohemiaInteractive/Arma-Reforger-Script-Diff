[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SquadLeadership_ENDClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SquadLeadership_END : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		SeparateAI();
		//PlayNarrativeCharacterStage("SQUADLEADERSHIP_Instructor", 17);
		SCR_VoiceoverSystem.GetInstance().PlaySequence("End");
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SeparateAI()
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;
		
		SCR_PlayerControllerGroupComponent playerGroupComp = SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!playerGroupComp)
			return;
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.FindGroup(playerGroupComp.GetGroupID());
		if (!group)
			return;
		
		array <AIAgent> agents = {};
		group.GetSlave().GetAgents(agents);
		
		if (!agents || agents.IsEmpty())
			return;
		
		SCR_AIGroup newGroup = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefabEx("{000CD338713F2B5A}Prefabs/AI/Groups/Group_Base.et", 0));
		if (!newGroup)
			return;
		
		newGroup.SetName("OldGroup");
		
		foreach (AIAgent agent : agents)
		{
			newGroup.AddAgent(agent);
		}
		
		IEntity pos = GetGame().GetWorld().FindEntityByName("WP_SquadLeadership_DEFEND");
		if (!pos)
			return;
		
		AIWaypoint wp = AIWaypoint.Cast(m_TutorialComponent.SpawnAsset("COMMANDING_DEFEND_OLD", "{93291E72AC23930F}Prefabs/AI/Waypoints/AIWaypoint_Defend.et", pos));
		if (wp)
			newGroup.AddWaypoint(wp);
		
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_sLastFinishedEvent == "SOUND_TUTORIAL_TA_POSITION_DEFENDED_INSTRUCTOR_O_02" || GetDuration() > 15000;
	}
}