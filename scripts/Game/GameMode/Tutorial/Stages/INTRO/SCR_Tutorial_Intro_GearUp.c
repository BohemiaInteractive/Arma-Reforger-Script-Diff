[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Intro_GearUpClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Intro_GearUp : SCR_BaseTutorialStage
{
	protected bool m_bItemA;
	protected bool m_bItemB;
	protected SCR_BaseTriggerEntity m_Trigger;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_Waypoint gearWP;
		
		if (m_TutorialComponent.IsEntityInPlayerInventory("intro_binoculars"))
		{
			m_bItemA = true;
		}
		else
		{
			gearWP = RegisterWaypoint("intro_binoculars", "", "PICKUP");
		}
		
		if (m_TutorialComponent.IsEntityInPlayerInventory("intro_gloves"))
		{
			m_bItemB = true;
		}
		else
		{
			gearWP = RegisterWaypoint("intro_gloves", "", "PICKUP");
		}

		SCR_InventoryStorageManagerComponent storageManComp = SCR_InventoryStorageManagerComponent.Cast(m_Player.FindComponent(SCR_InventoryStorageManagerComponent));

		if (storageManComp)	
		{
			storageManComp.m_OnItemAddedInvoker.Remove(OnItemAdded);
			storageManComp.m_OnItemAddedInvoker.Insert(OnItemAdded);
		}
		
		m_Trigger = SCR_BaseTriggerEntity.Cast(GetGame().GetWorld().FindEntityByName("INTRO_FALLTHROUGH_TRIGGER"));
		if (!m_Trigger)
			return;
		
		m_Trigger.GetOnActivate().Insert(OnStageFinished);
		m_Trigger.EnablePeriodicQueries(true);
	}
	
	
	//------------------------------------------------------------------------------------------------
	override protected void OnStageFinished()
	{
		m_Trigger.GetOnActivate().Remove(OnStageFinished);
		m_Trigger.EnablePeriodicQueries(false);
		super.OnStageFinished();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (m_bItemA && m_bItemB)
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool OnItemAdded(IEntity item, BaseInventoryStorageComponent storageComponent)
	{
		if (item.GetName() == "intro_binoculars")	
		{	
			m_bItemA = true;
			UnregisterWaypoint("intro_binoculars");
		}
		else if (item.GetName() == "intro_gloves")	
		{	
			m_bItemB = true;
			UnregisterWaypoint("intro_gloves");
		}
		
		return true;
	}
}