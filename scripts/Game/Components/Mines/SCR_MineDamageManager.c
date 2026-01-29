[EntityEditorProps(category: "GameScripted/Mines", description: "Damage manager for land mines.")]
class SCR_MineDamageManagerClass : SCR_DamageManagerComponentClass
{
}

class SCR_MineDamageManager : SCR_DamageManagerComponent
{
	//------------------------------------------------------------------------------------------------
	//! Hijack melee damage on mines so they cannot take melee damage when inactive
	override bool HijackDamageHandling(notnull BaseDamageContext damageContext)
	{
		if (damageContext.damageType != EDamageType.MELEE)
			return false;
		
		// Inactive mines don't take melee damage, activated mines can be destroyed using melee and explode
		SCR_PressureTriggerComponent triggerComponent = SCR_PressureTriggerComponent.Cast(GetOwner().FindComponent(SCR_PressureTriggerComponent));
		if (triggerComponent && !triggerComponent.IsActivated())
			damageContext.damageValue = 0;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override event protected void OnDamageStateChanged(EDamageState newState, EDamageState previousDamageState, bool isJIP)
	{
		if (newState != EDamageState.DESTROYED)
			return;
		
		IEntity owner = GetOwner();
		
		bool isProxy = false; 
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (rplComponent && rplComponent.IsProxy())
			isProxy = true;
		
		if (!isProxy)
			GetGame().GetCallqueue().CallLater(ExplodeWrapper, 1);
		else
			ExplodeWrapper();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void ExplodeWrapper()
	{
		// Can be called from CallLater. Therefore, we need to check if the entity still exists.
		IEntity owner = GetOwner();
		if (!owner)
			return;

		BaseTriggerComponent triggerComponent = BaseTriggerComponent.Cast(owner.FindComponent(BaseTriggerComponent));
		if (!triggerComponent)
			return;
		
		triggerComponent.SetLive();
		triggerComponent.OnUserTriggerOverrideInstigator(owner, GetInstigator());
	}
}
