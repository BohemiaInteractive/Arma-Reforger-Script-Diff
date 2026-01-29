//------------------------------------------------------------------------------------------------
class CareerMenuUI: ChimeraMenuBase
{	
	// Widget names 
	protected const string WIDGET_NAME_EDIT = "editPlayerName";
	protected const string WIDGET_LOADOUT_STATS = "hLoadoutPanel";
	protected const string WIDGET_CHARACTER_PREVIEW = "CharacterPreview"; 
	
	protected const string STRING_UNKNOWN_STAT = "Unknown";
	
	// Resources 
	const ResourceName ENTRY_LAYOUT = "{C87F7F6734B61688}UI/layouts/Menus/Career/CareerEntry.layout";
	
	// Components  
	protected ref SCR_EditBoxComponent m_EditPlayerName;
	protected ref SCR_LoadoutPreviewComponent m_LoadoutPreview;
	protected SCR_LoadoutManager m_LoadoutManager;
	protected ref SCR_LoadoutStatisticsComponent m_LoadoutStatistics;
	
	// Widgets 
	protected Widget m_wEditPlayerName;
	
	//private EditBoxWidget m_ProfileEditbox;
	//private Widget m_ProfileData;
	protected ref array<string> m_aBackendValues = new array<string>(); 
	
	// Values
	static CareerMenuUI m_sInstance;
	ref CareerBackendData m_BackendData;
	protected ref BackendCallback m_Callback = new BackendCallback();

	[MenuBindAttribute()]
	ButtonWidget Back;

