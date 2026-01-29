class SCR_BaseTriggerComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_BaseTriggerComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_BaseTriggerComponent trigger = SCR_BaseTriggerComponent.Cast(component);
		const BaseContainer source = component.GetComponentSource(owner);

		const bool active = trigger.IsActivated();
		const bool activeDefault = trigger.IsActivatedByDefault();

		if (active == activeDefault)
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteDefault(active, activeDefault);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		SCR_BaseTriggerComponent trigger = SCR_BaseTriggerComponent.Cast(component);

		int version;
		context.Read(version);

		bool active;
		if (context.Read(active))
		{
			if (active)
				trigger.ActivateTrigger();
			else
				trigger.DisarmTrigger();
		}

		return true;
	}
}
