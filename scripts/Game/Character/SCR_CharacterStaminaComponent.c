[ComponentEditorProps(category: "GameScripted/Character", description: "Stamina component for character.", icon: HYBRID_COMPONENT_ICON)]
class SCR_CharacterStaminaComponentClass : CharacterStaminaComponentClass
{
}

class SCR_CharacterStaminaComponent : CharacterStaminaComponent
{
	[Attribute("1", UIWidgets.EditBox, "Additional stamina drain multiplier at zero hydration, scaling linearly with dehydration. 2 means +200% drain (3x total). 0 disables the penalty.", params: "0 inf")]
	protected float m_fHydrationDrain;

	[Attribute("1800", UIWidgets.EditBox, "Define how long it takes to dehydrate fully in seconds. Set to 0 to disable dehydration.", "0 inf")]
	protected float m_fTimeOfDehydration;

	[Attribute("1", UIWidgets.CheckBox, "Show a dehydration warning when stamina is drained at or below the hydration warning threshold.")]
	protected bool m_bShowDehydrationWarning;

	[Attribute("50", UIWidgets.Slider, "Remaining hydration percentage at or below which the warning is shown. Does not change the stamina drain penalty.", params: "0 100 1")]
	protected float m_fDehydrationWarningThreshold;

	static const int STAMINA_REGENERATION_BOOST_UPDATE_INTERVAL = 50;
	static const float HYDRATION_MAXIMUM = 1;
	
	[RplProp(condition: RplCondition.OwnerOnly)]
	protected float m_fHydrationAtAnchor = HYDRATION_MAXIMUM;

	[RplProp(condition: RplCondition.OwnerOnly)]
	protected WorldTimestamp m_HydrationAnchorTimestamp;


	protected bool m_bApplyingHydrationPenalty;
	protected bool m_bDehydrationWarningShown;
	protected bool m_bStaminaRegenerationBoostTracking;
	protected float m_fStaminaBeforeBoostUpdate;
	protected SCR_FlaskStaminaRegenBoostDamageEffect m_StaminaRegenerationBoostEffect;

	//------------------------------------------------------------------------------------------------
	//! \return Whether the game mode enables hydration. Defaults to disabled when no game mode is available
	bool IsHydrationEnabled()
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		return gameMode && gameMode.GetHydrationRules() == SCR_EHydrationRules.ENABLED;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the current hydration level, accounting for elapsed dehydration time, or HYDRATION_MAXIMUM when the game mode disables hydration
	//! \return Hydration clamped between 0 and HYDRATION_MAXIMUM, where 0 means fully dehydrated and HYDRATION_MAXIMUM means fully hydrated
	float GetHydration()
	{
		if (!IsHydrationEnabled())
			return HYDRATION_MAXIMUM;

		if (m_fTimeOfDehydration <= 0)
			return Math.Clamp(m_fHydrationAtAnchor, 0, HYDRATION_MAXIMUM);

		IEntity owner = GetOwner();
		if (!owner)
			return Math.Clamp(m_fHydrationAtAnchor, 0, HYDRATION_MAXIMUM);

		ChimeraWorld world = owner.GetWorld();
		if (!world)
			return Math.Clamp(m_fHydrationAtAnchor, 0, HYDRATION_MAXIMUM);

		WorldTimestamp currentTimestamp = world.GetServerTimestamp();
		if (m_HydrationAnchorTimestamp == 0)
		{
			m_HydrationAnchorTimestamp = currentTimestamp;
			return Math.Clamp(m_fHydrationAtAnchor, 0, HYDRATION_MAXIMUM);
		}

		float elapsedSeconds = Math.Max(currentTimestamp.DiffMilliseconds(m_HydrationAnchorTimestamp), 0) * 0.001;

		return Math.Clamp(m_fHydrationAtAnchor - elapsedSeconds / m_fTimeOfDehydration, 0, HYDRATION_MAXIMUM);
	}

	//------------------------------------------------------------------------------------------------
	//! Raise current hydration by amount
	//! \param[in] amount
	//! \return How much hydration was recovered
	float ApplyHydrationRecovery(float amount)
	{
		if (amount <= 0)
			return 0;

		IEntity owner = GetOwner();
		if (!owner)
			return 0;

		RplComponent replication = SCR_EntityHelper.GetEntityRplComponent(owner);
		if (replication && replication.Role() != RplRole.Authority)
			return 0;

		ChimeraWorld world = owner.GetWorld();
		if (!world)
			return 0;

		float hydrationBefore = GetHydration();
		float hydrationAfter = Math.Clamp(hydrationBefore + amount, 0, HYDRATION_MAXIMUM);
		float appliedRecovery = Math.Max(hydrationAfter - hydrationBefore, 0);

		if (appliedRecovery > 0)
		{
			m_fHydrationAtAnchor = hydrationAfter;
			m_HydrationAnchorTimestamp = world.GetServerTimestamp();
			Replication.BumpMe();

			if (hydrationAfter >= HYDRATION_MAXIMUM || hydrationAfter * 100 > Math.Clamp(m_fDehydrationWarningThreshold, 0, 100))
				m_bDehydrationWarningShown = false;
		}

		return appliedRecovery;
	}

	//------------------------------------------------------------------------------------------------
	//! Immediately restores stamina
	//! \param[in] amount
	void ApplyImmediateStaminaRecovery(float amount)
	{
		if (amount <= 0)
			return;

		IEntity owner = GetOwner();
		if (!owner)
			return;

		RplComponent replication = SCR_EntityHelper.GetEntityRplComponent(owner);
		if (replication && replication.Role() != RplRole.Authority)
			return;

		AddStamina(amount);

		if (m_bStaminaRegenerationBoostTracking)
			m_fStaminaBeforeBoostUpdate = GetStamina();
	}

