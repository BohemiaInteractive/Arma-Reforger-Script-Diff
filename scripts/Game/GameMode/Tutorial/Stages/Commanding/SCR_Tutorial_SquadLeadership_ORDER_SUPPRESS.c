[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SquadLeadership_ORDER_SUPPRESSClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SquadLeadership_ORDER_SUPPRESS : SCR_BaseTutorialStage
{
	SCR_AIGroup m_Group;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		RegisterWaypoint("WP_SquadLeadership_SHED", "", "TARGET");
		
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
		
		PlayNarrativeCharacterStage("SQUADLEADERSHIP_Instructor", 9);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnWaypointAdded(AIWaypoint wp)
	{
		if (!wp.IsInherited(SCR_SuppressWaypoint))
			return;
		
		IEntity shed = GetGame().GetWorld().FindEntityByName("WP_SquadLeadership_SHED");
		if (!shed)
			return;
		
		if (vector.Distance(shed.GetOrigin(), wp.GetOrigin()) > 5)
		{
			//TODO> Use wherever possible
			m_Group.RemoveWaypoint(wp);
			//SCR_EntityHelper.DeleteEntityAndChildren(wp);
			GetGame().GetCallqueue().Remove(DelayedEnd);
			ShowHint(2);
			return;
		}
		
		ShowHint(1);
		GetGame().GetCallqueue().CallLater(DelayedEnd, 10000);
	}
	
	//------------------------------------------------------------------------------------------------
	void DelayedEnd()
	{
		m_bFinished = true;
		m_Group.GetSlave().GetOnWaypointAdded().Remove(OnWaypointAdded);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_bFinished;
	}
}