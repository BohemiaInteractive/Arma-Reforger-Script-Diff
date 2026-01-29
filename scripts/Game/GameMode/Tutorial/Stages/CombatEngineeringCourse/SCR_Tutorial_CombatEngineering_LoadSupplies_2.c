[EntityEditorProps(insertable: false)]
class SCR_Tutorial_CombatEngineering_LoadSupplies_2Class: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_CombatEngineering_LoadSupplies_2 : SCR_BaseTutorialStage
{
	protected SCR_ResourceContainer m_SupplyContainer;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		IEntity supplyTruck = GetGame().GetWorld().FindEntityByName("BUILDING_TRUCK");
		RegisterWaypoint("BUILDING_TRUCK", "", "LOAD");
		m_bCheckWaypoint = false;
		
		SetupSupplyConsumer();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupSupplyConsumer()
	{
		IEntity supplyTruck = GetGame().GetWorld().FindEntityByName("BUILDING_TRUCK");
		
		if (!supplyTruck)
			return;
		
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(supplyTruck.FindComponent(SlotManagerComponent));
		
		if (!slotManager)
			return;
		
		EntitySlotInfo slotInfo = slotManager.GetSlotByName("EngineerBox");
		if (!slotInfo)
			return;
		
		IEntity cargo = slotInfo.GetAttachedEntity();
		if (!cargo)
			return;
		
		SCR_ResourceComponent resourceComp = SCR_ResourceComponent.FindResourceComponent(cargo);
		if (!resourceComp)
			return;
		
		m_SupplyContainer = resourceComp.GetContainer(EResourceType.SUPPLIES);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_SupplyContainer)
			return false;
		
		return m_SupplyContainer.GetResourceValue() == m_SupplyContainer.GetMaxResourceValue();
	}
};