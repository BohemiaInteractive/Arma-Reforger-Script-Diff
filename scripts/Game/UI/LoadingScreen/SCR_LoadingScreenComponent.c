class SCR_LoadingScreenComponent : SCR_BaseLoadingScreenComponent
{
	protected bool m_bMissionDataRetrieved;
	
	protected const string AUTHOR_FORMAT = "#AR-AuthorLoadingScreen";		//"Author: "
	protected const string AUTHOR_UNKNOWN = "#AR-Author_Unknown";			//"By a community author"
	
	protected const float MAX_DESCRIPTION_LENGTH = 1000;
	protected const float FADE_TIME = 1;
	
	private float m_fLoadingTime_MissionDataRetrieved;
	private float m_fProgressBar_Progress;
	private ref SCR_LoadingHintComponent m_LoadingHintComponent;
	
	protected Widget m_wBuildVersion;
	
	//---------------------------------------------------------------------------------------------
	override protected void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wRoot.SetZOrder(10000);
	}

	//---------------------------------------------------------------------------------------------
	override protected void InitWidgets()
	{
		super.InitWidgets();
		
		m_Widgets.m_wContentOverlay.SetOpacity(0);
		
		m_Widgets.m_wContent.SetVisible(false);
		m_Widgets.m_wPreloadContent.SetVisible(true);
		
		m_Widgets.m_wContentOverlay.SetVisible(true);
		m_Widgets.m_wBlackOverlay.SetVisible(true);
		
		#ifdef DEBUG_LOADING_SCREENS
		m_Widgets.m_wCrossplayWarning.SetVisible(true);
		m_Widgets.m_wModdedWarning.SetVisible(true);		
		#else
		m_Widgets.m_wCrossplayWarning.SetVisible(false);
		m_Widgets.m_wModdedWarning.SetVisible(false);		
		#endif
	}	

	//---------------------------------------------------------------------------------------------
	void OnMissionDataRetrieved(SCR_MissionHeader header)
	{
		#ifdef DEBUG_LOADING_SCREENS
		PrintFormat(">> %1 >> OnMissionDataRetrieved | header: %2", this, header);
		#endif		

		// Create a build version layout
		m_wBuildVersion = GetGame().GetWorkspace().CreateWidgets("{B7A765172F0BD4D9}UI/layouts/Common/GameVersionWatermark.layout", m_wRoot);
		m_wBuildVersion.SetZOrder(1);
		m_wBuildVersion.SetVisible(false);
		
		// If loading header failed, keep the preloading state
		if (!header)
			return;
				
		// Add hint component
		m_LoadingHintComponent = new SCR_LoadingHintComponent();
		
		if (m_LoadingHintComponent)
			m_Widgets.m_wHintText.AddHandler(m_LoadingHintComponent);	
		
		// Get description text and check its length
		string description = header.m_sDetails;
		if (description == string.Empty)
			description = header.m_sDescription;
		
		description = WidgetManager.Translate(description);
		
		if (description.Length() > MAX_DESCRIPTION_LENGTH)
		{
			description = description.Substring(0, MAX_DESCRIPTION_LENGTH);
			description += "...";
		}		
		
		string image = header.m_sLoadingScreen;
		if (image == string.Empty)
			image = header.m_sIcon;
	
		if (m_Widgets.m_wTitle)
			m_Widgets.m_wTitle.SetText(header.m_sName);
		
		if (m_Widgets.m_wDescription)
			m_Widgets.m_wDescription.SetText(description);
		
		if (m_Widgets.m_wAuthor)
		{
			if (header.m_sAuthor.IsEmpty())
				m_Widgets.m_wAuthor.SetText(AUTHOR_UNKNOWN);
			else
			{
				string author = WidgetManager.Translate(AUTHOR_FORMAT);
				m_Widgets.m_wAuthor.SetText(author + header.m_sAuthor);
			}
		}
		
		if (m_Widgets.m_wLoadingImage)
		{
			m_Widgets.m_wLoadingImage.LoadImageTexture(0, image);
		}
		
		// Allow fade-in Update() animations
		m_bMissionDataRetrieved = true;
		m_fLoadingTime_MissionDataRetrieved = m_fLoadingTime;
	}	

	//---------------------------------------------------------------------------------------------
	override void OnHideInternal()
	{
		if (m_bMissionDataRetrieved)
		{
			ResetLoadingTime();
		}
		else
		{
			if (m_LoadingHintComponent)
				m_LoadingHintComponent.OnLoadingFinished();
			
			SaveLoadingTime(m_fLoadingTime);
		}
	}	
			
	//---------------------------------------------------------------------------------------------
	override void Update(float timeSlice, float progress = 0, float minDurationRatio = 0)
	{
		super.Update(timeSlice, progress, minDurationRatio);
		
		#ifdef DEBUG_LOADING_SCREENS
		PrintFormat(">> %1 >> Update | timeSlice: %2 | progress: %3 | m_fLoadingTime: %4", this, timeSlice, progress, m_fLoadingTime);
		#endif		
		
		if (!m_bMissionDataRetrieved)
			return;

		// Update loading hints
		if (m_LoadingHintComponent)
			m_LoadingHintComponent.Update(timeSlice);
		
		// Update progress bar
		SetProgressBar(progress);
						
		// Rotate spinner
		if (m_SpinnerComp)
			m_SpinnerComp.Update(timeSlice);
		
		// Hiding pre-load conntent
		if (m_fLoadingTime <= m_fLoadingTime_MissionDataRetrieved + FADE_TIME)
		{
			Fade(m_Widgets.m_wContentOverlay, true, FADE_TIME, timeSlice);
		}
		// Showing scenario content
		else if (m_fLoadingTime <= m_fLoadingTime_MissionDataRetrieved + 2 * FADE_TIME)
		{
			m_Widgets.m_wPreloadContent.SetVisible(false);
			m_Widgets.m_wContent.SetVisible(true);
			m_wBuildVersion.SetVisible(true);
		
			Fade(m_Widgets.m_wContentOverlay, false, FADE_TIME, timeSlice);
		}
		// Scenario content shown
		else
		{
			m_Widgets.m_wPreloadContent.SetVisible(false);
			m_Widgets.m_wContent.SetVisible(true);			
			m_Widgets.m_wContentOverlay.SetOpacity(0);
		}
	}
	
	//---------------------------------------------------------------------------------------------
	protected void SetProgressBar(float progress)
	{
		if (!m_Widgets.m_wProgressBarFill || !m_Widgets.m_wProgressBarSpace)
			return;
		
		// Make sure progression never goes back
		if (progress < m_fProgressBar_Progress)
			progress = m_fProgressBar_Progress;
		else if (progress > m_fProgressBar_Progress)
			m_fProgressBar_Progress = progress;
		
		HorizontalLayoutSlot.SetFillWeight(m_Widgets.m_wProgressBarFill, progress);
		HorizontalLayoutSlot.SetFillWeight(m_Widgets.m_wProgressBarSpace, 1 - progress);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetJoiningCrossPlay(bool isCrossPlay)
	{
		m_Widgets.m_wCrossplayWarning.SetVisible(isCrossPlay);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLoadingModded(bool isModded)
	{
		m_Widgets.m_wModdedWarning.SetVisible(isModded);
	}
};