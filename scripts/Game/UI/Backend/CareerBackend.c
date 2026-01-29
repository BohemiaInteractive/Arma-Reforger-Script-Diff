class GameplayTrait : JsonApiStruct
{
	protected EProfileSkillID m_eTraitID;
	protected int m_iTraitXP;	
	
	void GameplayTrait(EProfileSkillID thisId, int n)
	{
		m_eTraitID = thisId;
		m_iTraitXP = n;
		
		RegV("m_eTraitID");
		RegV("m_iTraitXP");
	}
	
	EProfileSkillID GetTraitID()
	{
		return m_eTraitID;
	}
	
	int GetTraitXP()
	{
		return m_iTraitXP;
	}
	
	void AddXP(int n)
	{
		m_iTraitXP+=n;
	}
	
	#ifdef ENABLE_DIAG
	string toString()
	{
		string toRet = "";
		toRet += "Gameplay Trait ID is "+m_eTraitID;
		toRet+= ", XP is "+m_iTraitXP;
		return toRet;
	}
	#endif
};



class CareerBackendData : JsonApiStruct
{	
	protected ref SCR_SessionInfo m_sessionInfo;
	protected static const int SESSIONS_TO_STORE = 8;
	
	// Backend-stored variables
	float m_fRank;
	protected int m_iKills;
	protected int m_iFriendlyKills;
	protected int m_iDeaths;
	protected float m_fTravelledDistance;
	
	protected ref array<ref GameplayTrait> m_aGameplayTraits;
	protected ref array<ref SCR_SessionInfo> m_aSessionInfos;
	
	//------------------------------------------------------------------------------------------------
	void AddKill(bool friendly = false)
	{
		if (friendly)
			m_iFriendlyKills++;
		else
			m_iKills++;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddDeath()
	{
		m_iDeaths++;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddSkillXP(EProfileSkillID skillID, int XP)
	{
		if(skillID >= 0 && skillID < m_aGameplayTraits.Count())
			m_aGameplayTraits.Get(skillID).AddXP(XP);
		else
			Print("CareerBackend::AddSkillXP Wrong skillID! Can't add experience to the unknown skillID: "+skillID, LogLevel.ERROR);
	}
	
	//------------------------------------------------------------------------------------------------
	float GetTravelledDistance()
	{
		return m_fTravelledDistance;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetFriendlyKills()
	{
		return m_iFriendlyKills;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetKills()
	{
		return m_iKills;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetDeaths()
	{
		return m_iDeaths;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetSkillXP(EProfileSkillID skillID)
	{
		if(skillID >= 0 && skillID < m_aGameplayTraits.Count())
			return m_aGameplayTraits.Get(skillID).GetTraitXP();
		else
		{
			Print("CareerBackend::GetSkillXP Wrong skillID!", LogLevel.ERROR);
			return -1;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//Called upon faction selection
	void SetFaction(string factionKey)
	{
		GetSessionInfo().SetFactionKey(factionKey);
	}
	
	//------------------------------------------------------------------------------------------------
	//Called upon login
	void SetLoginTime()
	{
		int year, month, day, hour, minute, second;
		
		System.GetYearMonthDay(year, month, day);
		System.GetHourMinuteSecond(hour, minute, second);
		
		GetSessionInfo().SetLoginTime(year, month, day, hour, minute, second);
	}
	
	//------------------------------------------------------------------------------------------------
	//Called upon logout
	void SetLogoutTime()
	{
		int year, month, day, hour, minute, second;
		
		System.GetYearMonthDay(year, month, day);
		System.GetHourMinuteSecond(hour, minute, second);
		
		GetSessionInfo().SetLogoutTime(year, month, day, hour, minute, second);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_SessionInfo GetSessionInfo()
	{
		if (!m_sessionInfo)
		{
			m_sessionInfo = new SCR_SessionInfo();
			BackendApi bApi = GetGame().GetBackendApi();
		
			if (bApi)
			{
				DSSession session = bApi.GetDSSession();
				
				if (session)
					m_sessionInfo.SetRoomID(session.RoomID());
			}
		}
		
		return m_sessionInfo;
	}
	
	//****************//
	//OVERRIDE METHODS//
	//****************//
	
	//------------------------------------------------------------------------------------------------
	override void OnSuccess( int errorCode )
	{
		// Register infos array if needed, resize if it overflows max size, make sure newest data is first
		int sessionsCnt;
		
		if (!m_aSessionInfos)
			m_aSessionInfos = new array<ref SCR_SessionInfo>();
		else
		{
			sessionsCnt = m_aSessionInfos.Count();
			
			if (sessionsCnt >= SESSIONS_TO_STORE)
			{
				m_aSessionInfos.Resize(SESSIONS_TO_STORE - 1);
				sessionsCnt = SESSIONS_TO_STORE - 1;
			}
		}
		
		// Check m_aSessionInfos for data stored for this session
		BackendApi bApi = GetGame().GetBackendApi();
		
		if (bApi && sessionsCnt != 0)
		{
			DSSession session = bApi.GetDSSession();
			
			if (session)
			{
				string roomID = session.RoomID();
				
				if (!roomID.IsEmpty())
				{
					for (int i = 0; i < sessionsCnt; i++)
					{
						SCR_SessionInfo sessionInfo = m_aSessionInfos[i];
						
						if (!sessionInfo)
							continue;
						
						if (sessionInfo.GetRoomID() == roomID)
						{
							// Session data found - register and set as the first element
							m_sessionInfo = sessionInfo;
							m_aSessionInfos.RemoveOrdered(i);
							m_aSessionInfos.InsertAt(m_sessionInfo, 0);
							break;
						}
					}
				}
			}
		}
		
		// Session data not stored, store new instance if backend returns proper ID
		if (!m_sessionInfo)
		{
			SCR_SessionInfo sessionInfo = GetSessionInfo();
			
			if (!sessionInfo.GetRoomID().IsEmpty())
				m_aSessionInfos.InsertAt(GetSessionInfo(), 0);
		}
		
		if (!CareerMenuUI.m_sInstance)
			return;
		
		if (CareerMenuUI.m_sInstance)
			CareerMenuUI.m_sInstance.UpdateCareerData();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnError( int errorCode )
	{
        Print("OnError: " + errorCode );
	}
	
	//------------------------------------------------------------------------------------------------
	void CareerBackendData()
	{
		m_aGameplayTraits = {};
		
		typename type = EProfileSkillID;
		
		for(int i = 0; i < type.GetVariableCount(); i++) 
		{
			m_aGameplayTraits.Insert(new GameplayTrait(i, 0));
		}
		
		RegV("m_fRank");
		RegV("m_iKills");
		RegV("m_iFriendlyKills");
		RegV("m_iDeaths");
		RegV("m_fTravelledDistance");
		
		RegV("m_aGameplayTraits");
		RegV("m_aSessionInfos");
	}
};

//------------------------------------------------------------------------------------------------
//! Used to identify various player skills
enum EProfileSkillID
{
	GLOBAL,
	WEAPON_HANDLER,
	DRIVER,
	SCOUT,
	OPERATOR
};