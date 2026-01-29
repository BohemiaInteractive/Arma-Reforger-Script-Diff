class MusicManagerSerializer : ScriptedEntitySerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return MusicManager;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity entity, notnull BaseSerializationSaveContext context)
	{
		const MusicManager musicManager = MusicManager.Cast(entity);

		array<int> categories();
		SCR_Enum.GetEnumValues(MusicCategory, categories);
		
		array<string> mutedCategories();
		foreach (auto category : categories)
		{
			if (musicManager.ServerIsCategoryMuted(category))
				mutedCategories.Insert(SCR_Enum.GetEnumName(MusicCategory, category));
		}

		if (mutedCategories.IsEmpty())
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.Write(mutedCategories);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity entity, notnull BaseSerializationLoadContext context)
	{
		auto musicManager = MusicManager.Cast(entity);

		int version;
		context.Read(version);

		array<string> mutedCategories;
		context.Read(mutedCategories);
		foreach (auto mutedCategory : mutedCategories)
		{
			const int category = typename.StringToEnum(MusicCategory, mutedCategory);
			if (category != -1)
				musicManager.RequestServerMuteCategory(category, true);
		}

		return true;
	}
}
