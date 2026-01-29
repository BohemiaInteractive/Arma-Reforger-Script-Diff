[ComponentEditorProps(category: "GameScripted/Editor", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditorVONControllerComponentClass : SCR_BaseEditorComponentClass
{
}

//! Manages interaction with Voice Over Network IEntityComponentSource
class SCR_EditorVONControllerComponent : SCR_BaseEditorComponent
{
	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorOpen()
	{
		SCR_EditorManagerEntity instance = SCR_EditorManagerEntity.GetInstance();
		
		if (instance)
			instance.GetOnModeChange().Insert(OnEditorModeChange);
		
		OnEditorModeChange();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorClose()
	{
		SCR_EditorManagerEntity instance = SCR_EditorManagerEntity.GetInstance();
		
		if (instance)
			instance.GetOnModeChange().Remove(OnEditorModeChange);
		
		SetVONMenuDisabled(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorModeChange()
	{
		EEditorMode mode = SCR_EditorManagerEntity.GetInstance().GetCurrentMode();
		
		SetVONMenuDisabled(mode == EEditorMode.PHOTO);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Enable / Disable VON menu
	//! \param[in] is disabled
	protected void SetVONMenuDisabled(bool disabled)
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		SCR_VONController vonController = SCR_VONController.Cast(playerController.FindComponent(SCR_VONController));
		if (vonController && vonController.GetVONMenu())
			vonController.GetVONMenu().SetMenuDisabled(disabled);
	}
}
