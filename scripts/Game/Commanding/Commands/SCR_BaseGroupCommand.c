//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseGroupCommandTitleField("m_sCommandName")]
class SCR_BaseGroupCommand : SCR_BaseRadialCommand
{
	[Attribute(defvalue: "1", desc: "Will the command be shown to leader?")]
	protected bool m_bShowToLeader;

	[Attribute(defvalue: "0", desc: "Will the command be shown to member?")]
	protected bool m_bShowToMember;

	[Attribute(defvalue: "0", desc: "Will the command be shown while commanding from map?")]
	protected bool m_bShowOnMap;

	[Attribute(defvalue: "0", desc: "Is the command available while player is dead?")]
	protected bool m_bUsableWhileDead;

	[Attribute(defvalue: "0", desc: "Is the command available while player is unconscious?")]
	protected bool m_bUsableWhileUncouscious;

	[Attribute(defvalue: "0", desc: "ID of the gesture in gesture state machine to be played when command is given out. 0 = no gesture")]
	protected int m_iGestureID;

	[Attribute("", UIWidgets.EditBox, "Name of the sound event to be played, leave empty for no event")]
	protected string m_sSoundEventName;
	
	[Attribute(defvalue: "-1", desc: "seed for the version of sound event to be played, -1 for random. For valid values refer to audio guide")]
	protected float m_fSoundEventSeed;

	[Attribute(defvalue: "0", desc: "Will the command show preview in world before it is given out")]
	protected bool m_bShowPreview;
	
	[Attribute(defvalue: "1", desc: "Will the AI respond with voicelines to given command")]
	protected bool m_bPlayAIResponse;

	[Attribute(SCR_ECommandVisualizationDuration.BRIEF.ToString(), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ECommandVisualizationDuration))]
	protected SCR_ECommandVisualizationDuration m_eVisualizationDuration;

	[Attribute("{10A7FB1074F322FC}Configs/Commanding/PlacedCommandInfoDisplay.conf", UIWidgets.ResourceNamePicker, params: "conf", category: "Visualization")]
	protected ResourceName m_sInfoDisplayConfig;

	protected Widget m_wVisualizationWidget;

	//------------------------------------------------------------------------------------------------
	override bool CanRoleShow()
	{
		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!groupController)
			return false;

		bool isPlayerLeader = groupController.IsPlayerLeaderOwnGroup();
		if (isPlayerLeader && !m_bShowToLeader)
			return false;

		if (!isPlayerLeader && !m_bShowToMember)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	string GetSoundEventName()
	{
		return m_sSoundEventName;
	}
	
	float GetSoundEventSeed()
	{
		return m_fSoundEventSeed;
	}

	//------------------------------------------------------------------------------------------------
	bool CanShowPreview()
	{
		return m_bShowPreview;
	}
	
	bool ShouldPlayAIResponse()
	{
		return m_bPlayAIResponse;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanShowOnMap()
	{
		return m_bShowOnMap;
	}

	//------------------------------------------------------------------------------------------------
	int GetCommandGestureID()
	{
		return m_iGestureID;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ECommandVisualizationDuration GetVisualizationDuration()
	{
		return m_eVisualizationDuration;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownInCurrentLifeState()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return false;

		//returns true because if there is no controlled entity then player is not dead nor uncoscious.
		ChimeraCharacter character = ChimeraCharacter.Cast(playerController.GetControlledEntity());
		if (!character)
			return true;

		//Return true in case of component not found to prevent VME
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return true;

		ECharacterLifeState lifeState = controller.GetLifeState();
		if ((!m_bUsableWhileDead && lifeState == ECharacterLifeState.DEAD) || (!m_bUsableWhileUncouscious && lifeState == ECharacterLifeState.INCAPACITATED))
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void VisualizeCommand(vector targetPosition)
	{
		if (!m_bShowPreview)
			return;
		
		Resource resource = BaseContainerTools.LoadContainer(m_sInfoDisplayConfig);
		if (!resource || !resource.IsValid())
			return;

		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.GetHUDManager();
		if (!hudManager)
			return;
		
		SCR_PlacedCommandInfoDisplay display = SCR_PlacedCommandInfoDisplay.Cast(BaseContainerTools.CreateInstanceFromContainer(resource.GetResource().ToBaseContainer()));
		if (!display)
			return;

		SCR_PlayerControllerCommandingComponent commandComp = SCR_PlayerControllerCommandingComponent.GetLocalPlayerControllerCommandingComponent();
		if (commandComp)
			commandComp.SetShownCommand(display);
		
		hudManager.StartDrawing(display);
		display.VisualizeCommand(targetPosition, this);
	}

	//------------------------------------------------------------------------------------------------
	override void VisualizeCommandPreview(vector targetPosition)
	{
		if (!m_bShowPreview)
			return;
		
		Resource resource = BaseContainerTools.LoadContainer(m_sInfoDisplayConfig);
		if (!resource || !resource.IsValid())
			return;
		
		SCR_PlacedCommandInfoDisplay display = SCR_PlacedCommandInfoDisplay.Cast(BaseContainerTools.CreateInstanceFromContainer(resource.GetResource().ToBaseContainer()));
		if (!display)
			return;
		
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.GetHUDManager();
		if (!hudManager)
			return;

		SCR_PlayerControllerCommandingComponent commandComp = SCR_PlayerControllerCommandingComponent.GetLocalPlayerControllerCommandingComponent();
		if (commandComp)
			commandComp.SetShownCommandPreview(display);
		
		hudManager.StartDrawing(display);
		display.SetIsPreview(true);
		display.VisualizeCommand(targetPosition, this);
	}
}
