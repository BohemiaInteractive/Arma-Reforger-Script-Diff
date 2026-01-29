class SCR_CampaignBuildingStartUserAction : ScriptedUserAction
{
	[Attribute(desc: "Action name if supplies are disabled")]
	protected LocalizedString m_sActionNameNoSupplies;

	protected SCR_ResourceSystemSubscriptionHandleBase m_ResourceSubscriptionHandleConsumer;
	protected RplId m_ResourceInventoryPlayerComponentRplId;
	protected SCR_ResourceComponent m_ResourceComponent;
	protected SCR_ResourceConsumer m_ResourceConsumer;
	protected SCR_CampaignBuildingProviderComponent m_ProviderComponent;
	protected IEntity m_MainParent;
	protected RplComponent m_RplComponent;
	protected DamageManagerComponent m_DamageManager;
	protected SCR_CompartmentAccessComponent m_CompartmentAccess;
	protected bool m_bUseRankLimitedAccess;
	protected bool m_bTemporarilyBlockedAccess;
	protected bool m_bAccessCanBeBlocked;

	protected const int PROVIDER_SPEED_TO_REMOVE_BUILDING_SQ = 1;
	protected const int TEMPORARY_BLOCKED_ACCESS_RESET_TIME = 1000;

	//------------------------------------------------------------------------------------------------
	protected override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_RplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		InitializeSuppliesComponent();

		m_DamageManager = DamageManagerComponent.Cast(GetOwner().FindComponent(DamageManagerComponent));

		if (GetGame().GetPlayerController())
			m_ResourceInventoryPlayerComponentRplId = Replication.FindId(SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent)));

		if (m_ProviderComponent && m_ProviderComponent.ObstrucViewWhenEnemyInRange())
			m_bAccessCanBeBlocked = true;
	}

	//------------------------------------------------------------------------------------------------
	void InitializeSuppliesComponent()
	{
		m_MainParent = SCR_EntityHelper.GetMainParent(GetOwner(), true);
		m_ProviderComponent = SCR_CampaignBuildingProviderComponent.Cast(GetOwner().FindComponent(SCR_CampaignBuildingProviderComponent));
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
		m_ProviderComponent.RequestBuildingMode(playerID, true);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!m_ProviderComponent || m_bTemporarilyBlockedAccess)
			return false;

		if (SCR_XPHandlerComponent.IsXpSystemEnabled() && m_ProviderComponent.GetAccessRank() > GetUserRank(user))
		{
			FactionAffiliationComponent factionAffiliationComp = FactionAffiliationComponent.Cast(user.FindComponent(FactionAffiliationComponent));
			if (!factionAffiliationComp)
				return false;

			string rankName;
			SCR_Faction faction = SCR_Faction.Cast(factionAffiliationComp.GetAffiliatedFaction());
			if (faction)
				rankName = faction.GetRankName(m_ProviderComponent.GetAccessRank());

			SetCannotPerformReason(rankName);
			return false;
		}

		if (m_bAccessCanBeBlocked)
		{
			SetTemporaryBlockedAccess();

			if (m_bTemporarilyBlockedAccess)
			{
				SetCannotPerformReason("#AR-Campaign_Action_ShowBuildPreviewEnemyPresence");
				return false;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] user
	//! \return
	SCR_ECharacterRank GetUserRank(notnull IEntity user)
	{
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!playerController)
			return SCR_ECharacterRank.INVALID;

		return SCR_CharacterRankComponent.GetCharacterRank(playerController.GetControlledEntity());
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_ProviderComponent || !m_MainParent)
			return false;

		if (!m_CompartmentAccess)
		{
			m_CompartmentAccess = SCR_CompartmentAccessComponent.Cast(user.FindComponent(SCR_CompartmentAccessComponent));

			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (playerController)
				playerController.m_OnControlledEntityChanged.Insert(SetNewCompartmentComponent);

			return false;
		}

		if (m_CompartmentAccess.IsGettingIn())
			return false;

		Physics providerPhysics = m_MainParent.GetPhysics();

		// Don't quit if the providerPhysics doesn't exist. The provider might not have one.
		if (providerPhysics)
		{
			vector velocity = providerPhysics.GetVelocity();
			if ((velocity.LengthSq()) > PROVIDER_SPEED_TO_REMOVE_BUILDING_SQ)
				return false;
		}

		// Don't show the action if player is within any vehicle.
		ChimeraCharacter char = ChimeraCharacter.Cast(user);
		if (!char || char.IsInVehicle())
			return false;

		// No action if the provider is destroyed
		if (m_DamageManager)
		{
			if (m_DamageManager.GetState() == EDamageState.DESTROYED)
				return false;
		}

		if (m_ProviderComponent.CanBeUsedByAnyFaction())
		{
			BaseCompartmentManagerComponent comparmentComponent = BaseCompartmentManagerComponent.Cast(GetOwner().GetRootParent().FindComponent(BaseCompartmentManagerComponent));
			if (comparmentComponent)
			{
				array<BaseCompartmentSlot> outCompartments = {};
				comparmentComponent.GetCompartments(outCompartments);

				foreach (BaseCompartmentSlot compartment : outCompartments)
				{
					if (!compartment.IsOccupied())
						continue;

					if (m_ProviderComponent.IsEntityFactionSame(compartment.GetOccupant(), user))
						continue;

					DamageManagerComponent occupatDamageManager = DamageManagerComponent.Cast(compartment.GetOccupant().FindComponent(DamageManagerComponent));
					if (occupatDamageManager && occupatDamageManager.GetState() == EDamageState.DESTROYED)
						continue;

					return false;
				}

				return true;
			}
		}

		return m_ProviderComponent.IsEntityFactionSame(m_ProviderComponent.GetOwner(), user);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (!m_ResourceComponent)
			m_ResourceComponent = m_ProviderComponent.GetResourceComponent();

		if (!m_ResourceComponent.IsResourceTypeEnabled() && !m_sActionNameNoSupplies.IsEmpty())
		{
			outName = m_sActionNameNoSupplies;
			return true;
		}

		if (!m_ResourceComponent
		||	!m_ResourceConsumer && !m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES, m_ResourceConsumer))
			return false;

		if (!m_ResourceInventoryPlayerComponentRplId || !m_ResourceInventoryPlayerComponentRplId.IsValid())
			m_ResourceInventoryPlayerComponentRplId = Replication.FindId(SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent)));

		if (m_ResourceSubscriptionHandleConsumer)
			m_ResourceSubscriptionHandleConsumer.Poke();
		else
			m_ResourceSubscriptionHandleConsumer = GetGame().GetResourceSystemSubscriptionManager().RequestSubscriptionListenerHandleGraceful(m_ResourceConsumer, m_ResourceInventoryPlayerComponentRplId);

		ActionNameParams[0] = string.ToString(m_ResourceConsumer.GetAggregatedResourceValue());

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets a new compartment component. Controlled by an event when the controlled entity has changed.
	//\param[in] from Entity from which the ownership is being pased
	//\param[in] to Entity to which the ownership is passed
	void SetNewCompartmentComponent(IEntity from, IEntity to)
	{
		if (to)
			m_CompartmentAccess = SCR_CompartmentAccessComponent.Cast(to.FindComponent(SCR_CompartmentAccessComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! Set temporary blocked access.
	void SetTemporaryBlockedAccess()
	{
		IEntity masterProvider = m_ProviderComponent.GetMasterProviderEntity();
		if (!masterProvider)
			return;

		SCR_CampaignBuildingProviderComponent masterProviderComponent = SCR_CampaignBuildingProviderComponent.Cast(masterProvider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!masterProviderComponent)
			return;

		GetGame().GetWorld().QueryEntitiesBySphere(GetOwner().GetOrigin(), masterProviderComponent.GetBuildingRadius(), EvaluateEntity, null, EQueryEntitiesFlags.DYNAMIC);
		GetGame().GetCallqueue().CallLater(ResetTemporaryBlockedAccess, TEMPORARY_BLOCKED_ACCESS_RESET_TIME, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Set temporary blocked access back to default false value.
	void ResetTemporaryBlockedAccess()
	{
		m_bTemporarilyBlockedAccess = false;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if this entity can block player to enter a building mode. If such anentity is found, return false to stop evaluating next enttiy found by query.
	//\param[in] ent Entity to evaluate by this filter.
	bool EvaluateEntity(IEntity ent)
	{
		if (!ent)
			return true;

		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(ent);
		if (!char)
			return true;

		SCR_CharacterDamageManagerComponent charDamageManager = SCR_CharacterDamageManagerComponent.Cast(char.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!charDamageManager || charDamageManager.GetState() == EDamageState.DESTROYED)
			return true;

		if (!m_ProviderComponent.IsEnemyFaction(char))
			return true;

		CharacterControllerComponent charControl = char.GetCharacterController();
		if (!charControl)
			return true;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(ent);
		if (playerId == 0)
		{
			AIControlComponent ctrComp = charControl.GetAIControlComponent();
			if (!ctrComp)
				return true;

			if (ctrComp.IsAIActivated())
			{
				m_bTemporarilyBlockedAccess = true;
				return false;
			}
		}
		else
		{
			m_bTemporarilyBlockedAccess = true;
			return false;
		}

		return true;
	}
}
