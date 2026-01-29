[BaseContainerProps()]
class SCR_BuildingRegion
{
	const int MAX_REGION_CONNECT = 128;
	
	private int m_Region;
	
	float m_MaxHealth = 500;
	float m_StructuralSupportPct = 0.55;
	ResourceName m_DestructFX_PTC = "{B6CBD40830C3F0A4}graphics/particle/legacy/tkom/weapon/destruct_wall.ptc";
	ResourceName m_DestructFX_SND = "";
	ResourceName m_DestructFX_PFB = "";
	
	private SCR_BuildingRegion m_RegionConnect_Out[MAX_REGION_CONNECT];
	private int m_RegionConnect_OutNum = 0;
	
	private SCR_BuildingRegion m_RegionConnect_In[MAX_REGION_CONNECT];
	private int m_RegionConnect_InNum = 0;
	
	private vector m_RegionPos = vector.Zero;
	private vector m_RegionSize = vector.Zero;
	
	//------------------------------------------------------------------------------------------------
	void SetRegionIndex(int region)
	{
		m_Region = region;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRegionIndex()
	{
		return m_Region;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRegionSize(vector size)
	{
		m_RegionSize = size;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetRegionSize()
	{
		return m_RegionSize;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRegionPos(vector pos)
	{
		m_RegionPos = pos;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetRegionPos()
	{
		return m_RegionPos;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMaxHealth(float health)
	{
		m_MaxHealth = health;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetMaxHealth()
	{
		return m_MaxHealth;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDestructFX(ResourceName ptc, ResourceName snd, ResourceName pfb)
	{
		m_DestructFX_PTC = ptc;
		m_DestructFX_SND = snd;
		m_DestructFX_PFB = pfb;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetDestructFX(out ResourceName ptc, out ResourceName snd, out ResourceName pfb)
	{
		if (m_DestructFX_PTC == string.Empty && m_DestructFX_SND == string.Empty && m_DestructFX_PFB == string.Empty)
			return false;
		
		ptc = m_DestructFX_PTC;
		snd = m_DestructFX_SND;
		pfb = m_DestructFX_PFB;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetStructuralSupportPercentage(float pct)
	{
		m_StructuralSupportPct = pct;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetStructuralSupportPercentage()
	{
		return m_StructuralSupportPct;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddConnectionIn(SCR_BuildingRegion region)
	{
		m_RegionConnect_In[m_RegionConnect_InNum++] = region;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddConnectionOut(SCR_BuildingRegion region)
	{
		m_RegionConnect_Out[m_RegionConnect_OutNum++] = region;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetConnectionNumIn()
	{
		return m_RegionConnect_InNum;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetConnectionNumOut()
	{
		return m_RegionConnect_OutNum;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_BuildingRegion GetConnectedRegionIn(int index)
	{
		if (index >= m_RegionConnect_InNum)
			return null;
		
		return m_RegionConnect_In[index];
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_BuildingRegion GetConnectedRegionOut(int index)
	{
		if (index >= m_RegionConnect_OutNum)
			return null;
		
		return m_RegionConnect_Out[index];
	}
};
