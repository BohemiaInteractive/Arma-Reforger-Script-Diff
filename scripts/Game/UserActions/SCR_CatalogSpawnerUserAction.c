/*!
Action to spawn entities using the ActionsManagerComponent
*/
class SCR_CatalogSpawnerUserAction : ScriptedUserAction
{
	[Attribute(defvalue: EResourceType.SUPPLIES.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Sets the type of Resource to be used.\nOnly a transaction matching Resource types can be successfully concluded.", enums: ParamEnumArray.FromEnum(EResourceType))]
	protected EResourceType m_eResourceType;
	
	protected SCR_ResourceSystemSubscriptionHandleBase m_ResourceSubscriptionHandleConsumer;
	protected SCR_ResourceSystemSubscriptionHandleBase m_ResourceSubscriptionHandleGenerator;
	protected SCR_ResourceComponent m_ResourceComponent;
	protected SCR_ResourceGenerator m_ResourceGenerator;
	protected SCR_ResourceConsumer m_ResourceConsumer;
	
	protected RplId m_ResourceInventoryPlayerComponentRplId;
	protected SCR_CatalogEntitySpawnerComponent m_EntitySpawner; 
	protected SCR_EEntityRequestStatus m_iRequestStatus;
	protected SCR_EntitySpawnerSlotComponent m_PreviewSlot;
	protected SCR_EntityCatalogEntry m_EntityData
	protected SCR_EntityCatalogSpawnerData m_EntitySpawnerData;
	protected bool m_bIsSelected;
	
	//------------------------------------------------------------------------------------------------
	void SetSpawnerData(SCR_EntityCatalogEntry entityData)
	{
		if (!entityData)
		{
			m_EntityData = null;
			m_EntitySpawnerData = null;
			return;
		}
		
		SCR_EntityCatalogSpawnerData spawnerData = SCR_EntityCatalogSpawnerData.Cast(entityData.GetEntityDataOfType(SCR_EntityCatalogSpawnerData));
		if (!spawnerData)
			return;
		
		m_EntityData = entityData;
		m_EntitySpawnerData = spawnerData;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	} 
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		IEntity parent = pOwnerEntity;
		SCR_CatalogEntitySpawnerComponent comp;
		while (parent)
		{
			comp = SCR_CatalogEntitySpawnerComponent.Cast(parent.FindComponent(SCR_CatalogEntitySpawnerComponent));
			if (comp)
			{
				m_EntitySpawner = comp;
				return;
			}
			
			parent = parent.GetParent();
		}
		
		if (!m_EntitySpawner)
			return;
		
		m_ResourceComponent = m_EntitySpawner.GetSpawnerResourceComponent();
		
		if (!m_ResourceComponent)
			return;
		
		m_ResourceGenerator	= m_ResourceComponent.GetGenerator(EResourceGeneratorID.DEFAULT, m_eResourceType);
		m_ResourceConsumer	= m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, m_eResourceType);
	}
	//------------------------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		m_bIsSelected = true;
		
		if (!m_EntitySpawner)
			return;
		
		if (IsAnySlotAvailable())
			m_EntitySpawner.CreatePreviewEntity(m_EntityData, m_PreviewSlot, m_iRequestStatus);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		m_bIsSelected = false;
		m_EntitySpawner.DeletePreviewEntity();
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		if (!m_EntitySpawner)
			return;
		
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		
		if (!playerController)
			return;
		
		SCR_SpawnerRequestComponent playerReqComponent = SCR_SpawnerRequestComponent.Cast(playerController.FindComponent(SCR_SpawnerRequestComponent));
		if (!playerReqComponent)
			return;
			
		SCR_EntitySpawnerSlotComponent usedPreviewSlot = m_PreviewSlot;
		playerReqComponent.RequestCatalogEntitySpawn(GetActionIndex(), m_EntitySpawner, pUserEntity, usedPreviewSlot);
		
		m_EntitySpawner.DeletePreviewEntity();
		m_EntitySpawner.ClearKnownSlots();
		m_EntitySpawner.AddKnownOccupiedSlot(usedPreviewSlot);
		
		if (IsAnySlotAvailable())
			m_EntitySpawner.CreatePreviewEntity(m_EntityData, m_PreviewSlot, m_iRequestStatus);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (!m_EntitySpawner || !m_EntitySpawnerData || !m_EntitySpawner.GetSpawnerResourceComponent())
			return false;
		
		ActionNameParams[0] = m_EntitySpawnerData.GetOverwriteName();
		if (ActionNameParams[0] == string.Empty)
			ActionNameParams[0] = m_EntityData.GetEntityName();
		
		if (m_EntitySpawner.IsSuppliesConsumptionEnabled())
		{
			ActionNameParams[1] = m_EntitySpawnerData.GetSupplyCost().ToString();
			ActionNameParams[2] = m_EntitySpawner.GetSpawnerResourceValue().ToString();
		}

