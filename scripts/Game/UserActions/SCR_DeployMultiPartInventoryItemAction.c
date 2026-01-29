class SCR_DeployMultiPartInventoryItemAction : SCR_DeployInventoryItemAction
{
	[Attribute(defvalue: "0", desc: "Id of a variant that this action is meant to deploy.", params: "0 inf")]
	protected int m_iVariantId;

	[Attribute(desc: "If game should check if ground on the deploy position is not tilted too much for given deployable variant")]
	protected bool m_bCheckTilt;

	[Attribute(desc: "If game should check if this place has enough space to fit this entity")]
	protected bool m_bCheckForAvailableSpace;

	[Attribute(desc: "If game should check if storage of this is entity is empty, and in case that it is not, then prevent the player from using this action.")]
	protected bool m_bCheckStorage;

	[Attribute(desc: "If game should check if this place doesnt have a roof above it")]
	protected bool m_bPreventIfIndoors;

	[Attribute(desc: "If game should try to render the visualisation of the replacment prefab")]
	protected bool m_bShowVisualisation;

	[Attribute(defvalue: "0", desc: "Dictates if more than one user at a time can use this action")]
	protected bool m_bCanBePerformedByMultipleUsers;

	[Attribute(defvalue: "0", desc: "Should the effect of this action be considered as dismantled")]
	protected bool m_bReportAsDismantling;

	[Attribute(EGadgetType.NONE.ToString(), uiwidget: UIWidgets.ComboBox, desc: "What gadget player has to equip in order to be able to use this action", enumType: EGadgetType)]
	protected EGadgetType m_eRequiredGadget;

	protected SCR_GadgetManagerComponent m_GadgetManager;
	protected BaseInventoryStorageComponent m_Storage;
	protected vector m_vDesiredDirection;
	protected ChimeraCharacter m_CurrentUser;
	protected bool m_bIsSelected;
	protected bool m_bLastCanPerformResult = true;
	protected WorldTimestamp m_NextCheckTimestamp;

	protected const LocalizedString CANNOT_PERFORM_REASON_TOO_STEEP = "#AR-UserAction_Assemble_TooSteep";
	protected const LocalizedString CANNOT_PERFORM_REASON_NOT_EMPTY = "#AR-UserAction_Assemble_NotEmpty";
	protected const LocalizedString CANNOT_PERFORM_REASON_NOT_INDOORS = "#AR-UserAction_Assemble_CannotPlaceHere";
	protected const LocalizedString CANNOT_PERFORM_REASON_ALREADY_IN_USE = "#AR-UserAction_Blocked_InUseByOther";

	//! delay in ms used to control how often costly parts are executed
	protected const int VALIDATION_DELAY = 500;

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);

		m_Storage = BaseInventoryStorageComponent.Cast(pOwnerEntity.FindComponent(BaseInventoryStorageComponent));
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		IEntity owner = GetOwner();
		if (!owner || owner.GetParent())
			return false;

		if (CompartmentAccessComponent.GetVehicleIn(user))
			return false;

		if (m_eRequiredGadget != EGadgetType.NONE)
		{
			m_GadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(user);
			if (!m_GadgetManager)
				return false;

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

		Vehicle veh = Vehicle.Cast(owner);
		if (veh)
		{
			if (veh.IsOccupied())
				return false;
		}
		else
		{
			SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(owner.FindComponent(SCR_BaseCompartmentManagerComponent));
			if (compartmentManager && compartmentManager.GetOccupantCount() > 0)
				return false;
		}

		return super.CanBeShownScript(user);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return false;

		if (!m_bIsSelected)
			return m_bLastCanPerformResult;

		if (!m_bCanBePerformedByMultipleUsers && m_DeployableItemComp.IsDeploying() && m_CurrentUser != user)
		{
			SetCannotPerformReason(CANNOT_PERFORM_REASON_ALREADY_IN_USE);
			return false;
		}

		SCR_MultiPartDeployableItemComponent multiPartComp = SCR_MultiPartDeployableItemComponent.Cast(m_DeployableItemComp);
		if (!multiPartComp)
			return false;

		if (multiPartComp.GetCurrentlyCachedVariantId() != m_iVariantId)
			multiPartComp.FetchVariantData(m_iVariantId);

		vector angle;
		vector transform[4];
		if (multiPartComp.IsUsingPartPositionAndRotation())
		{
			owner.GetTransform(transform);
		}
		else
		{
			TraceParam param = new TraceParam();
			array<IEntity> excludeArray = {owner, user};
			param.ExcludeArray = excludeArray;
			param.LayerMask = EPhysicsLayerPresets.Projectile;

			vector position = owner.GetOrigin() + owner.VectorToParent(multiPartComp.GetAdditionaPlacementOffset()) + vector.Up * 0.1;
			SCR_TerrainHelper.GetTerrainBasis(position, transform, owner.GetWorld(), false, param);

			ChimeraCharacter character = ChimeraCharacter.Cast(user);
			if (!character)
				return false;

			CharacterControllerComponent controller = character.GetCharacterController();
			if (!controller)
				return false;

			CharacterHeadAimingComponent headAiming = controller.GetHeadAimingComponent();
			if (!headAiming)
				return false;

			vector charMat[4];
			character.GetWorldTransform(charMat);
			angle = headAiming.GetLookAngles();
			angle = angle.AnglesToVector(); //Converts spherical coordinates to unit length vector
			angle = angle.Multiply3(charMat); //Translate from local space to world space
			angle = angle.VectorToAngles(); //Convert back to angles
			angle = angle + multiPartComp.GetAdditionaPlacementRotation();
		}

		if (m_bShowVisualisation && !m_bActionStarted)
		{
			if (m_bIsSelected)
				multiPartComp.VisualizeReplacementEntity(transform, angle);
			else
				multiPartComp.SetPreviewState(SCR_EPreviewState.NONE);
		}

		WorldTimestamp currentTimestamp = owner.GetWorld().GetTimestamp();
		if (m_NextCheckTimestamp && currentTimestamp.Less(m_NextCheckTimestamp))
			return m_bLastCanPerformResult;

		m_NextCheckTimestamp = currentTimestamp.PlusMilliseconds(VALIDATION_DELAY);
		m_bLastCanPerformResult = false;

		if (m_bCheckStorage && m_Storage)
		{
			array<IEntity> outItems = {};
			m_Storage.GetAll(outItems);
			if (!outItems.IsEmpty())
			{
				SetCannotPerformReason(CANNOT_PERFORM_REASON_NOT_EMPTY);
				return false;
			}
		}

		if (m_bCheckTilt && !multiPartComp.CheckSurfaceTilt(transform[1]))
		{
			SetCannotPerformReason(CANNOT_PERFORM_REASON_TOO_STEEP);
			return false;
		}

		string failReason;
		if (!multiPartComp.FindRequiredElements(failReason))
		{
			SetCannotPerformReason(failReason);
			return false;
		}

		if (m_bCheckForAvailableSpace && !multiPartComp.CheckAvailableSpace(transform, angle, failReason))
		{
			SetCannotPerformReason(failReason);
			return false;
		}

		if (m_bPreventIfIndoors && multiPartComp.CheckIfPositionIsIndoors(transform))
		{
			SetCannotPerformReason(CANNOT_PERFORM_REASON_NOT_INDOORS);
			return false;
		}

		if (m_bShowVisualisation && m_bIsSelected)
			multiPartComp.SetPreviewState(SCR_EPreviewState.PLACEABLE);

		m_bLastCanPerformResult = true;

		return m_bLastCanPerformResult;
	}

	//------------------------------------------------------------------------------------------------
	override void SetCannotPerformReason(string reason)
	{
		SCR_MultiPartDeployableItemComponent multiPartComp = SCR_MultiPartDeployableItemComponent.Cast(m_DeployableItemComp);
		if (multiPartComp)
		{
			if (m_bIsSelected)
				multiPartComp.SetPreviewState(SCR_EPreviewState.BLOCKED);
			else
				multiPartComp.SetPreviewState(SCR_EPreviewState.NONE);
		}

		super.SetCannotPerformReason(reason);
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		bool isAi;
		if (SCR_CharacterHelper.IsPlayerOrAIOwner(character, isAi))
			StopAction(character);

		GetGame().GetCallqueue().CallLater(ResetDeployingDelayed, 100, param1: pUserEntity); //reset bool later so there is enough time for user action to disappear
		RplComponent rplComp = SCR_EntityHelper.GetEntityRplComponent(pOwnerEntity);
		if (!rplComp || !rplComp.IsOwner())//ensure that only owner of the entity is doing the rest
			return;

		SCR_MultiPartDeployableItemComponent multiPartComp = SCR_MultiPartDeployableItemComponent.Cast(m_DeployableItemComp);
		if (!multiPartComp)
			return;

		//Do this again so other clients have the same data set
		multiPartComp.FetchVariantData(m_iVariantId);
		string failReason;
		if (!multiPartComp.FindRequiredElements(failReason))
			return;

		multiPartComp.Deploy(m_iVariantId, pUserEntity, m_vDesiredDirection, false, !m_bReportAsDismantling);
		OnConfirmed(pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		if (!m_DeployableItemComp)
			return;

		m_CurrentUser = ChimeraCharacter.Cast(pUserEntity);
		if (!m_CurrentUser)
			return;

		super.OnActionStart(pUserEntity);

		if (m_bReportAsDismantling)
			ToggleCompartmentAccess(pUserEntity);

		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;

		CharacterHeadAimingComponent headAiming = controller.GetHeadAimingComponent();
		if (!headAiming)
			return;

		vector charMat[4];
		character.GetWorldTransform(charMat);
		m_vDesiredDirection = headAiming.GetLookAngles();
		m_vDesiredDirection = m_vDesiredDirection.AnglesToVector(); //Converts spherical coordinates to unit length vector
		m_vDesiredDirection = m_vDesiredDirection.Multiply3(charMat); //Translate from local space to world space
		m_vDesiredDirection = m_vDesiredDirection.VectorToAngles(); //Convert back to angles

		if (!m_bActionStarted)
			return;//this is set only for the player who is doing the action

		if (m_eRequiredGadget == EGadgetType.NONE)
			return;

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
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to lock or unlock all compartments by reserving them for the player who performs the action
	//! \param[in] blockingUser for whom the compartments should be reserved
	protected void ToggleCompartmentAccess(IEntity blockingUser = null)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;

		SCR_BaseCompartmentManagerComponent compartmentMgr = SCR_BaseCompartmentManagerComponent.Cast(owner.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compartmentMgr)
			return;

		array<BaseCompartmentSlot> outCompartments = {};
		if (compartmentMgr.GetCompartments(outCompartments) < 1)
			return;

		foreach (BaseCompartmentSlot compartment : outCompartments)
		{
			compartment.SetReserved(blockingUser);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		super.OnActionCanceled(pOwnerEntity, pUserEntity);

		if (m_bReportAsDismantling)
			ToggleCompartmentAccess();

		m_vDesiredDirection = vector.Zero;
		if (m_eRequiredGadget == EGadgetType.NONE)
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		StopAction(character);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		UIInfo info = GetUIInfo();
		if (!info)
			return false;

		outName = WidgetManager.Translate(info.GetName(), info.GetDescription());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] character
	void StopAction(notnull ChimeraCharacter character)
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
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		m_bIsSelected = true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		m_NextCheckTimestamp = null;
		if (m_bIsSelected && m_bActionStarted && m_CurrentUser)
			StopAction(m_CurrentUser);

		m_bIsSelected = false;
		SCR_MultiPartDeployableItemComponent multiPartComp = SCR_MultiPartDeployableItemComponent.Cast(m_DeployableItemComp);
		if (!multiPartComp)
			return;

		if (m_bShowVisualisation)
			multiPartComp.SetPreviewState(SCR_EPreviewState.NONE);

		multiPartComp.ClearCache();
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_DeployMultiPartInventoryItemAction()
	{
		if (m_bIsSelected && m_bActionStarted && m_CurrentUser)
			StopAction(m_CurrentUser);

		m_bIsSelected = false;
		SCR_MultiPartDeployableItemComponent multiPartComp = SCR_MultiPartDeployableItemComponent.Cast(m_DeployableItemComp);
		if (!multiPartComp)
			return;

		if (multiPartComp.GetCurrentlyCachedVariantId() != m_iVariantId)
			return;

		if (m_bShowVisualisation)
			multiPartComp.SetPreviewState(SCR_EPreviewState.NONE);

		multiPartComp.ClearCache();
	}
}
