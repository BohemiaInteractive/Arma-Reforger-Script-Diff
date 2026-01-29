class SCR_MineWeaponComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_MineWeaponComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_MineWeaponComponent mine = SCR_MineWeaponComponent.Cast(component);

		const UUID flagId = GetSystem().GetId(mine.GetFlagEntity());

		if (flagId.IsNull())
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteDefault(flagId, UUID.NULL_UUID);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		SCR_MineWeaponComponent mine = SCR_MineWeaponComponent.Cast(component);

		int version;
		context.Read(version);

		UUID flagId;
		if (context.Read(flagId))
		{
			Tuple1<SCR_MineWeaponComponent> ctx(mine);
			PersistenceWhenAvailableTask task(OnFlagAvailable, ctx);
			GetSystem().WhenAvailable(flagId, task);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnFlagAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto flag = IEntity.Cast(instance);
		if (!flag)
			return;

		auto ctx = Tuple1<SCR_MineWeaponComponent>.Cast(context);
		if (ctx.param1)
			ctx.param1.SetFlag(flag);
	}
}
