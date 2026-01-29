[BaseContainerProps(configRoot: true)]
class SCR_EditorAttributeList
{
	[Attribute(category: "Attributes")]
	private ref array<ref SCR_BaseEditorAttribute> m_aAttributes;
	
	SCR_BaseEditorAttribute GetAttribute(int index)
	{
		return m_aAttributes[index];
	}
	int GetAttributesCount()
	{
		return m_aAttributes.Count();
	}
	int FindAttribute(SCR_BaseEditorAttribute attribute)
	{
		return m_aAttributes.Find(attribute);
	}
	void InsertAllAttributes(out notnull array<SCR_BaseEditorAttribute> outAttributes)
	{
		for (int i = 0, count = m_aAttributes.Count(); i < count; i++)
		{
			outAttributes.Insert(m_aAttributes[i]);
		}
	}
};