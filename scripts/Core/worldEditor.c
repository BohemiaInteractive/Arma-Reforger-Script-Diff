
/*class EditorWindowWidget: RenderTargetWidget
{
}*/

class WorldEditorIngame
{
	proto native bool LoadWorld(string worldFilePath);
	proto native bool SaveWorld();

	proto native void SetMoveTool();
	proto native void SetRotateTool();
	proto native void SetScaleTool();

	proto native bool Undo();
	proto native bool Redo();

	proto native void Update(float tDelta);
	proto native void Init(notnull RenderTargetWidget mainWindow);
	proto native void Cleanup();

	proto native WorldEditorAPI GetAPI();
}

class ContainerIdPathEntry : Managed
{
	string PropertyName;
	int Index;

	void ContainerIdPathEntry(string propertyName, int index = -1)
	{
		PropertyName = propertyName;
		Index = index;
	}
}

/*!
Wrapper around GetEditorEntity and GetEditorEntityCount
Skips all entities that are not top-level.
Editing the underlying entity list invalidates the iterator.
*/
class EditorEntityIterator
{
	void EditorEntityIterator(notnull WorldEditorAPI api)
	{
		m_pApi = api;
		m_iCurrentIdx = 0;
		m_iCount = m_pApi.GetEditorEntityCount();
	}

	//! Returns all top level entities in order, null when all entities were iterated.
	IEntitySource GetNext()
	{
		while (m_iCurrentIdx < m_iCount && m_pApi.GetEditorEntity(m_iCurrentIdx).GetParent())
			m_iCurrentIdx++;

		if (m_iCurrentIdx < m_iCount)
			return m_pApi.GetEditorEntity(m_iCurrentIdx++);

		return null;
	}

	int GetCurrentIdx()
	{
		return m_iCurrentIdx;
	}

	private WorldEditorAPI m_pApi;
	private int m_iCurrentIdx;
	private int m_iCount;
}
