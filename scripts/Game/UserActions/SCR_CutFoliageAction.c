class SCR_CutFoliageAction : SCR_ScriptedUserAction
{
	[Attribute(EGadgetType.NONE.ToString(), uiwidget: UIWidgets.ComboBox, desc: "What gadget player has to equip in order to be able to use this action", enumType: EGadgetType)]
	protected EGadgetType m_eRequiredGadget;

	[Attribute(defvalue: "2", desc: "Value by which action progress is going to be multiplied to speed it up when character is a engineer", params: "0.1 inf 0.01")]
	protected float m_fQualifiedPersonnelBonusUseSpeedFactor;

	// entity who starts performing the action.
	protected IEntity m_ActiveUser;
	protected SCR_GadgetManagerComponent m_GadgetManager;

	protected vector m_vDesiredDirection;
	protected ECharacterStance m_eSavedStance = -1;

	protected Tree m_AssignedTree;
	protected bool m_bHiddenAction;
	protected bool m_bIsQualified;

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		if (SCR_CharacterHelper.IsPlayerOrAIOwner(character))
			StopAction(character);

		m_bHiddenAction = true;

		RplComponent rplComp = character.GetRplComponent();

		if (m_AssignedTree && rplComp.Role() == RplRole.Authority)
			m_AssignedTree.ForceKill_S();

		// remove instantly so the action doesnt linger on client due to lag
		m_AssignedTree = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Override this if you would want to change which squad roles should make a character qualified
	protected array<SCR_EGroupRole> GetQualifiedRoles()
	{
		return {SCR_EGroupRole.ENGINEER};
	}

	//------------------------------------------------------------------------------------------------
	//! Override this if you would want to change which character label should make a character qualified
	protected array<EEditableEntityLabel> GetQualifiedLabels()
	{
		return {EEditableEntityLabel.ROLE_SAPPER};
	}

	//------------------------------------------------------------------------------------------------
	override float GetActionProgressScript(float fProgress, float timeSlice)
	{
		if (m_bIsQualified)
			timeSlice *= m_fQualifiedPersonnelBonusUseSpeedFactor;

		return super.GetActionProgressScript(fProgress, timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	protected override bool LoopActionUpdate(float timeSlice)
	{
		if (m_bIsQualified)
			timeSlice *= m_fQualifiedPersonnelBonusUseSpeedFactor;

		return super.LoopActionUpdate(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	//! Target specific foliage instance with this useraction
	//! \param[in] foliage Tree or shrub instance
	void AssignTargetFoliage(Tree foliage)
	{
		m_AssignedTree = foliage;
		m_bHiddenAction = false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		m_ActiveUser = pUserEntity;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!character || !SCR_CharacterHelper.IsPlayerOrAIOwner(character))
			return;

		m_bIsQualified = character && (character.HasRole(GetQualifiedRoles()) || character.HasLabel(GetQualifiedLabels()));

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;

		CharacterHeadAimingComponent headAiming = controller.GetHeadAimingComponent();
		if (!headAiming)
			return;

		super.OnActionStart(pUserEntity);

		vector charMat[4];
		character.GetWorldTransform(charMat);
		m_vDesiredDirection = headAiming.GetLookAngles();
		m_vDesiredDirection = m_vDesiredDirection.AnglesToVector(); //Converts spherical coordinates to unit length vector
		m_vDesiredDirection = m_vDesiredDirection.Multiply3(charMat); //Translate from local space to world space
		m_vDesiredDirection = m_vDesiredDirection.VectorToAngles(); //Convert back to angles

		m_eSavedStance = controller.GetStance();
		m_GadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(pUserEntity);
		if (!m_GadgetManager)
			return;

		IEntity gadgetEntity = m_GadgetManager.GetHeldGadget();
		if (!gadgetEntity)
			return;

		CharacterAnimationComponent animationComponent = controller.GetAnimationComponent();
		int itemActionId = animationComponent.BindCommand("CMD_Item_Action");

		ItemUseParameters params = new ItemUseParameters();
		params.SetEntity(gadgetEntity);
		params.SetAllowMovementDuringAction(false);
		params.SetKeepInHandAfterSuccess(true);
		params.SetCommandID(itemActionId);
		params.SetCommandIntArg(1);

		controller.TryUseItemOverrideParams(params);

		RplComponent rplComp = character.GetRplComponent();

		// add lock so the targeted foliage cant be switched mid action
		if (rplComp && rplComp.IsOwner())
		{
			PlayerController playerController = SCR_PlayerController.s_pLocalPlayerController;;
			if (!playerController)
					return;

			SCR_FoliageDestructionComponent foliageDestructionComp = SCR_FoliageDestructionComponent.Cast(playerController.FindComponent(SCR_FoliageDestructionComponent));
			if (foliageDestructionComp)
				foliageDestructionComp.SetIsFoliageLocked(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		super.OnActionCanceled(pOwnerEntity, pUserEntity);

		m_ActiveUser = null;

		m_eSavedStance = -1;
		m_vDesiredDirection = vector.Zero;

		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		StopAction(character);
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
	// If continues action, it will only execute everytime the duration is done
	override void PerformContinuousAction(IEntity pOwnerEntity, IEntity pUserEntity, float timeSlice)
	{
		if (!LoopActionUpdate(timeSlice))
			return; 

		PerformAction(pOwnerEntity, pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	// The user action is shown when the preview is visible - means the action is assigned to a shrub and the wplayer has a building tool ready (but also when he is running around with building tool hidden)
	override bool CanBeShownScript(IEntity user)
	{		
		if (!m_AssignedTree || m_bHiddenAction)
			return false;

		IEntity owner = GetOwner();
		if (!owner || owner.GetParent())
			return false;

		if (CompartmentAccessComponent.GetVehicleIn(user))
			return false;

		if (m_eRequiredGadget != EGadgetType.NONE)
		{
			if (!m_GadgetManager)
			{
				m_GadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(user);
				if (!m_GadgetManager)
					return false;
			}

			SCR_GadgetComponent gadgetComp = m_GadgetManager.GetHeldGadgetComponent();
			if (!gadgetComp || gadgetComp.GetType() != m_eRequiredGadget)
				return false;

			if (m_eRequiredGadget == EGadgetType.BUILDING_TOOL)
			{
				SCR_CampaignBuildingGadgetToolComponent etoolComp = SCR_CampaignBuildingGadgetToolComponent.Cast(gadgetComp);
				if (etoolComp && etoolComp.GetIsPlacementModeEnabled())
					return false;
			}
		}

		return super.CanBeShownScript(user);
	}

	//------------------------------------------------------------------------------------------------
	// The user action can be performed, when player has a building tool in hands.
	override bool CanBePerformedScript(IEntity user)
	{	
		SCR_GadgetComponent gadgetComponent = m_GadgetManager.GetHeldGadgetComponent();
		if (!gadgetComponent)
			return false;

		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		CharacterControllerComponent charController = character.GetCharacterController();

		if (charController && !IsInProgress() && !charController.CanUseItem())
			return false;

		return gadgetComponent.GetMode() == EGadgetMode.IN_HAND;
	}

	//------------------------------------------------------------------------------------------------
	//! Stop the action and animations associated with it
	//! \param[in] character
	protected void StopAction(notnull ChimeraCharacter character)
	{
		if (SCR_PlayerController.GetLocalControlledEntity() != character)
			return;

		CharacterControllerComponent charController = character.GetCharacterController();
		if (!charController)
			return;

		CharacterAnimationComponent animationComponent = charController.GetAnimationComponent();
		if (!animationComponent)
			return;

		CharacterCommandHandlerComponent cmdHandler = animationComponent.GetCommandHandler();
		if (cmdHandler)
			cmdHandler.FinishItemUse(true);

		RplComponent rplComp = character.GetRplComponent();

		// remove lock
		if (rplComp && rplComp.IsOwner())
		{
			PlayerController playerController = SCR_PlayerController.s_pLocalPlayerController;
			if (!playerController)
				return;

			SCR_FoliageDestructionComponent foliageDestructionComp = SCR_FoliageDestructionComponent.Cast(playerController.FindComponent(SCR_FoliageDestructionComponent));
			if (foliageDestructionComp)
				foliageDestructionComp.SetIsFoliageLocked(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	// Destructor
	void ~SCR_CutFoliageAction()
	{
		if (!m_ActiveUser)
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(m_ActiveUser);
		if (!character)
			return;

		StopAction(character);
	}
}