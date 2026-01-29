class SCR_ArsenalComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_ArsenalComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_ArsenalComponent arsenal = SCR_ArsenalComponent.Cast(component);
		const BaseContainer source = arsenal.GetComponentSource(owner);

		const bool enabled = arsenal.IsArsenalEnabled();
		bool enabledDefault = true;
		if (source)
			source.Get("m_bArsenalEnabled", enabledDefault);

		const SCR_EArsenalSaveType saveType = arsenal.GetArsenalSaveType();
		SCR_EArsenalSaveType saveTypeDefault = 0;
		if (source)
			source.Get("m_eArsenalSaveType", saveTypeDefault);

		const SCR_EArsenalItemType supportedItemTypes = arsenal.GetSupportedArsenalItemTypes();
		SCR_EArsenalItemType supportedItemTypesDefault = 0;
		if (source)
			source.Get("m_eSupportedArsenalItemTypes", supportedItemTypesDefault);

		const SCR_EArsenalItemMode supportedItemModes = arsenal.GetSupportedArsenalItemModes();
		SCR_EArsenalItemMode supportedItemModesDefault = 0;
		if (source)
			source.Get("m_eSupportedArsenalItemModes", supportedItemModesDefault);

		if (enabled == enabledDefault &&
			saveType == saveTypeDefault &&
			supportedItemTypes == supportedItemTypesDefault &&
			supportedItemModes == supportedItemModesDefault)
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteDefault(enabled, enabledDefault);
		context.WriteDefault(saveType, saveTypeDefault);
		context.WriteDefault(supportedItemTypes, supportedItemTypesDefault);
		context.WriteDefault(supportedItemModes, supportedItemModesDefault);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		auto arsenal = SCR_ArsenalComponent.Cast(component);

		int version;
		context.Read(version);

		bool enabled;
		if (context.Read(enabled))
			arsenal.SetArsenalEnabled(enabled);

		SCR_EArsenalSaveType saveType;
		if (context.Read(saveType))
			arsenal.SetArsenalSaveType(saveType);

		SCR_EArsenalItemType supportedItemTypes;
		if (context.Read(supportedItemTypes))
			arsenal.SetSupportedArsenalItemTypes(supportedItemTypes);

		SCR_EArsenalItemMode supportedItemModes;
		if (context.Read(supportedItemModes))
			arsenal.SetSupportedArsenalItemModes(supportedItemModes);

		return true;
	}
}
