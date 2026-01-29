//! SCR_QuickslotBaseContainer is intended to be used for quickslots, to allow quickslotting of non-item and custom things.

class SCR_QuickslotBaseContainer
{
	protected static const string COMMAND_OUTLINE_WIDGET_NAME = "CommandOutline";
	
	//------------------------------------------------------------------------------------------------
	//!	Gets called when the quickslot with this container is activated
	void ActivateContainer();
	
	//------------------------------------------------------------------------------------------------
	//! Handles how the quickslot with this container is represented visually
	//!	\params widgets that are available for the quickslot visualization
	void HandleVisualization(ImageWidget iconImage, RenderTargetWidget renderTarget, RichTextWidget text, TextWidget quickslotNumber);
	
	//------------------------------------------------------------------------------------------------
	//!	returns true if the containers action can be performed
	bool IsQuickslotActionAvailable();
}

class SCR_QuickslotEntityContainer : SCR_QuickslotBaseContainer
{
	protected IEntity m_Entity;

	//------------------------------------------------------------------------------------------------
	//!	
	void SetEntity(IEntity entity)
	{
		m_Entity = entity;
	}
	
	//------------------------------------------------------------------------------------------------
	//!	
	IEntity GetEntity()
	{
		return m_Entity;
	}
	
	//------------------------------------------------------------------------------------------------
	//!	
	void SCR_QuickslotEntityContainer(IEntity entity)
	{
		m_Entity = entity;
	}
	
	//------------------------------------------------------------------------------------------------
	//!	
	void ActivateContainer(SCR_InventorySlotUI slotUI, ChimeraCharacter playerCharacter)
	{
		slotUI.UseItem(playerCharacter, SCR_EUseContext.FROM_QUICKSLOT);
	}
	
	//------------------------------------------------------------------------------------------------
	//!	
	override void HandleVisualization(ImageWidget iconImage, RenderTargetWidget renderTarget, RichTextWidget text, TextWidget quickslotNumber)
	{
		iconImage.SetVisible(false);
		renderTarget.SetVisible(true);
		text.SetVisible(false);
		quickslotNumber.SetVisible(false);
		
		Widget commandOutline = iconImage.GetParent().FindAnyWidget(COMMAND_OUTLINE_WIDGET_NAME);
		if (commandOutline)
			commandOutline.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsQuickslotActionAvailable()
	{
		if (!m_Entity)
			return false;
		
		return true;
	}
}

class SCR_QuickslotCommandContainer : SCR_QuickslotBaseContainer
{
	protected string m_sCommandName;
	
	//------------------------------------------------------------------------------------------------
	//!	
	void SetCommandName(string name)
	{
		m_sCommandName = name;
	}
	
	//------------------------------------------------------------------------------------------------
	//!	
	string GetCommandName()
	{
		return m_sCommandName;
	}
	
	//------------------------------------------------------------------------------------------------
	//!	
	override void ActivateContainer()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		SCR_PlayerControllerCommandingComponent commandingComp = SCR_PlayerControllerCommandingComponent.Cast(playerController.FindComponent(SCR_PlayerControllerCommandingComponent));
		if (!commandingComp)
			return;
		
		commandingComp.PrepareExecuteCommand(m_sCommandName);
	}
	
	//------------------------------------------------------------------------------------------------
	//!	
	override void HandleVisualization(ImageWidget iconImage, RenderTargetWidget renderTarget, RichTextWidget text, TextWidget quickslotNumber)
	{
		//hide unused widgets which we do not want to use for command quickslot visualization
		iconImage.SetVisible(true);
		iconImage.SetColor(GUIColors.ENABLED);
		renderTarget.SetVisible(false);
		quickslotNumber.SetOpacity(0);
		
		Widget commandOutline = iconImage.GetParent().FindAnyWidget(COMMAND_OUTLINE_WIDGET_NAME);
		if (commandOutline)
			commandOutline.SetVisible(true);
		
		SCR_CommandingManagerComponent commandingManager = SCR_CommandingManagerComponent.GetInstance();
		if (!commandingManager)
			return;
		
		SCR_BaseRadialCommand command = commandingManager.FindCommand(m_sCommandName);
		if (!command)
			return;
		
		iconImage.LoadImageFromSet(0, command.GetIconImageset(), command.GetIconName());
	}
	
	//------------------------------------------------------------------------------------------------
	//!	
	void SCR_QuickslotCommandContainer(string name)
	{
		m_sCommandName = name;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsQuickslotActionAvailable()
	{
		return !m_sCommandName.IsEmpty();
	}
}