[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SquadLeadership_ORDER_DEFENDClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SquadLeadership_ORDER_DEFEND : SCR_BaseTutorialStage
{
	SCR_AIGroup m_Group;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		RegisterWaypoint("WP_SquadLeadership_DEFEND");
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;
		
		SCR_PlayerControllerGroupComponent playerGroupComp = SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!playerGroupComp)
			return;
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		m_Group = groupsManager.FindGroup(playerGroupComp.GetGroupID());
		if (!m_Group)
			return;
		
		m_Group.GetSlave().GetOnWaypointAdded().Insert(OnWaypointAdded);
		
		//PlayNarrativeCharacterStage("SQUADLEADERSHIP_Instructor", 16);
		//SCR_VoiceoverSystem.GetInstance().PlaySequence("AtCheckpoint");
	}
	
	//------------------------------------------------------------------------------------------------
	void OnWaypointAdded(AIWaypoint wp)
	{		
		if (!wp.IsInherited(SCR_DefendWaypoint))
			return;
		
		IEntity areaPos = GetGame().GetWorld().FindEntityByName("WP_SquadLeadership_DEFEND");
		if (!areaPos)
			return;
		
		if (vector.Distance(wp.GetOrigin(), areaPos.GetOrigin()) > 15 || !wp.IsInherited(SCR_DefendWaypoint))
		{
			ShowHint(1);
			m_Group.RemoveWaypoint(wp);
			return;
		}
		
		m_Group.GetSlave().GetOnWaypointAdded().Remove(OnWaypointAdded);
		
		m_bFinished = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_bFinished;
	}
}