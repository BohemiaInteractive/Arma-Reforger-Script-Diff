//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingBuildUserAction : SCR_ScriptedUserAction
{
	protected SCR_CampaignBuildingLayoutComponent m_LayoutComponent;
	// entity to whom the user action is just shown but is not actively perfroming the action.
	protected IEntity m_User;
	// entity who starts performing the action.
	protected IEntity m_ActiveUser;
	protected SCR_GadgetManagerComponent m_GadgetManager;
	protected bool m_bIsShown;
	
	//------------------------------------------------------------------------------------------------
	protected override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_LayoutComponent = SCR_CampaignBuildingLayoutComponent.Cast(pOwnerEntity.FindComponent(SCR_CampaignBuildingLayoutComponent));
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		SCR_CampaignBuildingNetworkComponent networkComponent = GetNetworkManager();
		if (!networkComponent)
			return;

		networkComponent.AddBuildingValue(GetBuildingToolValue(pUserEntity), pOwnerEntity);
		ProcesXPreward();
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		m_ActiveUser = pUserEntity;
		
		if (!ShouldPerformPerFrame())
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		CharacterControllerComponent charController = character.GetCharacterController();
		if (charController)
		{
			CharacterAnimationComponent pAnimationComponent = charController.GetAnimationComponent();
			int itemActionId = pAnimationComponent.BindCommand("CMD_Item_Action");

			ItemUseParameters params = new ItemUseParameters();
			params.SetEntity(GetBuildingTool(pUserEntity));
			params.SetAllowMovementDuringAction(false);
			params.SetKeepInHandAfterSuccess(true);
			params.SetCommandID(itemActionId);
			params.SetCommandIntArg(1);

			charController.TryUseItemOverrideParams(params);
		}

		super.OnActionStart(pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		m_ActiveUser = null;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		CharacterControllerComponent charController = character.GetCharacterController();
		if (charController)
		{
			CharacterAnimationComponent pAnimationComponent = charController.GetAnimationComponent();
			int itemActionId = pAnimationComponent.BindCommand("CMD_Item_Action");
			CharacterCommandHandlerComponent cmdHandler = pAnimationComponent.GetCommandHandler();
			if (cmdHandler)
				cmdHandler.FinishItemUse(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnConfirmed(IEntity pUserEntity)
	{
		m_ActiveUser = null;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		CharacterControllerComponent charController = character.GetCharacterController();
		if (charController)
		{
			CharacterAnimationComponent pAnimationComponent = charController.GetAnimationComponent();
			CharacterCommandHandlerComponent cmdHandler = pAnimationComponent.GetCommandHandler();
			cmdHandler.FinishItemUse(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	//~ If continues action it will only execute everytime the duration is done
	override void PerformContinuousAction(IEntity pOwnerEntity, IEntity pUserEntity, float timeSlice)
	{
		if (!LoopActionUpdate(timeSlice))
			return; 
		
		PerformAction(pOwnerEntity, pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	// The user action is shown when the preview is visible - means player has a building tool ready (but also when he is running around with building tool hiden)
	override bool CanBeShownScript(IEntity user)
	{
		m_User = user;	
		m_bIsShown = m_LayoutComponent && m_LayoutComponent.HasBuildingPreview();
		return m_bIsShown;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsShown()
	{
		return m_bIsShown;
	}
	
	//------------------------------------------------------------------------------------------------
	// The user action can be performed, when player has a building tool in hands.
	override bool CanBePerformedScript(IEntity user)
	{	
		if (!m_GadgetManager)
		{
			m_GadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(user);
			
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (playerController)
				playerController.m_OnControlledEntityChanged.Insert(SetNewGadgetManager);
			
			return false;
		}
		
		SCR_GadgetComponent gadgetComponent = m_GadgetManager.GetHeldGadgetComponent();
		if (!gadgetComponent)
			return false;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		CharacterControllerComponent charController = character.GetCharacterController();
		
		if (charController && !IsInProgress() && !charController.CanUseItem())
			return false;			
		
		return (gadgetComponent.GetMode() == EGadgetMode.IN_HAND);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Stops player animation
	protected void CancelPlayerAnimation(notnull IEntity pUserEntity)
	{		
		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		CharacterControllerComponent charController = character.GetCharacterController();
		if (!charController)
			return;

		CharacterAnimationComponent pAnimationComponent = charController.GetAnimationComponent();
		if (!pAnimationComponent)
			return;

		CharacterCommandHandlerComponent handlerComponent = pAnimationComponent.GetCommandHandler();
		if (!handlerComponent)
			return;

		handlerComponent.FinishItemUse(true);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (!m_LayoutComponent || !m_User)
			return false;

		float progressPercentage = GetBuildingToolValue(m_User) / (m_LayoutComponent.GetToBuildValue() * 0.01);
		float buildPercentage = m_LayoutComponent.GetCurrentBuildValue() / (m_LayoutComponent.GetToBuildValue() * 0.01);

		if (progressPercentage != (int)progressPercentage)
			ActionNameParams[0] = progressPercentage.ToString(lenDec: 1);
		else
			ActionNameParams[0] = progressPercentage.ToString();

		if (buildPercentage != (int)buildPercentage)
			ActionNameParams[1] = buildPercentage.ToString(lenDec: 1);
		else
			ActionNameParams[1] = buildPercentage.ToString();

		return super.GetActionNameScript(outName);
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets a new gadget manager. Controlled by an event when the controlled entity has changed.
	void SetNewGadgetManager(IEntity from, IEntity to)
	{
		m_GadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(to);
	}

	//------------------------------------------------------------------------------------------------
	//! Get building tool entity
	IEntity GetBuildingTool(notnull IEntity ent)
	{
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(ent);
		if (!gadgetManager)
			return null;

		return gadgetManager.GetHeldGadget();
	}

	//------------------------------------------------------------------------------------------------
	// Gets the "building tool value" of building tool. It means how many points are added by one usage of the building tool
	int GetBuildingToolValue(notnull IEntity ent)
	{
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(ent);
		if (!gadgetManager)
			return 0;

		SCR_CampaignBuildingGadgetToolComponent gadgetComponent = SCR_CampaignBuildingGadgetToolComponent.Cast(gadgetManager.GetHeldGadgetComponent());
		if (!gadgetComponent)
			return 0;

		return gadgetComponent.GetToolConstructionValue();
	}
	
	//------------------------------------------------------------------------------------------------
	void ProcesXPreward()
	{		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		SCR_CampaignBuildingManagerComponent campaignBuildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		campaignBuildingManagerComponent.ProcesXPreward();
	}

	//------------------------------------------------------------------------------------------------
	//! GetNetworkManager
	protected SCR_CampaignBuildingNetworkComponent GetNetworkManager()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return null;

		return SCR_CampaignBuildingNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignBuildingNetworkComponent));
	}

	//------------------------------------------------------------------------------------------------
	// Destructor
	void ~SCR_CampaignBuildingBuildUserAction()
	{
		if (m_ActiveUser)
			CancelPlayerAnimation(m_ActiveUser);
	}
}
