class SCR_PersistentThreatSector
{
	int idx;
	vector estimatedPos;
	float danger;
	SCR_EAIThreatSectorFlags flags;
}

class AIControlComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return AIControlComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const AIControlComponent aiControl = AIControlComponent.Cast(component);
		if (!aiControl.IsAIActivated())
			return ESerializeResult.DEFAULT;

		const SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(aiControl.GetControlAIAgent());
		const SCR_AIUtilityComponent utility = agent.m_UtilityComponent;

		float suppresion, shotsFired, injury, endangered;
		utility.m_ThreatSystem.GetThreatValues(suppresion, shotsFired, injury, endangered);
		const bool threatSystemDefault = float.AlmostEqual(suppresion + shotsFired + injury + endangered, 0);

		array<ref SCR_PersistentThreatSector> threatSectors();
		for (int i = 0; i < utility.m_SectorThreatFilter.SECTOR_COUNT; ++i)
		{
			SCR_AIThreatSector sector = utility.m_SectorThreatFilter.GetSector(i);
			if (sector.m_iState == SCR_AIThreatSector.SECTOR_STATE_IDLE || float.AlmostEqual(sector.m_fDanger, 0))
				continue;

			SCR_PersistentThreatSector threatSector();
			threatSector.idx = i;
			threatSector.estimatedPos = sector.m_vEstimatedPos;
			threatSector.danger = sector.m_fDanger;
			threatSector.flags = sector.m_eFlags;
			threatSectors.Insert(threatSector);
		}

		if (threatSystemDefault && threatSectors.IsEmpty())
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);

		// Threat system info
		if (!threatSystemDefault)
		{
			context.StartObject("threatSystem"); // We can skip threat if it is all near 0
			context.WriteDefault(suppresion, 0.0);
			context.WriteDefault(shotsFired, 0.0);
			context.WriteDefault(injury, 0.0);
			context.WriteDefault(endangered, 0.0);
			context.EndObject();
		}

		// Threat sectors
		context.Write(threatSectors);

		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		AIControlComponent aiControl = AIControlComponent.Cast(component);
		SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(aiControl.GetControlAIAgent());
		SCR_AIUtilityComponent utility = agent.m_UtilityComponent;

		int version;
		context.Read(version);

		// Threat system info
		if (context.DoesObjectExist("threatSystem"))
		{
			float suppresion, shotsFired, injury, endangered;

			context.StartObject("threatSystem");
			context.ReadDefault(suppresion, 0.0);
			context.ReadDefault(shotsFired, 0.0);
			context.ReadDefault(injury, 0.0);
			context.ReadDefault(endangered, 0.0);
			context.EndObject();

			utility.m_ThreatSystem.SetThreatValues(suppresion, shotsFired, injury, endangered);
			// m_fThreatTotal is updated through system update with current behavior on load
		}

		const WorldTimestamp timestamp = GetGame().GetWorld().GetTimestamp();
		array<ref SCR_PersistentThreatSector> threatSectors();
		context.Read(threatSectors);
		foreach (auto threatSector : threatSectors)
		{
			SCR_AIThreatSector sector = utility.m_SectorThreatFilter.GetSector(threatSector.idx);
			if (!sector)
				continue;

			sector.m_vEstimatedPos = threatSector.estimatedPos;
			sector.m_fMaxDangerBumpSinceLastUpdate = threatSector.danger;
			sector.m_eFlags = threatSector.flags;
			sector.m_iState = SCR_AIThreatSector.SECTOR_STATE_ACTIVE;
			sector.m_fTimestampLastUpdateWithEvents = timestamp;
			sector.m_fTimestampStart = timestamp;
		}

		return true;
	}
}
