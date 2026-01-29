class SCR_AntiPersonnelMineCollisionHandlerComponentClass : SCR_SpecialCollisionHandlerComponentClass
{
}

class SCR_AntiPersonnelMineCollisionHandlerComponent : SCR_SpecialCollisionHandlerComponent
{
	//------------------------------------------------------------------------------------------------
	override void OnContactStart(notnull SCR_ChimeraCharacter character)
	{
		IEntity owner = GetOwner();
		RplComponent rplComponent = SCR_EntityHelper.GetEntityRplComponent(owner);
		if (!rplComponent || rplComponent.IsProxy())
			return;

		SCR_PressureTriggerComponent mineTriggerComp = SCR_PressureTriggerComponent.Cast(owner.FindComponent(SCR_PressureTriggerComponent));
		if (!mineTriggerComp || !mineTriggerComp.IsActivated())
			return;

		mineTriggerComp.EOnContact(owner, character, null);
	}
}