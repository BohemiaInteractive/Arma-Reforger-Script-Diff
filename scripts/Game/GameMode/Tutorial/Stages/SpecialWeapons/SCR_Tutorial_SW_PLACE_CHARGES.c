[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_PLACE_CHARGESClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SW_PLACE_CHARGES: SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_TutorialLogic_SW logic = SCR_TutorialLogic_SW.Cast(m_Logic);
		if (logic)
		{
			GetGame().GetCallqueue().Remove(logic.CheckPlacedCharges);
			GetGame().GetCallqueue().CallLater(logic.CheckPlacedCharges, 1000, true);
		}
		
		SCR_PlaceableInventoryItemComponent.GetOnPlacementDoneInvoker().Insert(OnChargePlaced);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnChargePlaced(ChimeraCharacter user, SCR_PlaceableInventoryItemComponent item)
	{
		SCR_TutorialLogic_SW logic = SCR_TutorialLogic_SW.Cast(m_Logic);
		if (!logic)
			return;
		
		IEntity ent = logic.GetAttachedTo(item);
		if (!ent)
			return;
		
		if (!logic.m_aPlacedCharges)
			logic.m_aPlacedCharges = {};
		
		logic.m_aPlacedCharges.Insert(item);
		
		UnregisterWaypoint(ent);
		
		if (m_aStageWaypoints && !m_aStageWaypoints.IsEmpty())
			return;
		
		m_bFinished = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_SW_PLACE_CHARGES()
	{
		SCR_PlaceableInventoryItemComponent.GetOnPlacementDoneInvoker().Remove(OnChargePlaced);
	}
};