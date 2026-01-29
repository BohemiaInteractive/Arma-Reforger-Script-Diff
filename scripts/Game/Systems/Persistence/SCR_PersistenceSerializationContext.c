//------------------------------------------------------------------------------------------------
class SCR_PersistenceJsonSaveContext : SCR_JsonSaveContext
{
	void SCR_PersistenceJsonSaveContext(bool skipEmptyObjects = true)
	{
		ConfigureTypeDiscriminator();
		EnableTypeDiscriminator(true);
		SetMaxDecimalPlaces(4);
	}
}

//------------------------------------------------------------------------------------------------
class SCR_PersistencePrettyJsonSaveContext : SCR_PersistenceJsonSaveContext
{
	override protected void CreateContainer()
	{
		m_Container = new PrettyJsonSaveContainer();
	}
}

//------------------------------------------------------------------------------------------------
class SCR_PersistenceJsonLoadContext : SCR_JsonLoadContext
{
	void SCR_PersistenceJsonLoadContext(bool skipEmptyObjects = true)
	{
		ConfigureTypeDiscriminator();
		EnableTypeDiscriminator(true);
	}
}

//------------------------------------------------------------------------------------------------
class SCR_PersistenceBinSaveContext : SCR_BinSaveContext
{
	void SCR_PersistenceBinSaveContext(bool skipEmptyObjects = true)
	{
		ConfigureTypeDiscriminator();
		EnableTypeDiscriminator(true);
		ConfigureObjectSeeking(true);
	}
}

//------------------------------------------------------------------------------------------------
class SCR_PersistenceBinLoadContext : SCR_BinLoadContext
{
	void SCR_PersistenceBinLoadContext(bool skipEmptyObjects = true)
	{
		ConfigureTypeDiscriminator();
		EnableTypeDiscriminator(true);
		ConfigureObjectSeeking(true);
	}
}