	//------------------------------------------------------------------------------------------------
	void StartStaminaRegenerationBoostTracking(notnull SCR_FlaskStaminaRegenBoostDamageEffect boostEffect)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;

		RplComponent replication = SCR_EntityHelper.GetEntityRplComponent(owner);
		if (replication && replication.Role() != RplRole.Authority)
			return;

		m_StaminaRegenerationBoostEffect = boostEffect;
		m_fStaminaBeforeBoostUpdate = GetStamina();

		if (m_bStaminaRegenerationBoostTracking)
			return;

		m_bStaminaRegenerationBoostTracking = true;
		
		// Poll stamina changes periodically because native regeneration does not trigger OnStaminaDrain
		GetGame().GetCallqueue().CallLater(UpdateStaminaRegenerationBoost, STAMINA_REGENERATION_BOOST_UPDATE_INTERVAL, true);
	}

	//------------------------------------------------------------------------------------------------
	void StopStaminaRegenerationBoostTracking(notnull SCR_FlaskStaminaRegenBoostDamageEffect boostEffect)
	{
		if (m_StaminaRegenerationBoostEffect != boostEffect)
			return;

		m_bStaminaRegenerationBoostTracking = false;
		m_StaminaRegenerationBoostEffect = null;
		GetGame().GetCallqueue().Remove(UpdateStaminaRegenerationBoost);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateStaminaRegenerationBoost()
	{
		if (!m_bStaminaRegenerationBoostTracking)
			return;

		if (!m_StaminaRegenerationBoostEffect || !m_StaminaRegenerationBoostEffect.IsActive())
		{
			m_bStaminaRegenerationBoostTracking = false;
			m_StaminaRegenerationBoostEffect = null;
			GetGame().GetCallqueue().Remove(UpdateStaminaRegenerationBoost);
			return;
		}

		IEntity owner = GetOwner();
		if (!owner)
		{
			m_bStaminaRegenerationBoostTracking = false;
			m_StaminaRegenerationBoostEffect = null;
			GetGame().GetCallqueue().Remove(UpdateStaminaRegenerationBoost);
			return;
		}

		RplComponent replication = SCR_EntityHelper.GetEntityRplComponent(owner);
		if (replication && replication.Role() != RplRole.Authority)
		{
			m_bStaminaRegenerationBoostTracking = false;
			m_StaminaRegenerationBoostEffect = null;
			GetGame().GetCallqueue().Remove(UpdateStaminaRegenerationBoost);
			return;
		}

		float currentStamina = GetStamina();
		float recoveredStamina = currentStamina - m_fStaminaBeforeBoostUpdate;
		m_fStaminaBeforeBoostUpdate = currentStamina;

		if (recoveredStamina <= 0)
			return;

		float bonus = recoveredStamina * m_StaminaRegenerationBoostEffect.GetRegenerationBonus();
		if (bonus <= 0)
			return;

		AddStamina(bonus);
		m_fStaminaBeforeBoostUpdate = GetStamina();
	}

	//------------------------------------------------------------------------------------------------
	override void OnStaminaDrain(float pDrain)
	{
		super.OnStaminaDrain(pDrain);
	
		if (pDrain >= 0 || m_bApplyingHydrationPenalty)
			return;
	
		IEntity owner = GetOwner();
		if (!owner)
			return;
	
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager || playerManager.GetPlayerIdFromControlledEntity(owner) <= 0)
			return;

		float drainReduction;
		if (m_StaminaRegenerationBoostEffect)
			drainReduction = Math.AbsFloat(pDrain * m_StaminaRegenerationBoostEffect.GetStaminaDrainReduction());

		float hydration = GetHydration();
		float hydrationDeficit = HYDRATION_MAXIMUM - hydration;
		if (hydrationDeficit <= 0)
		{
			if (drainReduction > 0)
				AddStamina(drainReduction);

			return;
		}
	
		float requestedPenalty = pDrain * hydrationDeficit * m_fHydrationDrain;
		if (requestedPenalty >= 0)
		{
			if (drainReduction > 0)
				AddStamina(drainReduction);

			return;
		}
	
		m_bApplyingHydrationPenalty = true;
		AddStamina(requestedPenalty + drainReduction);
		m_bApplyingHydrationPenalty = false;
	
		if (!m_bShowDehydrationWarning || m_bDehydrationWarningShown || hydration * 100 > Math.Clamp(m_fDehydrationWarningThreshold, 0, 100))
			return;
	
		RplComponent replication = SCR_EntityHelper.GetEntityRplComponent(owner);
		if (replication && !replication.IsOwner())
			return;
	
		// Notify only the stamina owner, once until hydration recovers above the threshold.
		if (owner == SCR_PlayerController.GetLocalControlledEntity())
			m_bDehydrationWarningShown = SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_HYDRATION_CRITICAL);
		else if (Replication.IsServer())
			m_bDehydrationWarningShown = SCR_NotificationsComponent.SendToPlayer(playerManager.GetPlayerIdFromControlledEntity(owner), ENotification.PLAYER_HYDRATION_CRITICAL);
	}
}

[BaseContainerProps()]
class StaminaEventParams
{
}

[BaseContainerProps()]
class MetabolismEventParams : StaminaEventParams
{
	[Attribute("0", UIWidgets.EditBox, "Defines how many calories will be added/subtracted when triggering OnStaminaDrain event.")]
	float 		m_fCaloriesDrain;

	[Attribute("0", UIWidgets.EditBox, "Defines how much hydration will be added/subtracted when triggering OnStaminaDrain event.")]
	float 		m_fHydrationDrain;

	[Attribute("0", UIWidgets.EditBox, "Defines how much energy will be added/subtracted when triggering OnStaminaDrain event.")]
	float 		m_fEnergyDrain;
}
