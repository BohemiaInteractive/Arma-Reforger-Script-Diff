//simple script for control lights

class SCR_SwitchLightUserAction : ScriptedUserAction
{
	[Attribute("", UIWidgets.EditBox, "Description for action menu (light up)", "")]
	protected LocalizedString  m_sLightDescription;
	[Attribute("", UIWidgets.EditBox, "Description for action menu (extinguish)", "")]
	protected LocalizedString m_sExtinguishDescription;	
	
	protected SCR_BaseInteractiveLightComponent m_LightComp;
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_LightComp = SCR_BaseInteractiveLightComponent.Cast(pOwnerEntity.FindComponent(SCR_BaseInteractiveLightComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{		
		if (!m_LightComp)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{	
		if (m_LightComp)
			m_LightComp.ToggleLight(!m_LightComp.IsOn());
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (!m_LightComp)
			return false;

		if (m_LightComp.IsOn())
			outName = m_sExtinguishDescription;
		else
			outName = m_sLightDescription;
		
		return true;
	}
};
