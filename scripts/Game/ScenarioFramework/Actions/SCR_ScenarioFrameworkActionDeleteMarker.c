[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionDeleteMarker : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "SlotMarker from which the Marker is to be deleted (Optional if action is attached on target SlotMarker)")]
	ref SCR_ScenarioFrameworkGetLayerBase m_Getter;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;
		
		SCR_ScenarioFrameworkSlotMarker slotMarker = SCR_ScenarioFrameworkSlotMarker.Cast(entity.FindComponent(SCR_ScenarioFrameworkSlotMarker));
		if (!slotMarker)
			return;

		slotMarker.RemoveMapMarker();
	}
}