// Convenience Save/Load contexts to work with binary format

/*-----------------------------------------------------------
Context to save data to given serliazation container
*/
class SCR_BinSaveContext : ContainerSerializationSaveContext
{
	protected ref BinSerializationSaveContainer m_Container;

	void SCR_BinSaveContext(bool skipEmptyObjects = true)
	{
		m_Container = new BinSerializationSaveContainer();
		SetContainer(m_Container);
	}

	void ConfigureCompression(bool enabled)
	{
		m_Container.ConfigureCompression(enabled);
	}

	void ConfigureNameValidation(bool enabled)
	{
		m_Container.ConfigureNameValidation(enabled);
	}

	void ConfigureSkippableObjects(bool enabled)
	{
		m_Container.ConfigureSkippableObjects(enabled);
	}

	void ConfigureObjectSeeking(bool enabled)
	{
		m_Container.ConfigureObjectSeeking(enabled);
	}

	bool SaveToFile(string filePath)
	{
		return m_Container.SaveToFile(filePath);
	}

	BinSerializationContainer SaveToContainer()
	{
		return m_Container.SaveToContainer();
	}
}

/*-----------------------------------------------------------
Context to load data from given serliazation container
*/
class SCR_BinLoadContext : ContainerSerializationLoadContext
{
	protected ref BinSerializationLoadContainer m_Container;

	void SCR_BinLoadContext(bool skipEmptyObjects = true)
	{
		m_Container = new BinSerializationLoadContainer();
		SetContainer(m_Container);
	}

	void ConfigureCompression(bool enabled)
	{
		m_Container.ConfigureCompression(enabled);
	}

	void ConfigureNameValidation(bool enabled)
	{
		m_Container.ConfigureNameValidation(enabled);
	}

	void ConfigureSkippableObjects(bool enabled)
	{
		m_Container.ConfigureSkippableObjects(enabled);
	}

	void ConfigureObjectSeeking(bool enabled)
	{
		m_Container.ConfigureObjectSeeking(enabled);
	}

	bool LoadFromFile(string fileName)
	{
		return m_Container.LoadFromFile(fileName);
	}

	bool LoadFromContainer(notnull BinSerializationContainer container)
	{
		return m_Container.LoadFromContainer(container);
	}
}
