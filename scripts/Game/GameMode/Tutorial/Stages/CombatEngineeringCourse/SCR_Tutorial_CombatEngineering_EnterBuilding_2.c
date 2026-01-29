[EntityEditorProps(insertable: false)]
class SCR_Tutorial_CombatEngineering_EnterBuilding_2Class : SCR_BaseTutorialStageClass
{
}

class SCR_Tutorial_CombatEngineering_EnterBuilding_2 : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		SCR_TutorialLogic_CombatEngineering logic = SCR_TutorialLogic_CombatEngineering.Cast(m_Logic);
		if (!logic)
			return;
		
		IEntity truck = GetGame().GetWorld().FindEntityByName("BUILDING_VEHICLE");
		if (!truck)
			return;
		
		RegisterWaypoint(truck, "", "CUSTOM");
		
		SCR_VoiceoverSystem.GetInstance().RegisterActor(GetGame().GetWorld().FindEntityByName("COMBATENGINEERING_Instructor_E"));
		
		if (!logic.m_bBuildingVoiceLinePlayed)
		{
			PlayNarrativeCharacterStage("COMBATENGINEERING_Instructor_E", 1);
		}
		else
		{
			RegenerateSupplies(); //Hotfix for refund now working
			ShowHint();
		}
		
		SlotManagerComponent slotMan = SlotManagerComponent.Cast(truck.FindComponent(SlotManagerComponent));
		if (!slotMan)
			return;
		
		EntitySlotInfo slot = slotMan.GetSlotByName("EngineerBox");
		if (!slot)
			return;
		
		IEntity box = slot.GetAttachedEntity();
		if (box)
			m_TutorialComponent.BlockBuildingModeAccess(box, false);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnStageFinished()
	{
		SCR_TutorialLogic_CombatEngineering logic = SCR_TutorialLogic_CombatEngineering.Cast(m_Logic);
		if (logic)
			logic.m_bBuildingVoiceLinePlayed = true;
		
		super.OnStageFinished();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return IsBuildingModeOpen();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RegenerateSupplies()
	{
		IEntity supplyTruck = GetGame().GetWorld().FindEntityByName("BUILDING_VEHICLE");
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
		
		SCR_ResourceContainer supplyContainer = resourceComp.GetContainer(EResourceType.SUPPLIES);
		if (supplyContainer)
			supplyContainer.SetResourceValue(supplyContainer.GetMaxResourceValue());
	}
}