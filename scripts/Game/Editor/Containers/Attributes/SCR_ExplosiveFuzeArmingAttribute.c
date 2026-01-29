//! Explosive trigger attribute for manipulating the arming state
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_ExplosiveFuzeArmingAttribute : SCR_BaseEditorAttribute
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

		return SCR_BaseEditorAttributeVar.CreateBool(explosiveComp.GetUsedFuzeType() != SCR_EFuzeType.NONE);
	}

	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;

		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		IEntity owner = editableEntity.GetOwner();
		if (!owner)
			return;

		SCR_ExplosiveChargeComponent explosiveComp = SCR_ExplosiveChargeComponent.Cast(owner.FindComponent(SCR_ExplosiveChargeComponent));
		if (!explosiveComp)
			return;

		if (var.GetBool())
		{
			float fuzeTime;
			
			//~ Get fuze timer attribute
			SCR_BaseEditorAttributeVar fuzeTimerVar;
			if (manager.GetAttributeVariable(SCR_ExplosiveFuzeTimerAttribute, fuzeTimerVar))
				fuzeTime = fuzeTimerVar.GetFloat();
			
			//~ Fall back if no fuze timer attribute was found
			if (fuzeTime <= 0)
				fuzeTime = explosiveComp.GetFuzeTime();
			
			explosiveComp.SetFuzeTime(fuzeTime, true);
			explosiveComp.ArmWithTimedFuze(true);
		}
		else
		{
			if (explosiveComp.GetUsedFuzeType() != SCR_EFuzeType.NONE)
				explosiveComp.DisarmChargeSilent();
		}
	}
}
