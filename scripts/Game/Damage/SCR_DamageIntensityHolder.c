//------------------------------------------------------------------------------------------------
enum SCR_EDamageIntensityType
{
	NO_DAMAGE = 0,
	LOW,
	MEDIUM,
	HIGH,
	CRITICAL,
}

//------------------------------------------------------------------------------------------------
//! Holds the localization strings as well as the logic to get the damage severity strings and enums
[BaseContainerProps(configRoot: true)]
class SCR_DamageIntensityHolder
{
	[Attribute()]
	protected ref array<ref SCR_DamageIntensityEntry> m_aDamageIntensityEntries;
	
	protected ref SCR_SortedArray<SCR_DamageIntensityEntry> m_SortedDamageIntensityEntries;
	
	//------------------------------------------------------------------------------------------------
	SCR_DamageIntensityEntry GetValidIntensityEntry(float value)
	{
		if (value < 0)
		{
			Print("'SCR_DamageIntensityEntry' Damage scaled cannot be less than 0!", LogLevel.WARNING);
			value = 0;
		}

		SCR_DamageIntensityEntry entry;
		for (int i = 0, count = m_SortedDamageIntensityEntries.Count(); i < count; i++)
		{
			//~ Check if entry is valid damage state
			entry = m_SortedDamageIntensityEntries.Get(i);
			if (!entry || !entry.IsValidDamageState(value))
				continue;
			
			//~ Valid entry
			return entry;
		}
	
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_DamageIntensityEntry GetValidIntensityEntry(notnull array<HitZone> hitZones)
	{
		if (hitZones.IsEmpty())
			return null;
		
		float healthScaled = 0;
		SCR_FlammableHitZone flammableHitZone;
		SCR_DamageManagerComponent hitZoneContainer;
		foreach (HitZone hitZone : hitZones)
		{
			if (!hitZone)
				continue;

			healthScaled += hitZone.GetHealthScaled();
			flammableHitZone = SCR_FlammableHitZone.Cast(hitZone);
			if (!flammableHitZone)
				continue;

			hitZoneContainer = SCR_DamageManagerComponent.Cast(flammableHitZone.GetHitZoneContainer());
			if (!hitZoneContainer || !hitZoneContainer.IsOnFire(flammableHitZone))
				continue;

			healthScaled = 0;
			break;
		}
		
		return GetValidIntensityEntry(healthScaled / hitZones.Count());
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_DamageIntensityEntry GetValidIntensityEntry(notnull HitZone hitZone)
	{
		return GetValidIntensityEntry(hitZone.GetHealthScaled());
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_DamageIntensityEntry GetValidIntensityEntry(notnull SCR_DamageManagerComponent damageManager)
	{
		HitZone defaultHitZone = damageManager.GetDefaultHitZone();
		if (defaultHitZone)
			return GetValidIntensityEntry(defaultHitZone.GetHealthScaled());
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_UIName GetValidIntensityUIInfo(float value)
	{
		SCR_DamageIntensityEntry entry = GetValidIntensityEntry(value);
		
		if (entry)
			return entry.GetUiInfo();
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_UIName GetValidIntensityUIInfo(notnull array<HitZone> hitZones)
	{
		SCR_DamageIntensityEntry entry = GetValidIntensityEntry(hitZones);
		
		if (entry)
			return entry.GetUiInfo();
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_UIName GetValidIntensityUIInfo(notnull HitZone hitZone)
	{
		SCR_DamageIntensityEntry entry = GetValidIntensityEntry(hitZone);
		
		if (entry)
			return entry.GetUiInfo();
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_UIName GetValidIntensityUIInfo(notnull SCR_DamageManagerComponent damageManager)
	{
		SCR_DamageIntensityEntry entry = GetValidIntensityEntry(damageManager);
		
		if (entry)
			return entry.GetUiInfo();
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_EDamageIntensityType GetValidIntensityType(float value)
	{
		SCR_DamageIntensityEntry entry = GetValidIntensityEntry(value);
		
		if (entry)
			return entry.GetDamageIntensityType();
		
		return SCR_EDamageIntensityType.NO_DAMAGE;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_EDamageIntensityType GetValidIntensityType(notnull array<HitZone> hitZones)
	{
		SCR_DamageIntensityEntry entry = GetValidIntensityEntry(hitZones);
		
		if (entry)
			return entry.GetDamageIntensityType();
		
		return SCR_EDamageIntensityType.NO_DAMAGE;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_EDamageIntensityType GetValidIntensityType(notnull HitZone hitZone)
	{
		SCR_DamageIntensityEntry entry = GetValidIntensityEntry(hitZone);
		
		if (entry)
			return entry.GetDamageIntensityType();
		
		return SCR_EDamageIntensityType.NO_DAMAGE;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_EDamageIntensityType GetValidIntensityType(notnull SCR_DamageManagerComponent damageManager)
	{
		SCR_DamageIntensityEntry entry = GetValidIntensityEntry(damageManager);
		
		if (entry)
			return entry.GetDamageIntensityType();
		
		return SCR_EDamageIntensityType.NO_DAMAGE;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_DamageIntensityEntry GetIntensityEntry(SCR_EDamageIntensityType type)
	{
		SCR_DamageIntensityEntry entry;
		for (int i = 0, count = m_SortedDamageIntensityEntries.Count(); i < count; i++)
		{
			entry = m_SortedDamageIntensityEntries.Get(i);
			if (entry && entry.GetDamageIntensityType() == type)
				return entry;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_UIName GetIntensityUIInfo(SCR_EDamageIntensityType type)
	{
		SCR_DamageIntensityEntry entry =  GetIntensityEntry(type);
		if (entry)
			return entry.GetUiInfo();
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetIntensityName(SCR_EDamageIntensityType type)
	{
		SCR_UIName uiInfo = GetIntensityUIInfo(type);
		if (uiInfo)
			return uiInfo.GetName();
		
		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_DamageIntensityHolder()
	{
		m_SortedDamageIntensityEntries = new SCR_SortedArray<SCR_DamageIntensityEntry>();
		foreach (SCR_DamageIntensityEntry entry : m_aDamageIntensityEntries)
		{
			if (!entry || !entry.m_bIsEnabled || !entry.GetUiInfo())
				continue;
			
			m_SortedDamageIntensityEntries.Insert(entry.m_iOrder, entry);
		}
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_EDamageIntensityType, "m_eDamageIntensityType")]
class SCR_DamageIntensityEntry
{
	[Attribute(desc: "The intensity enum associated with the min and max damage", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EDamageIntensityType))]
	protected SCR_EDamageIntensityType m_eDamageIntensityType;
	
	[Attribute("1", desc: "If false it will be ignored")]
	bool m_bIsEnabled;
	
	[Attribute(desc: "A lower order means it is checked before any with a higher order")]
	int m_iOrder;
	
	[Attribute(desc: "Damage state UI info. If left null the entry will be ignored")]
	protected ref SCR_UIName m_UiInfo;
	
	[Attribute(desc: "If value (generally health scaled) is equal or greater than min and smaller (or equal and smaller if 1 or 0) than max than this intensity entry is valid", params: "0 inf")]
	protected float m_fMaxValue;
	
	[Attribute(desc: "If value (generally health scaled) is equal or greater than min and smaller than max than this intensity entry is valid. Generally this is damage scaled but does not need to be", params: "0 inf")]
	protected float m_fMinValue;
	
	//------------------------------------------------------------------------------------------------
	SCR_UIName GetUiInfo()
	{
		return m_UiInfo;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_EDamageIntensityType GetDamageIntensityType()
	{
		return m_eDamageIntensityType;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsValidDamageState(float value)
	{
		if (m_fMaxValue == 0)
			return (value >= m_fMinValue && value <= m_fMaxValue);
		else 
			return (value >= m_fMinValue && value < m_fMaxValue);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_DamageIntensityEntry()
	{
		if (m_fMinValue > m_fMaxValue)
		{
			Print("'SCR_DamageIntensityEntry': '" + typename.EnumToString(SCR_EDamageIntensityType, m_eDamageIntensityType) + "' has a min value that is greater than max value! Min value is set the same as max value", LogLevel.WARNING);
			m_fMinValue = m_fMaxValue;
		}
	}
}