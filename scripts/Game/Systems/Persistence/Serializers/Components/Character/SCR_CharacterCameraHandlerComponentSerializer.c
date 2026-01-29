class SCR_CharacterCameraHandlerComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_CharacterCameraHandlerComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_CharacterCameraHandlerComponent cameraHandler = SCR_CharacterCameraHandlerComponent.Cast(component);
		const bool isSingleplayer = RplSession.Mode() == RplMode.None;
		if (isSingleplayer && cameraHandler.Is3rdPersonView())
		{
			context.WriteValue("version", 1);
			context.WriteValue("is3rdPerson", true);
			return ESerializeResult.OK;
		}

		return ESerializeResult.DEFAULT;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		auto cameraHandler = SCR_CharacterCameraHandlerComponent.Cast(component);

		int version;
		context.Read(version);

		bool is3rdPerson = false;
		context.ReadValue("is3rdPerson", is3rdPerson);
		if (is3rdPerson)
			cameraHandler.SetThirdPerson(true);

		return true;
	}
}
