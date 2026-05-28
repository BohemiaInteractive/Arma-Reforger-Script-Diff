//------------------------------------------------------------------------------------------------
class SCR_PersistenceJsonSaveContext : JsonSaveContext
{
	void SCR_PersistenceJsonSaveContext(bool skipEmptyObjects = true)
	{
		ConfigureTypeDiscriminator();
		EnableTypeDiscriminator(true);
		SetMaxDecimalPlaces(4);
	}
}

//------------------------------------------------------------------------------------------------
class SCR_PersistencePrettyJsonSaveContext : PrettyJsonSaveContext
{
	void SCR_PersistencePrettyJsonSaveContext(bool skipEmptyObjects = true)
	{
		ConfigureTypeDiscriminator();
		EnableTypeDiscriminator(true);
		SetMaxDecimalPlaces(4);
	}
}

//------------------------------------------------------------------------------------------------
class SCR_PersistenceJsonLoadContext : JsonLoadContext
{
	void SCR_PersistenceJsonLoadContext(bool skipEmptyObjects = true)
	{
		ConfigureTypeDiscriminator();
		EnableTypeDiscriminator(true);
	}
}

//------------------------------------------------------------------------------------------------
class SCR_PersistenceBinarySaveContext : BinarySaveContext
{
	void SCR_PersistenceBinarySaveContext(bool skipEmptyObjects = true)
	{
		ConfigureTypeDiscriminator();
		EnableTypeDiscriminator(true);
		ConfigureObjectSeeking(true);
		ConfigureSkippableObjects(true);
	}
}

//------------------------------------------------------------------------------------------------
class SCR_PersistenceBinaryLoadContext : BinaryLoadContext
{
	void SCR_PersistenceBinaryLoadContext(bool skipEmptyObjects = true)
	{
		ConfigureTypeDiscriminator();
		EnableTypeDiscriminator(true);
		ConfigureObjectSeeking(true);
		ConfigureSkippableObjects(true);
	}
}
