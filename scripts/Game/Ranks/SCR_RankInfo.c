[BaseContainerProps()]
class SCR_RankInfo
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.ComboBox, desc: "Rank ID", enums: ParamEnumArray.FromEnum(SCR_ECharacterRank))]
	protected SCR_ECharacterRank m_iRank;
	
	[Attribute("0", UIWidgets.CheckBox, "Renegade", "Is this rank considered hostile by friendlies?")]
	protected bool m_bIsRenegade;
	
	[Attribute("100", UIWidgets.EditBox, "XP required to get promoted to this rank.")]
	protected int m_iRequiredXP;
	
	[Attribute(defvalue: "", uiwidget: UIWidgets.EditBox, desc: "Rank name")]
	protected string m_sRankName;
	
	[Attribute(defvalue: "", uiwidget: UIWidgets.EditBox, desc: "Rank name (upper case)")]
	protected string m_sRankNameUpper;
	
	[Attribute(defvalue: "", uiwidget: UIWidgets.EditBox, desc: "Rank name (short)")]
	protected string m_sRankNameShort;
	
	[Attribute("", "Rank insignia quad name in MilitaryIcons.imageset")]
	protected string m_sInsignia;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ECharacterRank GetRankID()
	{
		return m_iRank;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool IsRankRenegade()
	{
		return m_bIsRenegade;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetRequiredRankXP()
	{
		return m_iRequiredXP;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	string GetRankName()
	{
		return m_sRankName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	string GetRankNameUpperCase()
	{
		return m_sRankNameUpper;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	string GetRankNameShort()
	{
		return m_sRankNameShort;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	string GetRankInsignia()
	{
		return m_sInsignia;
	}
}

//~ Character ranks. Take note to update any changes in SCR_EntityCatalogSpawnerData and EEditableEntityBudget
enum SCR_ECharacterRank
{
	RENEGADE,
	PRIVATE,
	CORPORAL,
	SERGEANT,
	LIEUTENANT,
	CAPTAIN,
	MAJOR,
	COLONEL,
	GENERAL,
	CUSTOM1,
	CUSTOM2,
	CUSTOM3,
	CUSTOM4,
	CUSTOM5,
	CUSTOM6,
	CUSTOM7,
	CUSTOM8,
	CUSTOM9,
	CUSTOM10,
	INVALID
}