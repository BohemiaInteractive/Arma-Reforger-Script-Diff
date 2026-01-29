[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionOnCompartmentEnteredOrLeft : SCR_ScenarioFrameworkActionBase
{
	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "If true, we execute actions On Compartmented Entered. Otherwise On Compartment Left")]
	bool m_bEnteredOrLeft;

	[Attribute(desc: "Target entity (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "(Optional) If used, it will get executed only when specific entity enters the compartment")]
	ref SCR_ScenarioFrameworkGet m_OccupantGetter;

	[Attribute(desc: "(Optional) If used, it will get executed only when specific compartment slots are entered")]
	ref array<int> m_aSlotIDs;

	[Attribute(desc: "Actions that will be executed on compartment entered", UIWidgets.Auto)]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActions;

	IEntity m_OccupantEntity;

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicle
	//! \param[in] mgr
	//! \param[in] occupant
	//! \param[in] managerId
	//! \param[in] slotID
	void OnCompartmentEnteredOrLeft(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		if (m_OccupantEntity && occupant != m_OccupantEntity)
			return;

		if (m_aSlotIDs && !m_aSlotIDs.IsEmpty() && !m_aSlotIDs.Contains(slotID))
			return;

		foreach (SCR_ScenarioFrameworkActionBase actions : m_aActions)
		{
			actions.OnActivate(m_Entity);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		if (m_OccupantGetter)
		{
			SCR_ScenarioFrameworkParam<IEntity> occupantWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_OccupantGetter.Get());
			if (!occupantWrapper)
				return;

			m_OccupantEntity = occupantWrapper.GetValue();
			if (!m_OccupantEntity)
				return;
		}

		if (Vehicle.Cast(entity))
		{
			EventHandlerManagerComponent ehManager = EventHandlerManagerComponent.Cast(entity.FindComponent(EventHandlerManagerComponent));
			if (!ehManager)
			{
				if (object)
						Print(string.Format("ScenarioFramework Action: Event Handler Manager Component not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
					else
						Print(string.Format("ScenarioFramework Action: Event Handler Manager Component not found for Action %1.", this), LogLevel.ERROR);

				return;
			}

			if (m_bEnteredOrLeft)
				ehManager.RegisterScriptHandler("OnCompartmentEntered", this, OnCompartmentEnteredOrLeft, true, false);
			else
				ehManager.RegisterScriptHandler("OnCompartmentLeft", this, OnCompartmentEnteredOrLeft, true, false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetSubActions()
	{
		return m_aActions;
	}
}