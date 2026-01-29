[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkOnWeaponAmmoCountChangedAction : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Weapon getter.")]
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	[Attribute(desc: "Actions to be executed if conditions' evaluation is successful.")]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aActions;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		if (!ValidateInputEntity(object, m_Getter, m_Entity))
			return;
		
		EventHandlerManagerComponent eventHandler = EventHandlerManagerComponent.Cast(m_Entity.FindComponent(EventHandlerManagerComponent));
		if (!eventHandler)
			return;
		
		eventHandler.RegisterScriptHandler("OnAmmoCountChanged", this, OnAmmoCountChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAmmoCountChanged(BaseWeaponComponent changedWeaponComp, BaseMuzzleComponent changedMuzzleComp, BaseMagazineComponent changedMagComp, int newAmmoCount, bool isBarrelChambered)
	{	
		foreach (SCR_ScenarioFrameworkActionBase actions : m_aActions)
		{
			actions.OnActivate(m_Entity);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetSubActions()
	{
		return m_aActions;
	}
}