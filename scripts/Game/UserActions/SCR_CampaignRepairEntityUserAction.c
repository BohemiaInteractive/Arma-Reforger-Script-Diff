#define ENABLE_BASE_DESTRUCTION
//------------------------------------------------------------------------------------------------
class SCR_CampaignRepairEntityUserAction : ScriptedUserAction
{
#ifdef ENABLE_BASE_DESTRUCTION
	SCR_CampaignServiceEntityComponent m_ServiceEntityComp;
	SCR_DestructionMultiPhaseComponent m_DestructionMultiphaseComp;
	
	protected SCR_CampaignMilitaryBaseComponent m_CampaignBase;
	protected SCR_SiteSlotEntity m_Slot;
	protected EEditableEntityLabel m_CompType;
	protected const int SLOT_SEARCH_DISTANCE = 5;
		
	//------------------------------------------------------------------------------------------------
	void CompositionInit()
	{/*
		IEntity owner = GetOwner();
		m_ServiceEntityComp = SCR_CampaignServiceEntityComponent.Cast(owner.FindComponent(SCR_CampaignServiceEntityComponent));
		m_DestructionMultiphaseComp = SCR_DestructionMultiPhaseComponent.Cast(owner.FindComponent(SCR_DestructionMultiPhaseComponent));
				
		GetGame().GetWorld().QueryEntitiesBySphere(m_Owner.GetOrigin(), SLOT_SEARCH_DISTANCE, GetNearestSlot, null, EQueryEntitiesFlags.ALL);
		if (!m_Slot)	
		{
			IEntity parent = owner.GetParent();
			if (!parent)
				return;
			
			SCR_CampaignServiceCompositionComponent serviceCompositionComp = SCR_CampaignServiceCompositionComponent.Cast(parent.FindComponent(SCR_CampaignServiceCompositionComponent));
			if (serviceCompositionComp)
			{
				m_CompType = serviceCompositionComp.GetCompositionType();
				SCR_ServicePointComponent service = serviceCompositionComp.GetService();
				if (service)
				{
					array<SCR_MilitaryBaseComponent> bases = {};
					service.GetBases(bases);
					
					if (!bases.IsEmpty())
						m_CampaignBase = SCR_CampaignMilitaryBaseComponent.Cast(bases[0]);
				}
			}
		}
		
		SCR_GameModeCampaign gameModeMP = SCR_GameModeCampaign.GetInstance();
		if (!gameModeMP)	
			return;
		
		if (!m_CampaignBase)
			m_CampaignBase = gameModeMP.GetSlotPresetBase(m_Slot);*/
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		if (!m_CampaignBase)
			return;
		
		if (!m_ServiceEntityComp)
			return;
		
		SCR_MPDestructionManager destructionManager = SCR_MPDestructionManager.GetInstance();
		if (!destructionManager)
			return;
		
		IEntity compositionParent = SCR_EntityHelper.GetMainParent(pOwnerEntity);
		if (!compositionParent)
			return;

		RplComponent rplComp = RplComponent.Cast(compositionParent.FindComponent(RplComponent));
		if (!rplComp)	
			return;

		RplId destructibleID = rplComp.Id();
		if (!destructibleID.IsValid())
			return;
		
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
		if (!campaignNetworkComponent)
			return;
		
		//m_Slot can be a null. If so, composition type is used.
		campaignNetworkComponent.RepairComposition(destructionManager.FindDynamicallySpawnedDestructibleIndex(destructibleID, m_DestructionMultiphaseComp), m_ServiceEntityComp.GetRepairCost(), destructibleID, m_Slot, m_CampaignBase);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{	
		if (m_ServiceEntityComp.GetRepairCost() > m_CampaignBase.GetSupplies())
		{
			SetCannotPerformReason("#AR-Campaign_Repair_Composition-UC");
			return false;
		}	
		return true;	
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_DestructionMultiphaseComp)
			return false;
		
		if (!m_ServiceEntityComp)
		{
			CompositionInit();
			return false;
		}
		
		if (!m_CampaignBase)
			return false;
			
		// Composition is in initial state don't need to be repair...
		if (m_DestructionMultiphaseComp && m_DestructionMultiphaseComp.GetDamagePhase() == 0)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// Get the nearest slot to this composition
	protected bool GetNearestSlot(IEntity ent)
	{		
		SCR_SiteSlotEntity slotEnt = SCR_SiteSlotEntity.Cast(ent);
		if (!slotEnt)
		    return true;
						
		m_Slot = slotEnt;
		return false;
	}
			
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{		
		if (!m_CampaignBase)
			return false;
		// get composition value
		int availableSupplies = m_CampaignBase.GetSupplies();
		
		ActionNameParams[0] = m_ServiceEntityComp.GetRepairCost().ToString();
		ActionNameParams[1] = availableSupplies.ToString();
					
		outName = ("#AR-Campaign_Action_ServiceRepair-UC");
		return true;
		
	}
#endif
};
