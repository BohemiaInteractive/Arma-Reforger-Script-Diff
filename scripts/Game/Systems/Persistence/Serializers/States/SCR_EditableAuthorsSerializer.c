class SCR_PersistentEditableEntityAuthor
{
	UUID m_sIdentity;
	string m_sPlatformId;
	PlatformKind m_ePlatform;
}

class SCR_EditableEntityCoreData : PersistentState
{
}

class SCR_EditableEntityCoreSerializer : ScriptedStateSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_EditableEntityCoreData;
	}

	//------------------------------------------------------------------------------------------------
	override ESerializeResult Serialize(notnull Managed instance, notnull BaseSerializationSaveContext context)
	{
		if (!Replication.IsRunning())
			return ESerializeResult.DEFAULT;

		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!core)
			return ESerializeResult.DEFAULT;

		array<ref SCR_PersistentEditableEntityAuthor> persistentAuthors();
		set<SCR_EditableEntityAuthor> authors = core.GetAllAuthorsServer();
		foreach (SCR_EditableEntityAuthor author : authors)
		{
			SCR_PersistentEditableEntityAuthor persistentAuthor();
			persistentAuthor.m_sIdentity = author.m_sAuthorUID;
			persistentAuthor.m_sPlatformId = author.m_sAuthorPlatformID;
			persistentAuthor.m_ePlatform = author.m_ePlatform;
			persistentAuthors.Insert(persistentAuthor);
		}

		context.WriteValue("version", 1);
		const bool prev = context.EnableTypeDiscriminator(false);
		context.WriteValue("authors", persistentAuthors);
		context.EnableTypeDiscriminator(prev);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override bool Deserialize(notnull Managed instance, notnull BaseSerializationLoadContext context)
	{
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!core)
			return false;

		int version;
		context.Read(version);

		array<ref SCR_PersistentEditableEntityAuthor> authors;
		const bool prev = context.EnableTypeDiscriminator(false);
		context.Read(authors);
		context.EnableTypeDiscriminator(prev);
		if (authors)
		{
			foreach (SCR_PersistentEditableEntityAuthor authorData : authors)
			{
				SCR_EditableEntityAuthor author();
				author.m_sAuthorUID = authorData.m_sIdentity;
				author.m_sAuthorPlatformID = authorData.m_sPlatformId;
				author.m_ePlatform = authorData.m_ePlatform;
				author.m_iEntityCount = -1; // Registration will bump it to 0
				core.RegisterAuthorServer(author);

				Tuple1<SCR_EditableEntityAuthor> authorContext(author);
				PersistenceWhenAvailableTask task(OnAuthorAvailable, authorContext);
				GetSystem().WhenAvailable(authorData.m_sIdentity, task);
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnAuthorAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto playerController = PlayerController.Cast(instance);
		if (!playerController)
			return;

		auto ctx = Tuple1<SCR_EditableEntityAuthor>.Cast(context);
		if (ctx.param1)
			ctx.param1.m_iAuthorID = playerController.GetPlayerId();
	}
}
