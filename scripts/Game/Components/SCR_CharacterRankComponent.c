[EntityEditorProps(category: "GameScripted", description: "Handles the character's rank.", color: "0 0 255 255")]
class SCR_CharacterRankComponentClass: ScriptComponentClass
{
}

class SCR_CharacterRankComponent : ScriptComponent
{	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.ComboBox, desc: "Rank", enums: ParamEnumArray.FromEnum(SCR_ECharacterRank))]
	protected SCR_ECharacterRank m_iRank;
	
	protected IEntity m_Owner;
	static ref ScriptInvoker s_OnRankChanged = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] newRank
	//! \param[in] prevRank
	//! \param[in] silent
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDoSetCharacterRank(SCR_ECharacterRank newRank, SCR_ECharacterRank prevRank, bool silent)
	{
		SCR_ECharacterRank oldRank = m_iRank;
		m_iRank = newRank;
		OnRankChanged(oldRank, newRank, silent);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] unit
	//! \return
	static SCR_CharacterRankComponent GetCharacterRankComponent(IEntity unit)
	{
		return SCR_CharacterRankComponent.Cast(unit.FindComponent(SCR_CharacterRankComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] rank
	//! \param[in] silent
	void SetCharacterRank(SCR_ECharacterRank rank, bool silent = false)
	{
		if (rank != m_iRank)
		{
			Rpc(RpcDoSetCharacterRank, rank, m_iRank, silent);
			RpcDoSetCharacterRank(rank, m_iRank, silent);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRankChanged(SCR_ECharacterRank prevRank, SCR_ECharacterRank newRank, bool silent)
	{
		s_OnRankChanged.Invoke(prevRank, newRank, m_Owner, silent);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Helper method to easily read a character's rank by providing just the character parameter
	//! \param[in] unit
	//! \return
	static SCR_ECharacterRank GetCharacterRank(IEntity unit)
	{
		if (!unit)
			return SCR_ECharacterRank.INVALID;
		
		SCR_CharacterRankComponent comp = GetCharacterRankComponent(unit);
		
		if (!comp)
			return SCR_ECharacterRank.INVALID;
		
		return comp.GetCharacterRank();
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_Faction GetCharacterFaction(IEntity unit)
	{
		if (!unit)
			return null;
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(unit);
		if (!character)
			return null;
		
		Faction faction = character.GetFaction();
		if (!faction)
			return null;

		return SCR_Faction.Cast(faction);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return name of passed rank.
	//! \param[in] unit
	//! \param[in] rank
	//! \return
	static string GetRankName(IEntity unit, SCR_ECharacterRank rank)
	{
		if (!unit)
			return string.Empty;
		
		SCR_CharacterRankComponent comp = GetCharacterRankComponent(unit);
		if (!comp)
			return string.Empty;

		SCR_Faction faction = comp.GetCharacterFaction(unit);
		if (!faction)
			return string.Empty;
		
		return faction.GetRankName(rank);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] unit
	//! \return
	static string GetCharacterRankName(IEntity unit)
	{
		if (!unit)
			return "";
		
		SCR_CharacterRankComponent comp = GetCharacterRankComponent(unit);
		
		if (!comp)
			return "";
		
		SCR_ECharacterRank rank = comp.GetCharacterRank();
		SCR_Faction faction = comp.GetCharacterFaction(unit);
		
		if (!faction)
			return "";
		
		return faction.GetRankName(rank);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] unit
	//! \return
	static string GetCharacterRankNameUpperCase(IEntity unit)
	{
		if (!unit)
			return "";
		
		SCR_CharacterRankComponent comp = GetCharacterRankComponent(unit);
		
		if (!comp)
			return "";
		
		SCR_ECharacterRank rank = comp.GetCharacterRank();
		SCR_Faction faction = comp.GetCharacterFaction(unit);
		
		if (!faction)
			return "";
		
		return faction.GetRankNameUpperCase(rank);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] unit
	//! \return
	static string GetCharacterRankNameShort(IEntity unit)
	{
		if (!unit)
			return "";
		
		SCR_CharacterRankComponent comp = GetCharacterRankComponent(unit);
		
		if (!comp)
			return "";
		
		SCR_ECharacterRank rank = comp.GetCharacterRank();
		SCR_Faction faction = comp.GetCharacterFaction(unit);
		
		if (!faction)
			return "";
		
		return faction.GetRankNameShort(rank);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] unit
	//! \return
	static ResourceName GetCharacterRankInsignia(IEntity unit)
	{
		if (!unit)
			return "";
		
		SCR_CharacterRankComponent comp = GetCharacterRankComponent(unit);
		
		if (!comp)
			return "";
		
		SCR_ECharacterRank rank = comp.GetCharacterRank();
		SCR_Faction faction = comp.GetCharacterFaction(unit);
		
		if (!faction)
			return "";
		
		return faction.GetRankInsignia(rank);
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_ECharacterRank GetCharacterRank()
	{
		return m_iRank;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteIntRange(m_iRank, 0, SCR_ECharacterRank.INVALID-1);
		
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		reader.ReadIntRange(m_iRank, 0, SCR_ECharacterRank.INVALID-1);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (!ChimeraCharacter.Cast(owner))
			Print("SCR_CharacterRankComponent must be attached to ChimeraCharacter!", LogLevel.ERROR);
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_CharacterRankComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_Owner = ent;
	}
}

[BaseContainerProps()]
class SCR_CharacterRank
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.ComboBox, desc: "Rank ID set in FactionManager", enums: ParamEnumArray.FromEnum(SCR_ECharacterRank))]
	protected SCR_ECharacterRank m_iRank;
	
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

[BaseContainerProps()]
class SCR_RankID
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.ComboBox, desc: "Rank ID", enums: ParamEnumArray.FromEnum(SCR_ECharacterRank))]
	protected SCR_ECharacterRank m_iRank;
	
	[Attribute("0", UIWidgets.CheckBox, "Renegade", "Is this rank considered hostile by friendlies?")]
	protected bool m_bIsRenegade;
	
	[Attribute("100", UIWidgets.EditBox, "XP required to get promoted to this rank.")]
	protected int m_iRequiredXP;
	
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
