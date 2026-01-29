[EntityEditorProps(category: "GameScripted/GameMode", description: "Takes care of loading and storing player profile data.", color: "0 0 255 255")]
class SCR_PlayerProfileManagerComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_PlayerProfileManagerComponent : SCR_BaseGameModeComponent
{
	//************************//
	//RUNTIME STATIC VARIABLES//
	//************************//
	protected static SCR_RespawnSystemComponent s_RespawnSystemComponent = null;
	
	//*****************//
	//MEMBER ATTRIBUTES//
	//*****************//
	[Attribute("1", "Refresh time for profile loading. [s]")]
	protected float m_fRefreshTime;
	
	//************************//
	//RUNTIME MEMBER VARIABLES//
	//************************//
	protected ref map<int, ref CareerBackendData> m_mPlayerProfiles = null;
	protected ref array<int> m_aPlayerIDsToLoadProfile = {};
	protected float m_fCurrentRefreshTime = 1;
	protected ref BackendCallback m_Callback = new BackendCallback();
	
	//------------------------------------------------------------------------------------------------
	protected Faction GetPlayerFaction(int playerID)
	{
		return SCR_FactionManager.SGetPlayerFaction(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \return player profile from playerID
	CareerBackendData GetPlayerProfile(int playerID)
	{
		if (m_mPlayerProfiles)
			return m_mPlayerProfiles.Get(playerID);
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnPlayerKilled(notnull SCR_InstigatorContextData instigatorContextData)
	{
		super.OnPlayerKilled(instigatorContextData);
		
		CareerBackendData victimProfile = GetPlayerProfile(instigatorContextData.GetVictimPlayerID());
		int killerId = instigatorContextData.GetKillerPlayerID();
		
		// If victim profile exists, then add death no matter what
		if (victimProfile)
			victimProfile.AddDeath();
		
		//~ Not a player kill so ignore (Like suicide)
		if (!instigatorContextData.HasAnyVictimKillerRelation(SCR_ECharacterDeathStatusRelations.KILLED_BY_ENEMY_PLAYER | SCR_ECharacterDeathStatusRelations.KILLED_BY_FRIENDLY_PLAYER))
			return;
		
		//~ Possessed AI kills are never counted. Though any kills made without possessing, admin or not, will count
		SCR_ECharacterControlType killerControlType = instigatorContextData.GetKillerCharacterControlType();
		if (killerControlType == SCR_ECharacterControlType.POSSESSED_AI)
			return;
		
		//~ Check if killer profile exists
		CareerBackendData killerProfile = GetPlayerProfile(killerId);
		if (!killerProfile)
			return;

		//~ Is a teamkill, so add teamkill to profile
		if (instigatorContextData.HasAnyVictimKillerRelation(SCR_ECharacterDeathStatusRelations.KILLED_BY_FRIENDLY_PLAYER))
		{						
			if (instigatorContextData.DoesPlayerKillCountAsTeamKill())
				killerProfile.AddKill(true);
			return;
		}
			
		// It wasn't a team kill, add a regular kill
		killerProfile.AddKill();
	}
	
	//------------------------------------------------------------------------------------------------
	//! What happens when a player disconnects.
	//! \param[in] playerID is a unique player identifier that defines which player has disconnected.
	//! \param[in] cause
	//! \param[in] timeout
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		StoreProfile(playerId, true);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	//! \param[in] disconnecting
	void StoreProfile(int playerID, bool disconnecting = false)
	{
		if (!GetGame().GetBackendApi())
			return;
		
		CareerBackendData playerProfile = GetPlayerProfile(playerID);
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!playerProfile || !m_Callback || !campaign)
			return;
		
		if (disconnecting)
			playerProfile.SetLogoutTime();
		
		#ifndef WORKBENCH
			GetGame().GetBackendApi().PlayerRequest(EBackendRequest.EBREQ_GAME_CharacterUpdateS2S,m_Callback,playerProfile,playerID);
		#else
			GetGame().GetBackendApi().PlayerRequest(EBackendRequest.EBREQ_GAME_DevCharacterUpdate,m_Callback,playerProfile,playerID);
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	//! \return
	bool LoadPlayerProfileFromBackend(int playerID)
	{
		if (m_mPlayerProfiles && GetGame().GetBackendApi())
		{
			if (GetGame().GetBackendApi().GetDSSession() && GetGame().GetBackendApi().GetDSSession().Status() == EDsSessionState.EDSESSION_ACTIVE)
			{
				CareerBackendData playerProfile = new CareerBackendData();
				m_mPlayerProfiles.Set(playerID, playerProfile);
				playerProfile = GetPlayerProfile(playerID);
				
				if (m_Callback)
					GetGame().GetBackendApi().PlayerData(playerProfile, playerID);
				
				return true;
			}
			else
			{
				return false;
			}
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	void LoadConnectingPlayerProfile(int playerID)
	{
		if (!LoadPlayerProfileFromBackend(playerID))
		{
			m_aPlayerIDsToLoadProfile.Insert(playerID);
		}
		else
		{
			CareerBackendData playerProfile = GetPlayerProfile(playerID);
			
			if (!playerProfile)
				return;
			
			playerProfile.SetLoginTime();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		m_fCurrentRefreshTime -= timeSlice;
		
		if (m_fCurrentRefreshTime > 0)
			return;
		
		m_fCurrentRefreshTime = m_fRefreshTime;
		
		for (int count = m_aPlayerIDsToLoadProfile.Count(), i = count - 1; i >= 0; i--)
		{
			bool success = LoadPlayerProfileFromBackend(m_aPlayerIDsToLoadProfile[i]);
			
			if (success)
			{
				CareerBackendData playerProfile = GetPlayerProfile(m_aPlayerIDsToLoadProfile[i]);
				
				if (!playerProfile)
					return;
				
				playerProfile.SetLoginTime();
				m_aPlayerIDsToLoadProfile.Remove(i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
		owner.SetFlags(EntityFlags.NO_TREE | EntityFlags.NO_LINK);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_fCurrentRefreshTime = m_fRefreshTime;
		m_mPlayerProfiles = new map<int, ref CareerBackendData>();
		SCR_RespawnSystemComponent respawnSystem = SCR_RespawnSystemComponent.Cast(owner.FindComponent(SCR_RespawnSystemComponent));
		
		if (!respawnSystem)
		{
			Print("There is no RespawnSystemComponent attached to the GameMode entity. Faction scoring will not work.", LogLevel.WARNING);
			return;
		}
		
		s_RespawnSystemComponent = respawnSystem;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_PlayerProfileManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}
}