	//------------------------------------------------------------------------------------------------
	[MenuBindAttribute()]
	void Back()
	{
		if (IsFocused())
			Close();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateStatEntry(Widget wParent, string sName, string sLabel, string sValue)
	{
		Widget entry = GetGame().GetWorkspace().CreateWidgets(ENTRY_LAYOUT, wParent);
		entry.SetName(sName);
		
		// Setup label 
		const string sLabelName = "txtLabel";
		TextWidget wLabel = TextWidget.Cast(entry.FindAnyWidget(sLabelName));
		if(wLabel)
			wLabel.SetText(sLabel);
		
		ItemPreviewWidget preview;
	
		// set value 
		//SetStatValue(sName, sValue);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnChange(Widget w, bool finished)
	{
		if (!finished || !m_EditPlayerName || w != m_EditPlayerName.GetRootWidget())
			return false;
		
		// TODO: Future feature of custom name

		return false;
	}
	
	//------------------------------------------------------------------------------------------------\
	protected void OnPlayerNameConfirm(string value)
	{
		if (value.IsEmpty())
			return;
		
		string nameOld = SCR_Global.GetProfileName();
		//string nameNew = m_ProfileEditbox.GetText();
		string nameNew = m_EditPlayerName.GetValue();
		nameNew = nameNew.Trim();
		// TODO: Future feature of custom name
		m_EditPlayerName.SetValue(nameOld);
	}

	//------------------------------------------------------------------------------------------------
	void UpdateCareerData()
	{		
		if (!m_BackendData)
			return;
		Widget wClassList = GetGame().GetWorkspace().FindAnyWidget("vStatList");
		
		if (m_aBackendValues.Count() == 0)
		{
			// Prepare stats array
			typename stats = ECareerStatId;
			int count = stats.GetVariableCount();
			for (int i = 0; i < count; i++)
			{
				m_aBackendValues.Insert("");
			}
		}
		
		// General
		m_aBackendValues[ECareerStatId.KILLS] = m_BackendData.GetKills().ToString();
		m_aBackendValues[ECareerStatId.DEATHS] = m_BackendData.GetDeaths().ToString();
		m_aBackendValues[ECareerStatId.FRIENDLY_KILLS] = m_BackendData.GetFriendlyKills().ToString();
		
		// Playtime 
		m_aBackendValues[ECareerStatId.PLAYTIME_RIFFLEMAN] = STRING_UNKNOWN_STAT;
		m_aBackendValues[ECareerStatId.PLAYIME_SHARPSHOOTER] = STRING_UNKNOWN_STAT;
		
		// Weapons
		m_aBackendValues[ECareerStatId.HEADSHOTS] = STRING_UNKNOWN_STAT;
		
		if (m_LoadoutStatistics)
			m_LoadoutStatistics.UpdateStats(m_LoadoutStatistics.GetCurrentLoadoutId());
	}
	
	//------------------------------------------------------------------------------------------------
	array<string> GetBackendValues() { return m_aBackendValues; }
		
	//------------------------------------------------------------------------------------------------
	protected void TestingShow()
	{
		Widget wEntry = GetRootWidget().FindAnyWidget("StatEntry");
		
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		if (!m_BackendData && GetGame().GetBackendApi().IsAuthenticated())
		{
			m_BackendData = new CareerBackendData;
			BackendApi backendApi = GetGame().GetBackendApi();
			if (backendApi)
			{
				m_Callback.SetOnSuccess(UpdateCareerData);
				backendApi.PlayerRequest(EBackendRequest.EBREQ_GAME_CharacterGet, m_Callback, m_BackendData, 0);
			}
		}
		
		super.OnMenuUpdate(tDelta);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		// Disallow changing profile name on consoles
		#ifndef PLATFORM_WINDOWS
			if (m_wEditPlayerName)
				m_wEditPlayerName.SetEnabled(false);
		#endif
		
		// Player name edit
		m_wEditPlayerName = GetRootWidget().FindAnyWidget(WIDGET_NAME_EDIT);
		if (m_wEditPlayerName)
			m_EditPlayerName = SCR_EditBoxComponent.Cast(m_wEditPlayerName.FindHandler(SCR_EditBoxComponent));
		
		if (m_EditPlayerName)
		{
			// Listeners 
			m_EditPlayerName.m_OnConfirm.Insert(OnPlayerNameConfirm);
			// Setup text 
			string profileName = SCR_Global.GetProfileName();
			m_EditPlayerName.SetValue(profileName);
		}
		
		super.OnMenuShow();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		Widget w = GetRootWidget();
		m_sInstance = this;
		InputManager inputManager = GetGame().GetInputManager();
		inputManager.AddActionListener(UIConstants.MENU_ACTION_BACK, EActionTrigger.PRESSED, Back );
				
		// Preview character 
		Widget wCharacterPreview = GetRootWidget().FindAnyWidget(WIDGET_CHARACTER_PREVIEW);
		if (wCharacterPreview)
			m_LoadoutPreview = SCR_LoadoutPreviewComponent.Cast(wCharacterPreview.FindHandler(SCR_LoadoutPreviewComponent));

		//m_LoadoutManager = SCR_LoadoutManager.GetInstance();
		
		// Loadout stats 
		Widget wLoadoutStats = GetRootWidget().FindAnyWidget(WIDGET_LOADOUT_STATS);
		if (wLoadoutStats)
			m_LoadoutStatistics = SCR_LoadoutStatisticsComponent.Cast(wLoadoutStats.FindHandler(SCR_LoadoutStatisticsComponent));
			
		if (m_LoadoutStatistics)
		{
			m_LoadoutStatistics.SetCareerUI(this);
			m_LoadoutStatistics.m_OnLoadoutChange.Insert(OnLoadoutChange);
			OnLoadoutChange(m_LoadoutStatistics.GetCurrentLoadoutId());
		}
		
		super.OnMenuOpen();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		m_sInstance = null;
		
		super.OnMenuClose();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLoadoutChange(int id)
	{
		if (!m_LoadoutPreview || !m_LoadoutStatistics)
			return;
		
		//m_LoadoutPreview.SetPreviewedLoadout(m_LoadoutManager.m_aPlayerLoadouts[0]);
		LoadoutStatSet statSet = m_LoadoutStatistics.GetLodoutStatSets()[id];
		if (statSet)
			m_LoadoutPreview.SetPreviewedLoadout(statSet.GetLoadout());
	}
};

//------------------------------------------------------------------------------------------------
//! Ids to receive all
enum ECareerStatId
{
	// General
	KILLS,
	DEATHS,
	FRIENDLY_KILLS,
	
	// Play times 
	PLAYTIME_RIFFLEMAN,
	PLAYIME_SHARPSHOOTER,
	
	// Weapons 
	HEADSHOTS,
};

