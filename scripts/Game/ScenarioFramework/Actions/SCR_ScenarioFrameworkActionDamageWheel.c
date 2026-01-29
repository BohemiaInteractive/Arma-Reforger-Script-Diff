[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionDamageWheel : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity. (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(defvalue: "", desc: "Name of Slots that are defined on the SlotManagerComponent on target vehicle")]
	ref array<string> m_aSlotNamesOnSlotManager;

	[Attribute(defvalue: "100", desc: "Health Percentage to be set for target wheels", UIWidgets.Graph, "0 100 1")]
	int m_iHealthPercentage;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		SlotManagerComponent slotManager = SlotManagerComponent.Cast(entity.FindComponent(SlotManagerComponent));
		if (!slotManager)
		{
			if (object)
					Print(string.Format("ScenarioFramework Action: Slot Manager Component not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Slot Manager Component not found for Action %1.", this), LogLevel.ERROR);

			return;
		}

		foreach (string slotName : m_aSlotNamesOnSlotManager)
		{
			EntitySlotInfo slotInfo = slotManager.GetSlotByName(slotName);
			if (!slotInfo)
			{
				if (object)
						Print(string.Format("ScenarioFramework Action: Name of the slot %1 not found on target entity for Action %2 attached on %3.", slotName, this, object.GetName()), LogLevel.ERROR);
					else
						Print(string.Format("ScenarioFramework Action: Name of the slot %1 not found on target entity for Action %1.", slotName, this), LogLevel.ERROR);

				return;
			}

			IEntity wheelEntity = slotInfo.GetAttachedEntity();
			if (!wheelEntity)
			{
				if (object)
						Print(string.Format("ScenarioFramework Action: Retrieving target wheel entity failed for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
					else
						Print(string.Format("ScenarioFramework Action: Retrieving target wheel entity failed for Action %1.", this), LogLevel.ERROR);

				return;
			}

			SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.GetDamageManager(wheelEntity);
			if (!damageManager)
			{
				if (object)
						Print(string.Format("ScenarioFramework Action: Retrieving Damage Manager of target entity failed for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
					else
						Print(string.Format("ScenarioFramework Action: Retrieving Damage Manager of target entity failed for Action %1.", this), LogLevel.ERROR);

				return;
			}

			damageManager.SetHealthScaled(m_iHealthPercentage * 0.01);
		}
	}
}