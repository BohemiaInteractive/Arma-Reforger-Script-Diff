[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionOnDamageStateChanged : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to check damage changes on.")]
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	[Attribute(UIWidgets.Auto, desc: "What to do on damage state change")]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aAction;
	
	[Attribute(defvalue: "EDamageState.UNDAMAGED.ToString()", desc: "Activation Damage State", uiwidget: UIWidgets.ComboBox, enumType: EDamageState)]
	EDamageState m_eActivationDamageState;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		if (!ValidateInputEntity(object, m_Getter, m_Entity))
			return;
		
		SCR_DamageManagerComponent objectDmgManager = SCR_DamageManagerComponent.GetDamageManager(m_Entity);
		if (!objectDmgManager)
			return;
		
		objectDmgManager.GetOnDamageStateChanged().Insert(OnDamageStateChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] state
	void OnDamageStateChanged(EDamageState state)
	{
		if (state != m_eActivationDamageState)
			return;
		
		SCR_DamageManagerComponent objectDmgManager = SCR_DamageManagerComponent.GetDamageManager(m_Entity);
		if (objectDmgManager)
		 	objectDmgManager.GetOnDamageStateChanged().Remove(OnDamageStateChanged);
		
		foreach (SCR_ScenarioFrameworkActionBase action : m_aAction)
		{
			action.OnActivate(m_Entity);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetSubActions()
	{
		return m_aAction;
	}
}