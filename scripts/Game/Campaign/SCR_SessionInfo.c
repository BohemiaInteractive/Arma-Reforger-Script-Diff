class SCR_SessionInfo : JsonApiStruct
{
	protected WorldTimestamp m_fLoginTimestamp;
	
	// Backend-stored variables
	protected string m_sRoomID;
	protected FactionKey m_sFactionKey;
	protected int m_iLoginYear;
	protected int m_iLoginMonth;
	protected int m_iLoginDay;
	protected int m_iLoginHour;
	protected int m_iLoginMinute;
	protected int m_iLoginSecond;
	protected int m_iLogoutYear;
	protected int m_iLogoutMonth;
	protected int m_iLogoutDay;
	protected int m_iLogoutHour;
	protected int m_iLogoutMinute;
	protected int m_iLogoutSecond
	protected float m_fTimeInSession;
	
	//------------------------------------------------------------------------------------------------
	void SetRoomID(string ID)
	{
		m_sRoomID = ID;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetRoomID()
	{
		return m_sRoomID;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFactionKey(FactionKey factionKey)
	{
		m_sFactionKey = factionKey;
	}
	
	//------------------------------------------------------------------------------------------------
	FactionKey GetFactionKey()
	{
		return m_sFactionKey;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLoginTime(int year, int month, int day, int hour, int minute, int second)
	{
		m_iLoginYear = year;
		m_iLoginMonth = month;
		m_iLoginDay = day;
		m_iLoginHour = hour;
		m_iLoginMinute = minute;
		m_iLoginSecond = second;
		ChimeraWorld world = GetGame().GetWorld();
		m_fLoginTimestamp = world.GetServerTimestamp();
	}
	
	//------------------------------------------------------------------------------------------------
	void GetLoginTime(out notnull array<int> loginData)
	{
		loginData = {m_iLoginYear, m_iLoginMonth, m_iLoginDay, m_iLoginHour, m_iLoginMinute, m_iLoginSecond};
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLogoutTime(int year, int month, int day, int hour, int minute, int second)
	{
		m_iLogoutYear = year;
		m_iLogoutMonth = month;
		m_iLogoutDay = day;
		m_iLogoutHour = hour;
		m_iLogoutMinute = minute;
		m_iLogoutSecond = second;
		ChimeraWorld world = GetGame().GetWorld();
		m_fTimeInSession = world.GetServerTimestamp().DiffMilliseconds(m_fLoginTimestamp);
	}
	
	//------------------------------------------------------------------------------------------------
	void GetLogoutTime(out notnull array<int> logoutData)
	{
		logoutData = {m_iLogoutYear, m_iLogoutMonth, m_iLogoutDay, m_iLogoutHour, m_iLogoutMinute, m_iLogoutSecond};
	}
	
	//------------------------------------------------------------------------------------------------
	float GetTimeInSession()
	{
		return m_fTimeInSession;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_SessionInfo()
	{
		RegV("m_sRoomID");
		RegV("m_sFactionKey");
		
		RegV("m_iLoginYear");
		RegV("m_iLoginMonth");
		RegV("m_iLoginDay");
		RegV("m_iLoginHour");
		RegV("m_iLoginMinute");
		RegV("m_iLoginSecond");
		
		RegV("m_iLogoutYear");
		RegV("m_iLogoutMonth");
		RegV("m_iLogoutDay");
		RegV("m_iLogoutHour");
		RegV("m_iLogoutMinute");
		RegV("m_iLogoutSecond");
		
		RegV("m_fTimeInSession");
	}
};