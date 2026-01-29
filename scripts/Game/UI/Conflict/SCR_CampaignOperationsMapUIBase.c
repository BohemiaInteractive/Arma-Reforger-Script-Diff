class SCR_CampaignOperationsMapUIBase : SCR_CampaignMapUIBase
{
	protected const int EVALUATE_BUILD_MODE_AVAILABILITY_DELAY = 3000;

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (m_Base)
		{
			// Periodically check for availability of build mode on selected base
			EvaluateBuildModeAvailability();
			GetGame().GetCallqueue().CallLater(EvaluateBuildModeAvailability, EVALUATE_BUILD_MODE_AVAILABILITY_DELAY, true);
		}

		return super.OnMouseEnter(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (m_Base)
			GetGame().GetCallqueue().Remove(EvaluateBuildModeAvailability);

		return super.OnMouseLeave(w, enterW, x, y);
	}

	//------------------------------------------------------------------------------------------------
	override protected void DeinitOpenButton(Widget w)
	{
		super.DeinitOpenButton(w);

		GetGame().GetCallqueue().Remove(EvaluateBuildModeAvailability);
	}

	//------------------------------------------------------------------------------------------------
	//! Enables or disables button for entering the build mode
	//! \param[in] enable
	protected void SetOpenButtonEnabled(bool enable)
	{
		if (m_OpenInputButtonComponent)
			m_OpenInputButtonComponent.SetEnabled(enable);
	}

	//------------------------------------------------------------------------------------------------
	//! Checks all entities in vicinity of the base and if any entities blocking the build mode in the base are found. Enter build mode button is disabled.
	protected void EvaluateBuildModeAvailability()
	{
		if (!m_Base)
			return;

		array<SCR_CampaignBuildingProviderComponent> buildingProviders = {};
		m_Base.GetBuildingProviders(buildingProviders);
		if (buildingProviders.IsEmpty())
			return;

		SCR_CampaignBuildingProviderComponent masterProvider;
		foreach (SCR_CampaignBuildingProviderComponent provider : buildingProviders)
		{
			if (provider.IsMasterProvider())
			{
				masterProvider = provider;
				break;
			}
		}

		if (!masterProvider)
			return;

		if (GetGame().GetWorld().QueryEntitiesBySphere(m_Base.GetOwner().GetOrigin(), masterProvider.GetBuildingRadius(), EvaluateEntity, null, EQueryEntitiesFlags.DYNAMIC))
			SetOpenButtonEnabled(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Check if this entity can block player to enter a building mode. If such an entity is found, return false to stop evaluating next entity found by query.
	//\param[in] entity to evaluate by this filter.
	bool EvaluateEntity(notnull IEntity entity)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(entity);
		if (!character)
			return true;

		SCR_CharacterDamageManagerComponent characterDamageManager = SCR_CharacterDamageManagerComponent.Cast(character.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!characterDamageManager || characterDamageManager.GetState() == EDamageState.DESTROYED)
			return true;

		if (!m_Base.GetFaction().IsFactionEnemy(character.GetFaction()))
			return true;

		CharacterControllerComponent charController = character.GetCharacterController();
		if (!charController)
			return true;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(entity);
		if (playerId == 0)
		{
			AIControlComponent aiControlComponent = charController.GetAIControlComponent();
			if (!aiControlComponent)
				return true;

			if (aiControlComponent.IsAIActivated())
			{
				SetOpenButtonEnabled(false);
				return false;
			}
		}
		else
		{
			SetOpenButtonEnabled(false);
			return false;
		}

		return true;
	}
}