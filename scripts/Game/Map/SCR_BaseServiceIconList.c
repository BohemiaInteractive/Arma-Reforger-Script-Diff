//! Config for map's base service icons
[BaseContainerProps(configRoot: true)]
class SCR_BaseServiceIconList
{
	[Attribute()]
	protected ref array<ref SCR_BaseServiceIconInfo> m_aServiceIconList;

	//------------------------------------------------------------------------------------------------
	void GetIconList(out notnull array<ref SCR_BaseServiceIconInfo> iconList)
	{
		foreach (SCR_BaseServiceIconInfo info : m_aServiceIconList)
		{
			iconList.Insert(info);
		}
	}

	//------------------------------------------------------------------------------------------------
	void GetIconListWithType(out notnull map<SCR_EServicePointType, ref SCR_BaseServiceIconInfo> iconList)
	{
		foreach (SCR_BaseServiceIconInfo info : m_aServiceIconList)
		{
			iconList.Insert(info.GetServicePointType(), info);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	array<EEditableEntityLabel> GetEntityLabels()
	{
		array<EEditableEntityLabel> entityLabels = new array<EEditableEntityLabel>();
		
		foreach(SCR_BaseServiceIconInfo info : m_aServiceIconList)
		{
			EEditableEntityLabel entityLabel = info.GetEntityLabel();
			entityLabels.Insert(entityLabel);
		}
		
		return entityLabels;
	}
}
