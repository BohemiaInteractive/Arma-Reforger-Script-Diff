// Convenience Save/Load contexts to work with json format

/*-----------------------------------------------------------
 Context to save data to given serliazation container
*/
class SCR_JsonSaveContext : ContainerSerializationSaveContext
{
	protected ref BaseJsonSerializationSaveContainer m_Container;

	void SCR_JsonSaveContext(bool skipEmptyObjects = true)
	{
		CreateContainer();
		SetContainer(m_Container);
	}

	protected void CreateContainer()
	{
		m_Container = new JsonSaveContainer();
	}
	
	void SetMaxDecimalPlaces(int maxDecimalPlaces)
	{
		m_Container.SetMaxDecimalPlaces(maxDecimalPlaces);
	}

	int GetMaxDecimalPlaces()
	{
		return m_Container.GetMaxDecimalPlaces();
	}
	
	bool SaveToFile(string filePath)
	{
		return m_Container.SaveToFile(filePath);
	}
	
	string ExportToString()
	{
		return m_Container.ExportToString();
	}
}

/*-----------------------------------------------------------
 Context to load data from given serliazation container
*/
class SCR_JsonLoadContext : ContainerSerializationLoadContext
{
	protected ref JsonLoadContainer m_Container;
	
	void SCR_JsonLoadContext(bool skipEmptyObjects = true)
	{
		CreateContainer();
		SetContainer(m_Container);
	}

	protected void CreateContainer()
	{
		m_Container = new JsonLoadContainer();
	}
	
	bool ImportFromString(string jsonData)
	{
		return m_Container.ImportFromString(jsonData);
	}
	
	bool LoadFromFile(string fileName)
	{
		return m_Container.LoadFromFile(fileName);
	}
}
