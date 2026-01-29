class SCR_VoiceoverSystemData : PersistentState
{
}

class SCR_VoiceoverSystemSerializer : ScriptedStateSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_VoiceoverSystemData;
	}

	//------------------------------------------------------------------------------------------------
	override ESerializeResult Serialize(notnull Managed instance, notnull BaseSerializationSaveContext context)
	{
		if (RplSession.Mode() != RplMode.None)
			return ESerializeResult.DEFAULT;

		SCR_VoiceoverSystem voiceover = SCR_VoiceoverSystem.GetInstance();
		if (!voiceover)
			return ESerializeResult.DEFAULT;

		const SCR_VoiceoverSystemSnapshot snapshot = voiceover.CreateSnapshot();
		if (!snapshot.m_sCurrentSoundEvent && snapshot.m_aQueue.IsEmpty())
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteValueDefault("currentConfig", snapshot.m_sData, ResourceName.Empty);
		context.WriteValueDefault("currentSoundEvent", snapshot.m_sCurrentSoundEvent, string.Empty);
		context.WriteValueDefault("currentActorName", snapshot.m_sCurrentActorName, string.Empty);
		context.WriteValueDefault("currentSubtitle", snapshot.m_sCurrentSubtitle, string.Empty);
		context.WriteValueDefault("currentSequence", snapshot.m_sCurrentSequence, string.Empty);

		string currentActorId = GetSystem().GetId(snapshot.m_CurrentActor);
		if (currentActorId.IsEmpty() && snapshot.m_CurrentActor)
			currentActorId = snapshot.m_CurrentActor.GetName();

		context.WriteDefault(currentActorId, string.Empty);

		if (!snapshot.m_mActorNames.IsEmpty() || !context.CanSeekMembers())
			context.WriteValue("actorNames", snapshot.m_mActorNames);

		array<string> actorIds();
		foreach (IEntity actor : snapshot.m_aActors)
		{
			string id = GetSystem().GetId(actor);
			if (id.IsEmpty() && actor)
				id = actor.GetName();

			if (!id.IsEmpty())
				actorIds.Insert(id);
		}
		if (!actorIds.IsEmpty() || !context.CanSeekMembers())
			context.Write(actorIds);

		context.WriteValue("queue", snapshot.m_aQueue);

		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override bool Deserialize(notnull Managed instance, notnull BaseSerializationLoadContext context)
	{
		SCR_VoiceoverSystem voiceover = SCR_VoiceoverSystem.GetInstance();
		if (!voiceover)
			return false;

		int version;
		context.Read(version);

		SCR_VoiceoverSystemSnapshot snapshot();

		context.ReadValueDefault("currentConfig", snapshot.m_sData, ResourceName.Empty);
		context.ReadValueDefault("currentSoundEvent", snapshot.m_sCurrentSoundEvent, string.Empty);
		context.ReadValueDefault("currentActorName", snapshot.m_sCurrentActorName, string.Empty);
		context.ReadValueDefault("currentSubtitle", snapshot.m_sCurrentSubtitle, string.Empty);
		context.ReadValueDefault("currentSequence", snapshot.m_sCurrentSequence, string.Empty);

		string currentActorId;
		context.ReadDefault(currentActorId, string.Empty);

		context.ReadValue("actorNames", snapshot.m_mActorNames);

		array<string> actorIds;
		if (context.Read(actorIds))
			snapshot.m_aActors.Resize(actorIds.Count());

		context.ReadValue("queue", snapshot.m_aQueue);

		if (UUID.IsUUID(currentActorId))
		{
			SCR_VoiceoverSystemSerializerContext ctx();
			ctx.m_Snapshot = snapshot;
			PersistenceWhenAvailableTask task(OnActorAvailable, ctx);
			GetSystem().WhenAvailable(currentActorId, task, 30.0);  // it may take some time after init to spawn the player character
		}
		else if (!currentActorId.IsEmpty())
		{
			snapshot.m_CurrentActor = GetGame().GetWorld().FindEntityByName(currentActorId);
		}

		if (actorIds)
		{
			foreach (int idx, string actorId : actorIds)
			{
				if (UUID.IsUUID(actorId))
				{
					SCR_VoiceoverSystemSerializerContext ctx();
					ctx.m_Snapshot = snapshot;
					ctx.m_iActorIdx = idx;
					PersistenceWhenAvailableTask task(OnActorAvailable, ctx);
					GetSystem().WhenAvailable(currentActorId, task, 30.0);
				}
				else
				{
					snapshot.m_aActors[idx] = GetGame().GetWorld().FindEntityByName(currentActorId);
				}
			}
		}

		TryApplySnapshot(snapshot);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnActorAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto actor = IEntity.Cast(instance);
		if (!actor)
			return;

		auto ctx = SCR_VoiceoverSystemSerializerContext.Cast(context);
		if (ctx.m_iActorIdx == -1)
		{
			ctx.m_Snapshot.m_CurrentActor = actor;
		}
		else
		{
			ctx.m_Snapshot.m_aActors[ctx.m_iActorIdx] = actor;
		}

		TryApplySnapshot(ctx.m_Snapshot);
	}

	//------------------------------------------------------------------------------------------------
	protected static bool TryApplySnapshot(SCR_VoiceoverSystemSnapshot snapshot)
	{
		// Waiting for current actor to be linked
		if (snapshot.m_sCurrentSoundEvent && !snapshot.m_CurrentActor)
			return false;
		
		// Waiting for any sequence actor to be linked
		foreach (IEntity actor : snapshot.m_aActors)
		{
			if (!actor)
				return false;
		}

		// Wait so player is mentally ready when the load ends to listen to voiceover
		SCR_VoiceoverSystem voiceover = SCR_VoiceoverSystem.GetInstance();
		GetGame().GetCallqueue().CallLater(voiceover.ApplySnapshot, 1000, false, snapshot);
		return true;
	}
}

class SCR_VoiceoverSystemSerializerContext
{
	ref SCR_VoiceoverSystemSnapshot m_Snapshot;
	int m_iActorIdx = -1;
}
