//! Explosive trigger attribute for manipulating the time that remains until the detonation
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_ExplosiveFuzeTimerAttribute : SCR_BaseValueListEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return null;

		IEntity owner = editableEntity.GetOwner();
		if (!owner)
			return null;

		SCR_ExplosiveChargeComponent explosiveComp = SCR_ExplosiveChargeComponent.Cast(owner.FindComponent(SCR_ExplosiveChargeComponent));
		if (!explosiveComp)
			return null;

		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return null;

		TimeAndWeatherManagerEntity timeAndWeatherManager = world.GetTimeAndWeatherManager();
		if (!timeAndWeatherManager)
			return null;
		
		int fuzeTime; 
		if (explosiveComp.GetUsedFuzeType() == SCR_EFuzeType.TIMED)
			fuzeTime = explosiveComp.GetTimeOfDetonation() - timeAndWeatherManager.GetEngineTime();
		
		//~ No current timer so use default
		if (fuzeTime <= 0)
			fuzeTime = explosiveComp.GetFuzeTime();
		
		//~ No fuze time so do not show attribute
		if (fuzeTime <= 0)
			return null;

		return SCR_BaseEditorAttributeVar.CreateFloat(fuzeTime);
	}

	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;

		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return;

		IEntity owner = editableEntity.GetOwner();
		if (!owner)
			return;

		SCR_ExplosiveChargeComponent explosiveComp = SCR_ExplosiveChargeComponent.Cast(owner.FindComponent(SCR_ExplosiveChargeComponent));
		if (!explosiveComp)
			return;

		if (explosiveComp.GetUsedFuzeType() != SCR_EFuzeType.NONE)
			explosiveComp.DisarmChargeSilent();

		explosiveComp.SetFuzeTime(var.GetFloat(), true);
		explosiveComp.ArmWithTimedFuze(true);
	}
}