[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_ARM_CHARGESClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SW_ARM_CHARGES: SCR_BaseTutorialStage
{
	ref array <SCR_ExplosiveChargeComponent> m_aCharges;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		IEntity ent = GetGame().GetWorld().FindEntityByName("DESTRUCTABLE_OBSTACLE");
		if (!ent)
			return;
		
		SCR_TutorialLogic_SW logic = SCR_TutorialLogic_SW.Cast(m_Logic);
		if (!logic)
			return;
		
		foreach (SCR_PlaceableInventoryItemComponent charge : logic.m_aPlacedCharges)
		{
			RegisterWaypoint(charge.GetOwner(), "", "CUSTOM");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		SCR_TutorialLogic_SW logic = SCR_TutorialLogic_SW.Cast(m_Logic);
		if (!logic)
			return false;
		
		if (!logic.m_aPlacedCharges)
			return false;
		
		SCR_ExplosiveChargeComponent chargeComp;
		foreach (SCR_PlaceableInventoryItemComponent charge : logic.m_aPlacedCharges)
		{
			if (!charge)
				continue;
			
			chargeComp = SCR_ExplosiveChargeComponent.Cast(charge.GetOwner().FindComponent(SCR_ExplosiveChargeComponent));
			if (!chargeComp)
				continue;
			
			if (chargeComp.GetUsedFuzeType() == SCR_EFuzeType.REMOTE)
				UnregisterWaypoint(charge.GetOwner());
			else
				RegisterWaypoint(charge.GetOwner(), "", "CUSTOM");
		}
		
		return !m_aStageWaypoints || m_aStageWaypoints.IsEmpty();
	}
};