class SCR_BaseUseSupportStationAction : SCR_ScriptedUserAction
{
	[Attribute("#AR-SupportStation_ActionInvalid_NotInRange", desc: "No support stations of type in range", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidRangeActionReason;

	[Attribute("#AR-SupportStation_ActionInvalid_Destroyed", desc: "Support station in range but the only one is destroyed", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidDestroyedActionReason;

	[Attribute("#AR-SupportStation_ActionInvalid_HostileFaction", desc: "There are support stations in range but user is an invalid faction (Generally means faction is hostile)", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidFactionActionReason;

	[Attribute("#AR-SupportStation_ActionInvalid_Supplies", desc: "There are support stations in range but non of them have enough supplies", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidSuppliesActionReason;

	[Attribute("#AR-SupportStation_ActionInvalid_NoSupplyStorage", desc: "There are support stations in range but it cannot execute the action as the supply storages are all full", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidNoSupplyStorageSpaceActionReason;

	[Attribute("#AR-SupportStation_ActionInvalid_IsMoving", desc: "There are support stations in range but all of them are moving", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidMovingActionReason;

	[Attribute("#AR-SupportStation_ActionInvalid_Disabled", desc: "There are support stations use range and are in range but all of them are disabled (by the game master)", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidDisabledActionReason;

	[Attribute("1", desc: "Show supplies in action if there is a supply cost")]
	protected bool m_bShowSupplyCostOnAction;

	[Attribute("0", desc: "If true will check if support station component can be found on children. Only enable if support station is on Vehicle part and support station does not have range")]
	protected bool m_bAllowCheckChildrenSupportStation;

	[Attribute("-1", desc: "What animation the player should play if using the support station with a gadget. Leave -1 to not play any animation")]
	protected int m_iGadgetAnimationCommand;

	[Attribute("1", desc: "If an animation is assigned, does the animation loop?")]
	protected bool m_bDoesGadgetAnimationLoop;

	[Attribute("0", desc: "If an animation is assigned, does the animation play while the character is in a vehicle?")]
	protected bool m_bAnimateGadgetInVehicle;

	protected bool m_bIsMaster;
	protected bool m_bActionActive;

	protected IEntity m_ActionUser; //~ The player that uses is currently using the action
	protected SCR_BaseSupportStationComponent m_ActionOwnerSupportStationComponent;
	protected SCR_BaseSupportStationComponent m_SupportStationComponent;

	//~ To save performance it will only look for valid support station every x miliseconds or if no current supply station or if current is no longer valid
	protected BaseWorld m_World;
	protected float m_fLastCheckedCanPerform = 0;
	protected bool m_bSupportStationWasNull = false;
	protected const int DELAY_CAN_PERFORM = 500; //~ In milisecs if player hovers over action it will check every x miliseconds the highest priority Support station, Else it will simply check if the current is still valid

	protected int m_iSuppliesOnUse;
	protected int m_iAvailableSupplies;
	protected bool m_bCanPerform;
	protected ESupportStationReasonInvalid m_eCannotPerformReason;

	protected SCR_SupportStationGadgetComponent m_SupportStationGadget;
	protected bool m_bOnActionAnimationInDone;

	protected const LocalizedString ACTION_WITH_SUPPLYCOST_FORMATTING = "#AR-ActionFormat_SupplyCost";
	protected const LocalizedString ACTION_WITH_SUPPLYGAIN_FORMATTING = "#AR-ActionFormat_SupplyGain";
	protected const LocalizedString ACTION_WITH_PARAM_FORMATTING = "#AR-SupportStation_ActionFormat_WithParam";
	protected const LocalizedString ACTION_PERCENTAGE_FORMATTING = "#AR-SupportStation_ActionFormat_Percentage";

	//------------------------------------------------------------------------------------------------
	protected ESupportStationType GetSupportStationType()
	{
		Print("Do not use 'SCR_BaseUseSupportStationAction' as action, inherit from it and override ' SupportStationActionType' function to the correct type!", LogLevel.ERROR);
		return ESupportStationType.NONE;
	}

	//------------------------------------------------------------------------------------------------
	//~ Executed on perform (server only) and on CanBePerformedScript every time DELAY_CAN_PERFORM passes (Local only)
	protected SCR_BaseSupportStationComponent GetClosestValidSupportStation(IEntity actionOwner, IEntity actionUser, out ESupportStationReasonInvalid reasonInvalid = 0)
	{
		SCR_BaseSupportStationComponent supportStationComponent;
		vector actionWorldPosition = GetWorldPositionAction();

		//~ Should always take held gadget over any other support station unless the user holds no Gadget
		if (m_SupportStationGadget && PrioritizeHeldGadget() && AllowGetSupportStationFromGadget())
		{
			supportStationComponent = m_SupportStationGadget.GetSupportStation(GetSupportStationType());
			if (supportStationComponent)
			{
				//~ Held Gadget is valid
				if (supportStationComponent.IsValid(actionOwner, actionUser, this, actionWorldPosition, reasonInvalid, m_iSuppliesOnUse))
					return supportStationComponent;
				//~ The held gadget wasn't valid
				else
				{
					m_iAvailableSupplies = supportStationComponent.GetMaxAvailableSupplies();
					return null;
				}

			}
		}

		//~ Only execute without manager if its owner has a SCR_BaseSupportStationComponent on it that must be executed without manager
		if (m_ActionOwnerSupportStationComponent && !m_ActionOwnerSupportStationComponent.UsesRange() && !m_ActionOwnerSupportStationComponent.IgnoreSelf())
		{
			if (m_ActionOwnerSupportStationComponent.IsValid(actionOwner, actionUser, this, actionWorldPosition, reasonInvalid, m_iSuppliesOnUse))
				return m_ActionOwnerSupportStationComponent;

			m_iAvailableSupplies = m_ActionOwnerSupportStationComponent.GetMaxAvailableSupplies();
			return null;
		}

		//~ Get manager
		SCR_SupportStationManagerComponent supportStationManager = SCR_SupportStationManagerComponent.GetInstance();
		if (supportStationManager)
		{
			//~ If not get First valid SCR_BaseSupportStationComponent
			supportStationComponent = supportStationManager.GetClosestValidSupportStation(GetSupportStationType(), actionUser, actionOwner, this, actionWorldPosition, reasonInvalid, m_iSuppliesOnUse);
			if (supportStationComponent)
				return supportStationComponent;
		}

		//~ Only check held gadget if other support stations around at least did not fail because there were no supplies
		if (reasonInvalid != ESupportStationReasonInvalid.NO_SUPPLIES)
		{
			//~ Get held Gadget support station if it did not already check it because it had priority
			if (m_SupportStationGadget && !PrioritizeHeldGadget() && AllowGetSupportStationFromGadget())
			{
				supportStationComponent = m_SupportStationGadget.GetSupportStation(GetSupportStationType());
				if (supportStationComponent)
				{
					//~ Held Gadget is valid
					if (supportStationComponent.IsValid(actionOwner, actionUser, this, actionWorldPosition, reasonInvalid, m_iSuppliesOnUse))
						return supportStationComponent;
					else
					{
						m_iAvailableSupplies = supportStationComponent.GetMaxAvailableSupplies();
						return null;
					}
				}
			}
		}

		if (supportStationComponent)
			m_iAvailableSupplies = supportStationComponent.GetMaxAvailableSupplies();
		else
			m_iAvailableSupplies = 0;

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!GetOwner() || !super.CanBeShownScript(user))
			return false;

		m_SupportStationGadget = SCR_SupportStationGadgetComponent.GetHeldSupportStationGadget(GetSupportStationType(), user);
		if (RequiresGadget() && !m_SupportStationGadget)
			return false;

		//~ Don't show if support station is on owner and it is disabled
		SCR_BaseSupportStationComponent supportStationComponent = SCR_BaseSupportStationComponent.Cast(GetOwner().FindComponent(SCR_BaseSupportStationComponent));
		if (supportStationComponent && !supportStationComponent.UsesRange() && !supportStationComponent.IsEnabled())
			return false;

		//~ Don't show if fully destroyed (vehicle parts will check parent)
		if (!CanShowDestroyed())
			return false;

		//~ All checks passed
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanShowDestroyed()
	{
		//~ Don't show if damage manager destroyed
		SCR_DamageManagerComponent damageComponent = SCR_DamageManagerComponent.Cast(GetOwner().FindComponent(SCR_DamageManagerComponent));
		if (damageComponent)
			return damageComponent.GetState() != EDamageState.DESTROYED;

		//~ Don't show if default hitZone is destroyed
		HitZoneContainerComponent hitZoneContainer = HitZoneContainerComponent.Cast(GetOwner().FindComponent(HitZoneContainerComponent));
		if (hitZoneContainer)
		{
			HitZone defaultHitZone = hitZoneContainer.GetDefaultHitZone();

			if (defaultHitZone)
				return defaultHitZone.GetDamageState() != EDamageState.DESTROYED;
		}

		return true;
	}

	//---- REFACTOR NOTE START: CanBePerformedScript is updated each fix frame by code. The can perform check however is quite expensive to check so I add delays to only check x seconds ----
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		//~ Owner can be deleted so make sure the rest of CanBePerformed is not called
		if (!GetOwner())
			return false;

		//~ To optimize only get perform if action is active
		//if (!m_bActionActive && !m_bIsMaster)
		//	return true;

		//~ Only one player can perform at the same time
		if (!CanHaveMultipleUsers() && m_ActionUser && m_ActionUser != user)
		{
			SetCanPerform(false, ESupportStationReasonInvalid.IN_USE);
			return false;
		}

		ESupportStationReasonInvalid reasonInvalid;
		//~ Only check every x seconds if support station is still valid (valid support station is also checked on execute action). This is for performance issue as checking for all support stations can be performance heavy
		if (m_World.GetWorldTime() < (m_fLastCheckedCanPerform + DELAY_CAN_PERFORM))
		{
			//~ Support station assigned and still valid. So action can be performed
			if (m_SupportStationComponent && m_SupportStationComponent.IsValid(GetOwner(), user, this, GetWorldPositionAction(), reasonInvalid, m_iSuppliesOnUse))
			{
				SetCanPerform(true, -1);
				return true;
			}
			//~ No support station was valid but only recheck once DELAY_CAN_PERFORM if it is valid again. So make sure action cannot be performed
			else if (m_bSupportStationWasNull)
			{
				SetCanPerform(false, -1);
				return false;
			}
		}

		//~ Support station is no longer valid so check for a new one
		m_SupportStationComponent = GetClosestValidSupportStation(GetOwner(), user, reasonInvalid);
		if (m_SupportStationComponent)
			m_iAvailableSupplies = m_SupportStationComponent.GetMaxAvailableSupplies();

		//~ Only called when updated
		CanBePerformedUpdate(user);

		//~ Save if support station was null to make sure it only checks for valid support station after the delay
		m_bSupportStationWasNull = m_SupportStationComponent == null;

		SetCanPerform(m_SupportStationComponent != null, reasonInvalid);
		return m_SupportStationComponent != null;
	}
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	//~ Set if action can be performed and to update displayed action name and invalid reason
	protected void SetCanPerform(bool canPerform, ESupportStationReasonInvalid reasonInvalid)
	{
		m_bCanPerform = canPerform;

		if (reasonInvalid >= 0)
		{
			m_eCannotPerformReason = reasonInvalid;
			m_sCannotPerformReason = GetInvalidPerformReasonString(reasonInvalid);
		}
	}

	//------------------------------------------------------------------------------------------------
	bool CanPerform(out ESupportStationReasonInvalid cannotPerformReason)
	{
		cannotPerformReason = m_eCannotPerformReason;
		return m_bCanPerform;
	}

	//------------------------------------------------------------------------------------------------
	//~ Executed every time can be performed delay is done
	protected void CanBePerformedUpdate(IEntity user)
	{
		//~ Save last performed
		m_fLastCheckedCanPerform = m_World.GetWorldTime();
	}

	//------------------------------------------------------------------------------------------------
	//~ Get invalid perform reason text
	protected LocalizedString GetInvalidPerformReasonString(ESupportStationReasonInvalid reasonInvalid)
	{
		//~ No stations of type in range
		if (reasonInvalid == ESupportStationReasonInvalid.NOT_IN_RANGE)
			return m_sInvalidRangeActionReason;
		if (reasonInvalid == ESupportStationReasonInvalid.DESTROYED_STATION)
			return m_sInvalidDestroyedActionReason;
		//~ Disabled by Editor
		else if (reasonInvalid == ESupportStationReasonInvalid.DISABLED)
			return m_sInvalidDisabledActionReason;
		//~ Faction of user is invalid compaired to faction of Support station
		else if (reasonInvalid == ESupportStationReasonInvalid.INVALID_FACTION)
			return m_sInvalidFactionActionReason;
		//~ Stations have no supplies
		else if (reasonInvalid == ESupportStationReasonInvalid.NO_SUPPLIES)
			return m_sInvalidSuppliesActionReason;
		//~ Stations are unable to store any more supplies
		else if (reasonInvalid == ESupportStationReasonInvalid.NO_SUPPLY_STORAGE_SPACE)
			return m_sInvalidNoSupplyStorageSpaceActionReason;
		//~ Stations are all moving
		else if (reasonInvalid == ESupportStationReasonInvalid.IS_MOVING)
			return m_sInvalidMovingActionReason;

		//~ Show default unavailible
		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	protected void ResetReferencesOnServer()
	{
		m_fLastCheckedCanPerform = 0;
		m_SupportStationComponent = null;
		m_bSupportStationWasNull = false;
	}

	//------------------------------------------------------------------------------------------------
	//~ If continues action it will only execute everytime the duration is done
	override void PerformContinuousAction(IEntity pOwnerEntity, IEntity pUserEntity, float timeSlice)
	{
		if (!m_bIsMaster && SCR_PlayerController.GetLocalControlledEntity() != pUserEntity)
			return;

		if (!LoopActionUpdate(timeSlice))
			return;

		//~ Has gadget and loops animation
		if (m_SupportStationGadget && m_iGadgetAnimationCommand > 0 && m_bDoesGadgetAnimationLoop)
		{
			//~ Character animates in vehicle or is not in vehicle
			if (m_bAnimateGadgetInVehicle || !IsUserInVehicle(pUserEntity))
			{
				//~ Not yet in loop animation
				if (!m_SupportStationGadget.InUseAnimationLoop(pUserEntity))
					return;
			}
		}

		//~ Only start showing progress after first action completed
		m_bLoopAction = true;

		PerformAction(pOwnerEntity, pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		//~ Action can be performed server only
		if (!m_bIsMaster)
		{
			if (!m_bLoopAction) // if we arent looping, then whatever will happen, action user is no longer valid
				m_ActionUser = null;

			return;
		}

		//~ Resets all references so server grabs them all fresh
		ResetReferencesOnServer();

		if (!CanBeShownScript(pUserEntity))
		{
			if (!m_bLoopAction)
				m_ActionUser = null;

			return;
		}

		if (!CanBePerformedScript(pUserEntity))
		{
			if (!m_bLoopAction)
				m_ActionUser = null;

			return;
		}

		//Execute Supply station
		m_SupportStationComponent.OnExecutedServer(pOwnerEntity, pUserEntity, this);

		//~ Update supplies
		m_iAvailableSupplies = m_SupportStationComponent.GetMaxAvailableSupplies();
		if (!m_bLoopAction)
			m_ActionUser = null;
	}

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if ((gameMode && gameMode.IsMaster()) || (!gameMode && Replication.IsServer()))
			m_bIsMaster = true;

		m_World = GetGame().GetWorld();

		//~ Init check
		GetSupportStationType();

		if (ShouldPerformPerFrame() && GetActionDuration() > 0)
			Print("'SCR_BaseUseSupportStationAction' Support Station action type: '" + typename.EnumToString(ESupportStationType, GetSupportStationType()) + "' has PerformPerFrame true but the duraction is greater than 0. Set it to less that 0, eg: 1 sec == -1", LogLevel.ERROR);

		//~ Delay init to get components correctly
		GetGame().GetCallqueue().CallLater(DelayedInit, param1: pOwnerEntity);
	}

	//------------------------------------------------------------------------------------------------
	protected void DelayedInit(IEntity owner)
	{
		if (!owner)
			return;

		m_ActionOwnerSupportStationComponent = SCR_BaseSupportStationComponent.FindSupportStation(owner, GetSupportStationType(), queryFlags: SCR_EComponentFinderQueryFlags.ENTITY);

		//~ Get support station of children
		if (!m_ActionOwnerSupportStationComponent && m_bAllowCheckChildrenSupportStation)
		{
			if (Vehicle.Cast(owner))
				m_ActionOwnerSupportStationComponent = SCR_BaseSupportStationComponent.FindSupportStation(owner, GetSupportStationType(), queryFlags: SCR_EComponentFinderQueryFlags.SLOTS);
			else
				m_ActionOwnerSupportStationComponent = SCR_BaseSupportStationComponent.FindSupportStation(owner, GetSupportStationType(), queryFlags: SCR_EComponentFinderQueryFlags.CHILDREN);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBroadcastScript()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanHaveMultipleUsers()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! If not -1 it will show the percentage on the current action
	protected float GetActionPercentage()
	{
		return int.MIN;
	}

	//------------------------------------------------------------------------------------------------
	protected int GetActionDecimalCount()
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! If true will only show the action if the player is holding the correct support station gadget
	protected bool RequiresGadget()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! If true will always take support station from held gadget and skip any other checks if a held support station is found. RequiresGadget() is not required to be true
	protected bool PrioritizeHeldGadget()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! If true it allows getting the support station from gadgets
	protected bool AllowGetSupportStationFromGadget()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Get which entity is currently activly using the action
	protected IEntity GetCurrentActionUser()
	{
		return m_ActionUser;
	}

	//-----------------------------------------------------------------------------------
	int GetSupportStationSuppliesOnUse()
	{
		return m_iSuppliesOnUse;
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		m_ActionUser = pUserEntity;

		super.OnActionStart(pUserEntity);

		//~ Or if AI just do it
		if (!ShouldPerformPerFrame())
			return;

		m_SupportStationGadget = SCR_SupportStationGadgetComponent.GetHeldSupportStationGadget(GetSupportStationType(), pUserEntity);
		if (!m_SupportStationGadget)
		{
			m_bLoopAction = true;
			return;
		}
		else if (!m_bDoesGadgetAnimationLoop)
		{
			m_bLoopAction = true;

			if (m_iGadgetAnimationCommand > 0 && (m_bAnimateGadgetInVehicle || !IsUserInVehicle(pUserEntity)))
				m_SupportStationGadget.StartGadgetAnimation(pUserEntity, m_iGadgetAnimationCommand, this);

			return;
		}

		m_bLoopAction = false;

		if (m_iGadgetAnimationCommand > 0 && (m_bAnimateGadgetInVehicle || !IsUserInVehicle(pUserEntity)))
			m_SupportStationGadget.StartGadgetAnimation(pUserEntity, m_iGadgetAnimationCommand, this);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsUserInVehicle(notnull IEntity user)
	{
		ChimeraCharacter chimeraCharacter = ChimeraCharacter.Cast(user);
		return chimeraCharacter && chimeraCharacter.IsInVehicle();
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		super.OnActionCanceled(pUserEntity);

		//~ Remove gadget user
		m_ActionUser = null;

		if (!m_SupportStationGadget)
			return;

		if (m_bDoesGadgetAnimationLoop && m_iGadgetAnimationCommand > 0 && (m_bAnimateGadgetInVehicle || !IsUserInVehicle(pUserEntity)))
			m_SupportStationGadget.StopGadgetAnimation(pUserEntity, m_iGadgetAnimationCommand);
	}

	//------------------------------------------------------------------------------------------------
	protected string GetActionStringParam()
	{
		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnActionSelected()
	{
		super.OnActionSelected();
		m_fLastCheckedCanPerform = 0;
		m_bActionActive = true;

	}

	//------------------------------------------------------------------------------------------------
	protected override void OnActionDeselected()
	{
		super.OnActionSelected();
		m_bActionActive = false;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		//~ Support station is overwriting the action name
		if (outName.IsEmpty())
		{
			if (m_SupportStationComponent)
				outName = m_SupportStationComponent.GetOverrideUserActionName();

			if (outName.IsEmpty())
			{
				UIInfo uiInfo = GetUIInfo();
				if (uiInfo)
					outName = uiInfo.GetName();
			}
		}

		//~ If action is active show additional params
		if (m_bActionActive)
		{
			string actionParam = GetActionStringParam();
			float actionPercentage = int.MIN;

			if (actionParam.IsEmpty())
				actionPercentage = GetActionPercentage();

			//~ Get percentage string
			if (actionPercentage != int.MIN)
				actionParam = SCR_FormatHelper.FloatToStringNoZeroDecimalEndings(actionPercentage, GetActionDecimalCount());

			//~ Show percentage
			if (!actionParam.IsEmpty())
			{
				if (actionPercentage != int.MIN)
					outName = WidgetManager.Translate(ACTION_PERCENTAGE_FORMATTING, outName, actionParam);
				else
					outName = WidgetManager.Translate(ACTION_WITH_PARAM_FORMATTING, outName, actionParam);
			}

			//~ Show supply cost
			if (m_bShowSupplyCostOnAction && m_iSuppliesOnUse > 0 && m_SupportStationComponent)
			{
				//~ If has supply cost
				if (!m_SupportStationComponent.HasSupplyGainInsteadOfCost(this))
				{
					//~ Show if can perform or if not enough supplies
					if (m_bCanPerform || m_eCannotPerformReason == ESupportStationReasonInvalid.NO_SUPPLIES)
						outName = WidgetManager.Translate(ACTION_WITH_SUPPLYCOST_FORMATTING, outName, m_iSuppliesOnUse, m_iAvailableSupplies);
				}
				//~ If has supply gain
				else
				{
					//~ Show if can perform or if not enough storage for supply gain supplies
					if (m_bCanPerform || m_eCannotPerformReason == ESupportStationReasonInvalid.NO_SUPPLY_STORAGE_SPACE)
						outName = WidgetManager.Translate(ACTION_WITH_SUPPLYGAIN_FORMATTING, outName, m_iSuppliesOnUse, m_iAvailableSupplies);
				}


			}
		}

		return !outName.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_BaseUseSupportStationAction()
	{
		//~ Safety if action owner is deleted while player is still using it
		if (m_ActionUser && m_SupportStationGadget)
		{
			if (m_bDoesGadgetAnimationLoop && m_iGadgetAnimationCommand > 0 && (m_bAnimateGadgetInVehicle || !IsUserInVehicle(m_ActionUser)))
				m_SupportStationGadget.StopGadgetAnimation(m_ActionUser, m_iGadgetAnimationCommand);
		}
	}
}

