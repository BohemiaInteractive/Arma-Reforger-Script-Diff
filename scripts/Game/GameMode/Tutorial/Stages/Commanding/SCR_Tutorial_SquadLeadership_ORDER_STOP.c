[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SquadLeadership_ORDER_STOPClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SquadLeadership_ORDER_STOP : SCR_BaseTutorialStage
{
	SCR_AIGroup m_Group;
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
		
		PlayNarrativeCharacterStage("SQUADLEADERSHIP_Instructor", 10);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		array <AIWaypoint> wps = {};
		m_Group.GetSlave().GetWaypoints(wps);
		return !wps || wps.Count() <= 0;
	}
}