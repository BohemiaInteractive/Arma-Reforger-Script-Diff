[ComponentEditorProps(category: "GameScripted/Sound", description: "Component is active only in conflict mode")]
class SCR_CampaignSoundComponentClass : SoundComponentClass
{
}

class SCR_CampaignSoundComponent : SoundComponent
{
	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{
		super.OnInit(owner);
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode || SCR_GameModeEditor.Cast(gameMode))
			EnableDynamicSimulation(false);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_CampaignSoundComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SetScriptedMethodsCall(true);	
	}
}
