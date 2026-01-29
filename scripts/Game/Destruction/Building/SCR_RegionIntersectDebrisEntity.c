[EntityEditorProps(category: "GameScripted/Buildings", description: "Entity used to represent debris between building region intersections.", dynamicBox: true)]
class SCR_RegionIntersectDebrisEntityClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_RegionIntersectDebrisEntity : GenericEntity
{
	private int m_OtherRegionNumber = -1;
	
	//------------------------------------------------------------------------------------------------
	int GetOtherRegionNumber()
	{
		return m_OtherRegionNumber;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetOtherIndex(int regionNumber)
	{
		m_OtherRegionNumber = regionNumber;
	}
};