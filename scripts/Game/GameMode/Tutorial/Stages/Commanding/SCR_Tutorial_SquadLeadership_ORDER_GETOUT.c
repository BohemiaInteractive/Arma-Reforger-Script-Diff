[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SquadLeadership_ORDER_GETOUTClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SquadLeadership_ORDER_GETOUT : SCR_BaseTutorialStage
{
	IEntity m_Soldier;
	TurretCompartmentSlot m_TurretCompartment;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		BaseWorld world = GetGame().GetWorld();
		m_Soldier = world.FindEntityByName("Course_Rifleman");
		IEntity turret = world.FindEntityByName("COURSE_M60");
		
		SCR_BaseCompartmentManagerComponent compartmentComp = SCR_BaseCompartmentManagerComponent.Cast(turret.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compartmentComp)
			return;
		
		array <BaseCompartmentSlot> compartments = {};
		compartmentComp.GetCompartments(compartments);
		m_TurretCompartment = TurretCompartmentSlot.Cast(compartments[0]);
		if (!m_TurretCompartment)
			return;
		
		RegisterWaypoint(turret, "", "GETOUT");
		
		PlayNarrativeCharacterStage("SQUADLEADERSHIP_Instructor", 11);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return !m_TurretCompartment.GetOccupant();
	}
}