//simple script for lighting fireplces

class SCR_LightFireplaceUserAction : ScriptedUserAction
{
	[Attribute("#AR-UserAction_LightFire", UIWidgets.EditBox, "Description for action menu (light up)", "")]
	protected LocalizedString  m_sLightDescription;
	[Attribute("#AR-UserAction_PutOutFire", UIWidgets.EditBox, "Description for action menu (extinguish)", "")]
	protected LocalizedString m_sExtinguishDescription;	
	
	protected SCR_FireplaceComponent m_FireplaceComponent;
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_FireplaceComponent = SCR_FireplaceComponent.Cast(pOwnerEntity.FindComponent(SCR_FireplaceComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_FireplaceComponent)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{	
		m_FireplaceComponent.ToggleLight(!m_FireplaceComponent.IsOn())
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (!m_FireplaceComponent)
			return false;
		
		if (!m_FireplaceComponent.IsOn())
			outName = m_sLightDescription;
		else
			outName = m_sExtinguishDescription;
		
		return true;
	}
};
