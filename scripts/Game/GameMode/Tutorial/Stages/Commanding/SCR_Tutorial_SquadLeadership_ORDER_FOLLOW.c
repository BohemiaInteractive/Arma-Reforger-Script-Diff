[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SquadLeadership_ORDER_FOLLOWClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SquadLeadership_ORDER_FOLLOW : SCR_BaseTutorialStage
{
	SCR_AIGroup m_Group;
	bool m_bWPAdded;
	ResourceName m_sFollowWaypoint = "{A0509D3C4DD4475E}Prefabs/AI/Waypoints/AIWaypoint_Follow.et";
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
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
		
		PlayNarrativeCharacterStage("SQUADLEADERSHIP_Instructor", 6);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnWaypointAdded(AIWaypoint wp)
	{
		m_bWPAdded = wp.GetPrefabData().GetPrefabName() == m_sFollowWaypoint;
		if (m_bWPAdded)
			m_Group.GetSlave().GetOnWaypointAdded().Remove(OnWaypointAdded);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_bWPAdded;
	}
}