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
	override ESerializeResult Serialize(notnull Managed instance, notnull SaveContext context)
	{
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

		if (persistentAuthors.IsEmpty())
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		const bool prev = context.EnableTypeDiscriminator(false);
		context.WriteValue("authors", persistentAuthors);
		context.EnableTypeDiscriminator(prev);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override bool Deserialize(notnull Managed instance, notnull LoadContext context)
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
			const PlayerManager playerManager = GetGame().GetPlayerManager();
			array<int> players();
			playerManager.GetAllPlayers(players);

			foreach (SCR_PersistentEditableEntityAuthor authorData : authors)
			{
				SCR_EditableEntityAuthor author();
				author.m_sAuthorUID = authorData.m_sIdentity;
				author.m_sAuthorPlatformID = authorData.m_sPlatformId;
				author.m_ePlatform = authorData.m_ePlatform;
				author.m_iAuthorID = -1;
				foreach (int playerId : players)
				{
					if (SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId) == authorData.m_sPlatformId)
					{
						author.m_iAuthorID = playerId;
						break;
					}
				}

				author.m_iEntityCount = -1; // Registration will bump it to 0
				core.RegisterAuthorServer(author);
			}
		}

		return true;
	}
}
