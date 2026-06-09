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

		SpecialRankHandling(newRank, prevRank);
	}

	//------------------------------------------------------------------------------------------------
	//! Helper method for specific ranks with custom logic attached to them
	protected void SpecialRankHandling(SCR_ECharacterRank newRank, SCR_ECharacterRank prevRank)
	{
		// this logic should currently only be triggered when the renegade faction is configured! otherwise we ignore this logic
		SCR_CampaignFaction Renegade = SCR_GameModeCampaign.GetInstance().GetFactionByEnum(SCR_ECampaignFaction.RNGD);

		if (!Renegade)
			return;

		// RNGD (Renegades) is a "hidden faction" set up for when players get kicked out of their faction.
		if (newRank == SCR_ECharacterRank.RENEGADE && GetCharacterFaction(m_Owner).IsRenegadePunishedExile())
		{
			AttemptSwitchFaction(Renegade);
			return;
		}

		// Currently FIA is the only one to use the punishment mechanic, so if they are a renegade we can assume they came from
		// the FIA faction. When they regain their rank they are able to rejoin FIA.
		if (prevRank == SCR_ECharacterRank.RENEGADE && (GetCharacterFaction(m_Owner) == Renegade))
			AttemptSwitchFaction(SCR_GameModeCampaign.GetInstance().GetFactionByEnum(SCR_ECampaignFaction.INDFOR));
	}

	//------------------------------------------------------------------------------------------------
	//! Helper method for attempting to switch factions
	protected void AttemptSwitchFaction(SCR_CampaignFaction campaignFaction)
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		bool isServer = (gameMode && gameMode.IsMaster()) || (!gameMode && Replication.IsServer());

		if (!isServer)
			return;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(m_Owner);
		if (!character)
			return;

		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
				return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(character);

		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!playerController)
			return;

		SCR_PlayerFactionAffiliationComponent playerFactionAffiliation = SCR_PlayerFactionAffiliationComponent.Cast(playerController.FindComponent(SCR_PlayerFactionAffiliationComponent));
		if (!playerFactionAffiliation)
			return;

		// self hosted clients might give an immediate response so we must be sure to be ready before attempting to set the faction
		// if we notice the faction request didnt go through we dont need to watch for the response
		playerFactionAffiliation.GetOnPlayerFactionResponseInvoker_S().Insert(FinishFactionSwitch);
		if (!factionManager.SetPlayerFaction(character, campaignFaction))
			playerFactionAffiliation.GetOnPlayerFactionResponseInvoker_S().Remove(FinishFactionSwitch);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Callback method for ensuring that completed faction switches are set up correctly
	protected void FinishFactionSwitch(SCR_PlayerFactionAffiliationComponent component, int factionIndex, bool response)
	{
		component.GetOnPlayerFactionResponseInvoker_S().Remove(FinishFactionSwitch);

		if (!response)
			return;

		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(component.GetPlayerId());
		if (!groupController)
			return;

		groupController.CreateAndJoinGroup(component.GetAffiliatedFaction(), true);
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
		
		return faction.GetRanks().GetRankName(rank);
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
		
		return faction.GetRanks().GetRankName(rank);
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
		
		return faction.GetRanks().GetRankNameUpperCase(rank);
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
		
		return faction.GetRanks().GetRankNameShort(rank);
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

		return faction.GetRanks().GetRankInsignia(rank);
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
