[EntityEditorProps(insertable: false)]
class SCR_TutorialStageMortars_SightsClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageMortars_Sights : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return SCR_CharacterCameraHandlerComponent.Cast(SCR_PlayerController.GetLocalControlledEntity().FindComponent(SCR_CharacterCameraHandlerComponent)).CheckVehicleADS();
	}
};