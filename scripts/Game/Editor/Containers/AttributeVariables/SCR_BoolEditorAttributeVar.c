/*
class SCR_BoolEditorAttributeVar: SCR_BaseEditorAttributeVar
{
	private bool m_bValue;
	
	bool GetValue()
	{
		return m_bValue;
	}
	void SetValue(bool value)
	{
		m_bValue = value;
	}
	
	override bool PropCompareScripted(SSnapSerializerBase snapshot) 
	{
		return snapshot.Compare(m_bValue, 1);
	}
	override bool ExtractScripted(SSnapSerializerBase snapshot) 
	{
		snapshot.SerializeBytes(m_bValue, 1);
		return true;
	}
	override bool InjectScripted(SSnapSerializerBase snapshot) 
	{
		snapshot.SerializeBytes(m_bValue, 1);
		return true;
	}
	
	void SCR_BoolEditorAttributeVar(bool value)
	{
		m_bValue = value;
	}
};
*/