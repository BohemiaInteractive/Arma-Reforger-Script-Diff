[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetResourceTypeEnabled : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "entity to manage resource types on.")]
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	[Attribute(defvalue: "1", desc: "Check Children")]
	bool m_bCheckChildren;
	
	[Attribute(defvalue: "1", desc: "Enable Resource Type")]
	bool m_bEnableResourceType;
	
	[Attribute(defvalue: "0", desc: "Resource Type To Handle", uiwidget: UIWidgets.ComboBox, enumType: EResourceType)]
	EResourceType m_eResourceTypeToHandle;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!ValidateInputEntity(object, m_Getter, m_Entity))
			return;
		
		ProcessEntity(m_Entity);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ProcessEntity(IEntity ent)
	{
		SCR_ResourceComponent resourceComp = SCR_ResourceComponent.Cast(ent.FindComponent(SCR_ResourceComponent));
		if (resourceComp)
			resourceComp.SetResourceTypeEnabled(m_bEnableResourceType, m_eResourceTypeToHandle);
		
		IEntity child = ent.GetChildren();
		while (child)
		{
			ProcessEntity(child);
			child = child.GetSibling();
		}
	}
}