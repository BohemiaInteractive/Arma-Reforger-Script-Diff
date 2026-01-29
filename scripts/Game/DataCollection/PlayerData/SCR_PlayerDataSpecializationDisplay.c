//------------------------------------------------------------------------------------------------
class SCR_PlayerDataSpecializationDisplay : Managed
{	
	protected int m_iId;
	protected SCR_EDataStats m_iEnumId;
	protected string m_sTitle, m_sUnits;
	protected int m_iValue;
	
	//------------------------------------------------------------------------------------------------
	void SCR_PlayerDataSpecializationDisplay(int id, SCR_EDataStats en, string title, string units, float value = 0)
	{
		m_iId = id;
		m_iEnumId = en;
		m_sTitle = title;
		m_sUnits = units;
		m_iValue = value;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_EDataStats GetEnumId()
	{
		return m_iEnumId;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTitle(string s)
	{
		m_sTitle = s;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetTitle()
	{
		return m_sTitle;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetUnits(string u)
	{
		m_sUnits = u;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetUnits()
	{
		return m_sUnits;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetValue(float v)
	{
		m_iValue = v;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetValue()
	{
		return m_iValue;
	}
	
}