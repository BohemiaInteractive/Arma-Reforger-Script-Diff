/*
class SCR_IntEditorAttributeVar: SCR_BaseEditorAttributeVar
{
	private int m_iValue;
	
	int GetValue()
	{
		return m_iValue;
	}
	void SetValue(int value)
	{
		m_iValue = value;
	}
	
	override bool PropCompareScripted(SSnapSerializerBase snapshot) 
	{
		return snapshot.Compare(m_iValue, 4);
	}
	override bool ExtractScripted(SSnapSerializerBase snapshot) 
	{
		snapshot.SerializeBytes(m_iValue, 4);
		return true;
	}
	override bool InjectScripted(SSnapSerializerBase snapshot) 
	{
		snapshot.SerializeBytes(m_iValue, 4);
		return true;
	}
	
	void SCR_IntEditorAttributeVar(int value)
	{
		m_iValue = value;
	}
};
*/