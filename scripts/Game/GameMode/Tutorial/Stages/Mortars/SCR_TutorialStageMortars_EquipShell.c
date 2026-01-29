[EntityEditorProps(insertable: false)]
class SCR_TutorialStageMortars_EquipShellClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageMortars_EquipShell : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		PlayNarrativeCharacterStage("FIRESUPPORT_Instructor", 6);
		
		SCR_TutorialStageMortars_SecondTarget.SpawnShell();
		RegisterWaypoint("MortarShell", "", "AMMO");
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		
		if (!player)
			return false;
		
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.Cast(player.FindComponent(SCR_GadgetManagerComponent));
		
		if (!gadgetManager)
			return false;
		
		return gadgetManager.GetHeldGadget() == GetGame().GetWorld().FindEntityByName("MortarShell");
	}
};