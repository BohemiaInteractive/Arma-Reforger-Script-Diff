[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionToggleLights : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity to manipulate lights with (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(defvalue: ELightType.Head.ToString(), UIWidgets.ComboBox, desc: "Which lights to be toggled", "", ParamEnumArray.FromEnum(ELightType))]
	ELightType m_eLightType;

	[Attribute(defvalue: "1", desc: "If true, light will be turned on. Otherwise it will turn it off.")]
	bool m_bTurnedOn;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		BaseLightManagerComponent lightManager = BaseLightManagerComponent.Cast(entity.FindComponent(BaseLightManagerComponent));
		if (!lightManager)
		{
			if (object)
					Print(string.Format("ScenarioFramework Action: Light Manager Component not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Light Manager Component not found for Action %1.", this), LogLevel.ERROR);

			return;
		}

		lightManager.SetLightsState(m_eLightType, m_bTurnedOn);
	}
}