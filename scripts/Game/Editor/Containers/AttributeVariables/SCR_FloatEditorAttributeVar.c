/*
class SCR_FloatEditorAttributeVar: SCR_BaseEditorAttributeVar
{
	private float m_fValue;
	
	float GetValue()
	{
		return m_fValue;
	}
	void SetValue(float value)
	{
		m_fValue = value;
	}
	
	override bool PropCompareScripted(SSnapSerializerBase snapshot) 
	{
		return snapshot.Compare(m_fValue, 4);
	}
	override bool ExtractScripted(SSnapSerializerBase snapshot) 
	{
		snapshot.SerializeBytes(m_fValue, 4);
		return true;
	}
	override bool InjectScripted(SSnapSerializerBase snapshot) 
	{
		snapshot.SerializeBytes(m_fValue, 4);
		return true;
	}
	
	void SCR_FloatEditorAttributeVar(float value)
	{
		m_fValue = value;
	}
};
*/