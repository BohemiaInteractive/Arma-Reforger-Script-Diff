[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_PICKUP_CHARGESClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SW_PICKUP_CHARGES : SCR_BaseTutorialStage
{
	bool m_bHasDetonator; 
	int m_iChargesCount;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		if (!m_TutorialComponent)
			return;
		
		m_TutorialComponent.EnableArsenal("SW_ARSENAL_EXPLOSIVES", true);
		
		m_TutorialComponent.SpawnAsset("DETONATOR", "{CE0AF733722B3978}Prefabs/Items/Equipment/Detonators/BlastingMachine_M34/BlastingMachine_M34.et");
		m_TutorialComponent.SpawnAsset("C4_1", "{33CBDE73AB48172A}Prefabs/Weapons/Explosives/DemoBlock_M112/DemoBlock_M112.et");
		m_TutorialComponent.SpawnAsset("C4_2", "{33CBDE73AB48172A}Prefabs/Weapons/Explosives/DemoBlock_M112/DemoBlock_M112.et");
		m_TutorialComponent.SpawnAsset("C4_3", "{33CBDE73AB48172A}Prefabs/Weapons/Explosives/DemoBlock_M112/DemoBlock_M112.et");
		
		RegisterWaypoint("SpawnPos_DETONATOR", "", "PICKUP");
		RegisterWaypoint("SpawnPos_C4_2", "", "PICKUP");
		
		SCR_InventoryStorageManagerComponent storageManComp = SCR_InventoryStorageManagerComponent.Cast(m_Player.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!storageManComp)
			return;
		
		storageManComp.m_OnItemAddedInvoker.Insert(OnItemAdded);
		storageManComp.m_OnItemRemovedInvoker.Insert(OnItemRemoved);
		
		IEntity ent = GetGame().GetWorld().FindEntityByName("DESTRUCTABLE_OBSTACLE");
		if (!ent)
			return;
		
		SCR_DestructionMultiPhaseComponent destruction;
		ent = ent.GetChildren();
		while (ent)
		{
			destruction = SCR_DestructionMultiPhaseComponent.Cast(ent.FindComponent(SCR_DestructionMultiPhaseComponent));
			if (destruction)
				destruction.EnableDamageHandling(true);
			
			ent = ent.GetSibling();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool OnItemAdded(IEntity item, BaseInventoryStorageComponent storageComponent)
	{
		if (item.FindComponent(SCR_DetonatorGadgetComponent))
		{
			m_bHasDetonator = true;
			item.SetName("CourseDetonator");
			UnregisterWaypoint("SpawnPos_DETONATOR");
		}
		
		if (item.FindComponent(SCR_ExplosiveChargeComponent))
			m_iChargesCount++;
		
		if (m_iChargesCount >= 3)
			UnregisterWaypoint("SpawnPos_C4_2");
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool OnItemRemoved(IEntity item, BaseInventoryStorageComponent storageComponent)
	{
		if (item.FindComponent(SCR_DetonatorGadgetComponent))
			m_bHasDetonator = false;
		
		if (item.FindComponent(SCR_ExplosiveChargeComponent))
			m_iChargesCount--;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return (m_iChargesCount >= 3) && m_bHasDetonator;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_SW_PICKUP_CHARGES()
	{
		SCR_InventoryStorageManagerComponent storageManComp = SCR_InventoryStorageManagerComponent.Cast(m_Player.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!storageManComp)
			return;
		
		storageManComp.m_OnItemAddedInvoker.Remove(OnItemAdded);
		storageManComp.m_OnItemRemovedInvoker.Remove(OnItemRemoved);
	}
}