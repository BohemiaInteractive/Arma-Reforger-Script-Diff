[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetWoundState : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to be killed (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	[Attribute("false", UIWidgets.Auto)]
	bool m_bHead;
	
	[Attribute("false", UIWidgets.Auto)]
	bool m_bTorso;
	
	[Attribute("false", UIWidgets.Auto)]
	bool m_bLeftArm;
	
	[Attribute("false", UIWidgets.Auto)]
	bool m_bRightArm;
	
	[Attribute("false", UIWidgets.Auto)]
	bool m_bLeftLeg;

	[Attribute("false", UIWidgets.Auto)]
	bool m_bRightLeg;	
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		SCR_CharacterIdentityComponent identityComp = SCR_CharacterIdentityComponent.Cast(entity.FindComponent(SCR_CharacterIdentityComponent));

		if (!identityComp)
			return;

		if (m_bHead)
			identityComp.SetWoundState("Head", true);
		
		if (m_bTorso)
			identityComp.SetWoundState("Torso", true);		
		
		if (m_bLeftArm)
			identityComp.SetWoundState("Arm_L", true);
		
		if (m_bRightArm)
			identityComp.SetWoundState("Arm_R", true);
		
		if (m_bLeftLeg)
			identityComp.SetWoundState("Leg_L", true);
		
		if (m_bRightLeg)
			identityComp.SetWoundState("Leg_R", true);
	}
}