		outName = "#AR-EntitySpawner_Request";
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetPreviewSlot(SCR_EntitySpawnerSlotComponent previewSlot)
	{
		if (m_PreviewSlot == previewSlot)
			return;
		
		m_PreviewSlot = previewSlot;
		if (m_PreviewSlot && m_bIsSelected)
			m_EntitySpawner.CreatePreviewEntity(m_EntityData, m_PreviewSlot, m_iRequestStatus);
		
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsAnySlotAvailable()
	{	
		if (!m_EntityData)
			return false;
		
		SCR_EntityCatalogSpawnerData data = SCR_EntityCatalogSpawnerData.Cast(m_EntityData.GetEntityDataOfType(SCR_EntityCatalogSpawnerData));
		
		SetPreviewSlot(m_EntitySpawner.GetFreeSlot(data));
		
		return m_PreviewSlot != null;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!m_EntitySpawner)
			return false;
		
		if (!m_ResourceComponent)
			m_ResourceComponent = m_EntitySpawner.GetSpawnerResourceComponent();
		
		if (!m_ResourceInventoryPlayerComponentRplId  || !m_ResourceInventoryPlayerComponentRplId.IsValid())
			m_ResourceInventoryPlayerComponentRplId = Replication.FindId(SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent)));
		
		if (!m_ResourceComponent 
		||	!m_ResourceInventoryPlayerComponentRplId.IsValid()
		||	!m_ResourceGenerator	&& !m_ResourceComponent.GetGenerator(EResourceGeneratorID.DEFAULT, m_eResourceType, m_ResourceGenerator)
		||	!m_ResourceConsumer		&& !m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, m_eResourceType, m_ResourceConsumer))
		{
			SetCannotPerformReason("#AR-Supplies_CannotPerform_Generic");
			
			return false;
		}
		
		if (m_ResourceSubscriptionHandleConsumer)
			m_ResourceSubscriptionHandleConsumer.Poke();
		else
			m_ResourceSubscriptionHandleConsumer = GetGame().GetResourceSystemSubscriptionManager().RequestSubscriptionListenerHandleGraceful(m_ResourceConsumer, m_ResourceInventoryPlayerComponentRplId);
		
		if (m_ResourceSubscriptionHandleGenerator)
			m_ResourceSubscriptionHandleGenerator.Poke();
		else
			m_ResourceSubscriptionHandleGenerator = GetGame().GetResourceSystemSubscriptionManager().RequestSubscriptionListenerHandleGraceful(m_ResourceGenerator, m_ResourceInventoryPlayerComponentRplId);
		
		
		switch (m_iRequestStatus)
		{
			case SCR_EEntityRequestStatus.AI_LIMIT_REACHED:
			{
				SetCannotPerformReason("#AR-Campaign_Action_BuildBlocked-UC");
				break;
			}
			
			case SCR_EEntityRequestStatus.GROUP_FULL:
			{
				SetCannotPerformReason("#AR-EntitySpawner_RequestDenied_GroupFull-UC");
				break;
			}
			
			case SCR_EEntityRequestStatus.REQUESTER_NOT_GROUPLEADER:
			{
				SetCannotPerformReason("#AR-EntitySpawner_RequestDenied_NotGroupLeader-UC");
				break;
			}
			
			case SCR_EEntityRequestStatus.NOT_ENOUGH_SUPPLIES:
			{
				SetCannotPerformReason("#AR-Supplies_CannotPerform_Generic");
				break;
			}
				
			case SCR_EEntityRequestStatus.RANK_LOW:
			{
				string rankName;
				FactionAffiliationComponent factionAffiliationComp = FactionAffiliationComponent.Cast(user.FindComponent(FactionAffiliationComponent));
				if (!factionAffiliationComp)
					break;
				
				SCR_Faction faction = SCR_Faction.Cast(factionAffiliationComp.GetAffiliatedFaction());
				if (faction)
					rankName = faction.GetRankName(m_EntitySpawnerData.GetMinimumRequiredRank());
					
				SetCannotPerformReason(rankName);
				break;
			}
			
			case SCR_EEntityRequestStatus.COOLDOWN:
			{
				SetCannotPerformReason("#AR-Campaign_Action_Cooldown-UC");
				break;
			}
			
			case SCR_EEntityRequestStatus.CAN_SPAWN:
			{	
				if (IsAnySlotAvailable())
					return true;
					
				SetCannotPerformReason("#AR-Campaign_Action_BuildBlocked-UC");		
				break;
			}
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_EntitySpawner || !m_EntityData || !m_EntitySpawnerData)
			return false;
		
		SCR_ChimeraCharacter chimeraCharacter = SCR_ChimeraCharacter.Cast(user);
		if (!chimeraCharacter || chimeraCharacter.GetFaction() != m_EntitySpawner.GetFaction())
			return false;
		
		m_iRequestStatus = m_EntitySpawner.GetRequestState(m_EntityData, user);
		if (m_iRequestStatus != SCR_EEntityRequestStatus.NOT_AVAILABLE)
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetActionIndex()
	{
		return GetActionID();
	}
}