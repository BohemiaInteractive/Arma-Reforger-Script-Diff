class SCR_BaseHUDComponentClass: BaseHUDComponentClass
{
};

class SCR_BaseHUDComponent : BaseHUDComponent
{
	protected ref array<SCR_InfoDisplay> m_aHUDElements = new array<SCR_InfoDisplay>;
	void RegisterHUDElement(SCR_InfoDisplay element)
	{
		int i = m_aHUDElements.Find(element);
		if (i == -1)
			m_aHUDElements.Insert(element);
	}
	
	void UnregisterHUDElement(SCR_InfoDisplay element)
	{
		int i = m_aHUDElements.Find(element);
		if (i > -1)
			m_aHUDElements.Remove(i);
	}
	
	array<SCR_InfoDisplay> GetHUDElements()
	{
		return m_aHUDElements;
	}
};