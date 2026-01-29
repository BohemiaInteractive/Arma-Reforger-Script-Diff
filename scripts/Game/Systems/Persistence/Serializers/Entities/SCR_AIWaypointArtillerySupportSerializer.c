class SCR_AIWaypointArtillerySupportSerializer : ScriptedEntitySerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_AIWaypointArtillerySupport;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity entity, notnull BaseSerializationSaveContext context)
	{
		const SCR_AIWaypointArtillerySupport artillerySupport = SCR_AIWaypointArtillerySupport.Cast(entity);
		BaseContainer source;
		if (artillerySupport.GetPrefabData())
			source = artillerySupport.GetPrefabData().GetPrefab();

		const bool active = artillerySupport.IsActive();
		bool activeDefault = true;
		if (source)
			source.Get("m_bActive", activeDefault);

		const int targetShotCount = artillerySupport.GetTargetShotCount();
		int targetShotCountDefault = true;
		if (source)
			source.Get("m_iTargetShotCount", targetShotCountDefault);

		const SCR_EAIArtilleryAmmoType ammoType = artillerySupport.GetAmmoType();
		SCR_EAIArtilleryAmmoType ammoTypeDefault = SCR_EAIArtilleryAmmoType.HIGH_EXPLOSIVE;
		if (source)
			source.Get("m_eAmmoType", ammoTypeDefault);

		context.StartObject("base");
		const ESerializeResult baseResult = super.Serialize(entity, context);
		context.EndObject();
		if (baseResult == ESerializeResult.ERROR)
			return baseResult;

		if (baseResult == ESerializeResult.DEFAULT &&
			active == activeDefault &&
			targetShotCount == targetShotCountDefault &&
			ammoType == ammoTypeDefault)
		{
			return baseResult;
		}

		context.WriteValue("version", 1);
		context.WriteDefault(active, activeDefault);
		context.WriteDefault(targetShotCount, targetShotCountDefault);
		context.WriteDefault(ammoType, ammoTypeDefault);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity entity, notnull BaseSerializationLoadContext context)
	{
		auto artillerySupport = SCR_AIWaypointArtillerySupport.Cast(entity);

		if (context.DoesObjectExist("base"))
		{
			if (!context.StartObject("base") || 
				!super.Deserialize(entity, context) || 
				!context.EndObject())
			{
				return false;
			}
		}

		int version;
		context.Read(version);

		bool active;
		if (context.Read(active))
			artillerySupport.SetActive(active);
		
		int targetShotCount;
		if (context.Read(targetShotCount))
			artillerySupport.SetTargetShotCount(targetShotCount);
		
		SCR_EAIArtilleryAmmoType ammoType;
		if (context.Read(ammoType))
			artillerySupport.SetAmmoType(ammoType);

		return true;
	}
}
