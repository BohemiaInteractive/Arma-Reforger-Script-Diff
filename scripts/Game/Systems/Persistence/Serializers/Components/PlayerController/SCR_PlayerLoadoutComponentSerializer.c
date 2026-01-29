class SCR_PlayerLoadoutComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_PlayerLoadoutComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		SCR_ArsenalManagerComponent arsenalManager;
		if (!SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
			return ESerializeResult.DEFAULT;
		
		const PlayerController controller = PlayerController.Cast(owner);
		const UUID uuid = SCR_PlayerIdentityUtils.GetPlayerIdentityId(controller.GetPlayerId());
		
		SCR_ArsenalPlayerLoadout arsenalLoadout;
		if (!arsenalManager.GetPlayerArsenalLoadout(uuid, arsenalLoadout))
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.Write(arsenalLoadout);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		auto fastTravel = SCR_PlayerLoadoutComponent.Cast(component);
		auto world = ChimeraWorld.CastFrom(owner.GetWorld());
		
		int version;
		context.Read(version);

		SCR_ArsenalPlayerLoadout arsenalLoadout;
		if (!context.Read(arsenalLoadout))
			return false;
		
		SCR_ArsenalManagerComponent arsenalManager;
		if (SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
		{
			const PlayerController controller = PlayerController.Cast(owner);
			const int playerId = controller.GetPlayerId();
			const UUID uuid = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
			arsenalManager.DoSetPlayerLoadout(uuid, arsenalLoadout);
			arsenalManager.SyncPlayerloadout(playerId, arsenalLoadout);
		}

		return true;
	}
}
