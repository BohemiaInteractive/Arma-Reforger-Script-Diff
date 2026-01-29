[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkResourceComponentAction : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity for Resource Action")];
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(defvalue: "EResourceType.INVALID.ToString()", desc: "Supply type", uiwidget: UIWidgets.ComboBox, enumType: EResourceType)]
	EResourceType m_eResourceType;
	
	[Attribute(desc: "Resource system actions to be executed")];
	ref array<ref SCR_ScenarioFrameworkResourceComponentActionBase> m_aResourceComponentActions;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ResourceComponent resourceComp = SCR_ResourceComponent.Cast(entity.FindComponent(SCR_ResourceComponent));
		if (!resourceComp)
		{
			Print(string.Format("ScenarioFramework Action: Entity does not have SCR_ResourceComponent for Action %1.", this), LogLevel.ERROR);
			return;
		}

		foreach (SCR_ScenarioFrameworkResourceComponentActionBase resourceAction : m_aResourceComponentActions)
		{
			resourceAction.Init(resourceComp, m_eResourceType);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_ScenarioFrameworkResourceComponentActionBase> GetSubActionsResourceSystem()
	{
		return m_aResourceComponentActions;
	}
}