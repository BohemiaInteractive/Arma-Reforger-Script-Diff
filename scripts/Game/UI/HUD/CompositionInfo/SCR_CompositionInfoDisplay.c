class SCR_CompositionInfoDisplay : SCR_InfoDisplayExtended
{
	protected const string TIME_FORMAT_SINGULAR = "_LC";
	protected const string TIME_FORMAT_HOUR = "#AR-Time_HourAgo_LC";
	protected const string TIME_FORMAT_MINUTE = "#AR-Time_MinuteAgo_LC";
	protected const string TIME_FORMAT_SECOND = "#AR-Time_SecondAgo_LC";
	protected const string TIME_FORMAT_HOURS = "#AR-Time_HoursAgo_LC";
	protected const string TIME_FORMAT_MINUTES = "#AR-Time_MinutesAgo_LC";
	protected const string TIME_FORMAT_SECONDS = "#AR-Time_SecondsAgo_LC";
	
	[Attribute("1.0")]
	protected float m_fReportedCooldown;
	
	protected ref SCR_CompositionInfoWidgets m_InfoWidgets;
	protected ref SCR_CompositionDetails m_Details;
	
	protected float m_fReportedTimer;
	
	//------------------------------------------------------------------------------------------------	
	override void DisplayStartDraw(IEntity owner)
	{		
		if (!m_wRoot)
			return;
		
		m_InfoWidgets = new SCR_CompositionInfoWidgets();
		m_InfoWidgets.Init(m_wRoot);
	}
	
	//------------------------------------------------------------------------------------------------	
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{		
		if (!m_InfoWidgets)
			return;
		
		if (m_fReportedTimer > 0)
			m_fReportedTimer -= timeSlice;
		
		if (IsShown() && m_Details && !m_Details.m_sAuthorUID.IsEmpty())
			UpdateLine(m_Details);
	}
	
	//------------------------------------------------------------------------------------------------	
	protected void UpdateLine(SCR_CompositionDetails details)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace || !m_InfoWidgets.m_wLinePoint)
			return;
		
		float lineX, lineY;
		vector direction;
		m_InfoWidgets.m_wLinePoint.GetScreenPos(lineX, lineY);
		vector line[2];
		line[0] = workspace.ProjScreenToWorld(workspace.DPIUnscale(lineX), workspace.DPIUnscale(lineY), direction, GetGame().GetWorld());
		line[0] = line[0] + direction * 100;
		
		line[1] = details.m_vPosition;
		Shape.CreateLines(UIColors.CONTRAST_COLOR.PackToInt(), ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, line, 2);
		
		vector projectedPos = workspace.ProjWorldToScreen(details.m_vPosition, GetGame().GetWorld());
		
		FrameSlot.SetPos(m_InfoWidgets.m_wEntityPivot, projectedPos[0], projectedPos[1]);
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowReported()
	{
		m_InfoWidgets.m_wReportedInfo.SetVisible(true);
		m_InfoWidgets.m_wAuthorDetails.SetVisible(false);
		m_InfoWidgets.m_wDetailsInfo.SetVisible(false);
		
		m_fReportedTimer = m_fReportedCooldown;
	}
	
	//------------------------------------------------------------------------------------------------
	void HideReported()
	{
		m_InfoWidgets.m_wReportedInfo.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------	
	void UpdateCompositionDetails(SCR_CompositionDetails details)
	{
		bool shouldShow = details != null && !details.m_sAuthorUID.IsEmpty();
		
		if (!m_InfoWidgets || !IsShown() || m_fReportedTimer > 0)
			return;
		
		m_Details = details;
		
		HideReported();
		m_InfoWidgets.m_wDetailsInfo.SetVisible(!shouldShow);
		m_InfoWidgets.m_wAuthorDetails.SetVisible(shouldShow);
		m_InfoWidgets.m_wEntityPivot.SetVisible(shouldShow);
		
		// Disabled Reporting -> Rewrite due to ID
		//if (shouldShow)
		//	m_InfoWidgets.m_wReportButton.SetVisible((details.m_iAuthorUID > -1 && m_PlayerController.GetPlayerId() != details.m_iAuthorUID));
		//else
		//	m_InfoWidgets.m_wReportButton.SetVisible(false);
		
		if (!shouldShow)
			return;
		
		int currentUnixTime = System.GetUnixTime();
		int hours, minutes, seconds;
		int deltaTime = currentUnixTime - details.m_iLastUpdated;
		
		seconds = deltaTime % 60;
		minutes = ((deltaTime - seconds) / 60) % 60;
		hours = ((deltaTime - ((minutes * 60) + seconds)) / (60 * 60));
		
		string format;
		if (hours > 0)
		{
			format = TIME_FORMAT_HOURS;
			if (hours == 1)
				format = TIME_FORMAT_HOUR;
			m_InfoWidgets.m_wAuthorTime.SetTextFormat(format, hours);
		}
		else if (minutes > 0)
		{
			format = TIME_FORMAT_MINUTES;
			if (minutes == 1)
				format = TIME_FORMAT_MINUTE;
			m_InfoWidgets.m_wAuthorTime.SetTextFormat(format, minutes);
		}
		else if (seconds > 0)
		{
			format = TIME_FORMAT_SECONDS;
			if (seconds == 1)
				format = TIME_FORMAT_SECOND;
			m_InfoWidgets.m_wAuthorTime.SetTextFormat(format, seconds);
		}
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (playerController)
			playerController.SetPlatformImageToKind(details.m_ePlatform, m_InfoWidgets.m_wAuthorPlatformIcon, showOnPC: true, showOnXbox: true);
		
		m_InfoWidgets.m_wEntityName.SetText(details.m_sEntityName);
	}
	
	//------------------------------------------------------------------------------------------------	
	void UpdateName(string name)
	{
		m_InfoWidgets.m_wAuthorName.SetText(name);
	}
}