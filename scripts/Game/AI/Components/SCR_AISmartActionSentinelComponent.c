enum ELeaningType
{
	LEFT = -1,
	NONE = 0,
	RIGHT = 1,
}

class SCR_AISmartActionSentinelComponentClass : SCR_AISmartActionComponentClass
{
	[Attribute("0 0 0", UIWidgets.EditBox, desc: "Position where AI will look from action offset (in local coords of the object entity)", params: "inf inf 0 purpose=coords space=entity")]
	protected vector m_vLookPosition;
	
	[Attribute("180", UIWidgets.Coords, desc: "Range of rotation within which AI will restrict their observing")]
	protected float m_fLookDirectionRange;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "Desired stance the AI will use upon reaching the observationpost", "", ParamEnumArray.FromEnum(ECharacterStance))]
	protected int m_iCharacterStance;
	
	[Attribute("0", uiwidget: UIWidgets.CheckBox, "Whether the AI will be using binoculars during observation", "")]
	protected bool m_bUseBinoculars;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "Whether the AI will occasionally lean to side", "", ParamEnumArray.FromEnum(ELeaningType))]
	protected ELeaningType m_eLeaningType;
	
	[Attribute("0", UIWidgets.ComboBox, "Loiter animation to be played.", "", ParamEnumArray.FromEnum(ELoiteringType))];
	protected ELoiteringType m_eLoiterAnimation;
	
	[Attribute("false", UIWidgets.CheckBox, "Holster weapon during loiter animation?")];
	protected bool m_bHolsterWeapon;

	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetLookPosition()
	{
		return m_vLookPosition;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetLookDirectionRange()
	{
		return m_fLookDirectionRange;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetDesiredStance()
	{
		return m_iCharacterStance;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetUseBinoculars()
	{
		return m_bUseBinoculars;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ELeaningType GetLeaningType()
	{
		return m_eLeaningType;
	}
	
	//----------------------------------------------------------------------------------------
	ELoiteringType GetLoiterAnimation()
	{
		return m_eLoiterAnimation;
	}
	
	//----------------------------------------------------------------------------------------
	bool GetHolsterWeapon()
	{
		return m_bHolsterWeapon;
	}
}

class SCR_AISmartActionSentinelComponent : SCR_AISmartActionComponent
{
	SCR_AISmartActionSentinelComponentClass GetPrefabData()
	{
		return SCR_AISmartActionSentinelComponentClass.Cast(GetComponentData(GetOwner()));
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetLookPosition()
	{
		SCR_AISmartActionSentinelComponentClass prefabData = GetPrefabData();
		return prefabData.GetLookPosition();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetLookDirectionRange()
	{
		SCR_AISmartActionSentinelComponentClass prefabData = GetPrefabData();
		return prefabData.GetLookDirectionRange();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetDesiredStance()
	{
		SCR_AISmartActionSentinelComponentClass prefabData = GetPrefabData();
		return prefabData.GetDesiredStance();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetUseBinoculars()
	{
		SCR_AISmartActionSentinelComponentClass prefabData = GetPrefabData();
		return prefabData.GetUseBinoculars();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ELeaningType GetLeaningType()
	{
		SCR_AISmartActionSentinelComponentClass prefabData = GetPrefabData();
		return prefabData.GetLeaningType();
	}
	
	//----------------------------------------------------------------------------------------
	ELoiteringType GetLoiterAnimation()
	{
		SCR_AISmartActionSentinelComponentClass prefabData = GetPrefabData();
		return prefabData.GetLoiterAnimation();
	}
	
	//----------------------------------------------------------------------------------------
	bool GetHolsterWeapon()
	{
		SCR_AISmartActionSentinelComponentClass prefabData = GetPrefabData();
		return prefabData.GetHolsterWeapon();
	}
}
