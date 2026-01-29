class SCR_GadgetManagerComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_GadgetManagerComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.Cast(component);

		const UUID gadgetId = GetSystem().GetId(gadgetManager.GetHeldGadget());
		if (gadgetId.IsNull())
			return ESerializeResult.DEFAULT;

		context.Write(gadgetId);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		auto gadgetManager = SCR_GadgetManagerComponent.Cast(component);

		UUID gadgetId;
		if (context.Read(gadgetId) && !gadgetId.IsNull())
		{
			Tuple1<SCR_GadgetManagerComponent> ctx(gadgetManager);
			PersistenceWhenAvailableTask task(OnGadgetAvailable, ctx);
			GetSystem().WhenAvailable(gadgetId, task);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnGadgetAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto gadget = IEntity.Cast(instance);
		if (!gadget)
			return;

		auto ctx = Tuple1<SCR_GadgetManagerComponent>.Cast(context);
		if (ctx.param1)
			ctx.param1.HandleInput(gadget, 1);
	}
}
