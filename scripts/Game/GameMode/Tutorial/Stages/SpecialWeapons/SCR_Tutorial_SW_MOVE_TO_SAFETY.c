[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_MOVE_TO_SAFETYClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SW_MOVE_TO_SAFETY : SCR_BaseTutorialStage
{
	protected IEntity m_RadiusCenter;
	protected SCR_DetonatorGadgetComponent m_Detonator;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_RadiusCenter = GetGame().GetWorld().FindEntityByName("DESTRUCTABLE_OBSTACLE");
	
		RegisterWaypoint("WP_SW_SAFETY");
		
		IEntity detonator = GetGame().GetWorld().FindEntityByName("CourseDetonator");
		if (!detonator)
			return;
		
		m_Detonator = SCR_DetonatorGadgetComponent.Cast(detonator.FindComponent(SCR_DetonatorGadgetComponent));
		if (!m_Detonator)
			return;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (m_Detonator && m_Detonator.GetConnectedCharges().Count() == 0)
			return true;
		
		return vector.Distance(m_RadiusCenter.GetOrigin(), m_Player.GetOrigin()) >= 20;
	}
}