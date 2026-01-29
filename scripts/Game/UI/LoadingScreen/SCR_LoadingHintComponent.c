//------------------------------------------------------------------------------------------------
class SCR_LoadingHintComponent : ScriptedWidgetComponent
{
	protected const int HINT_INDEX = -1;
	protected const float SWITCH_TIME = 10;
	
	protected int m_iHintCount;
	protected int m_iHintIndex = -1;
	protected float m_fTime;
	protected TextWidget m_wText;
	protected ResourceName m_Config = "{CB10921F1096D4A0}Configs/UI/LoadingScreenHints.conf";
	protected ref array<string> m_aAllHints;
	protected ref array<string> m_aReadHints;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wText = TextWidget.Cast(w);

		// Init arrays of all hints and read hints
		BaseContainer cont = GetGame().GetGameUserSettings().GetModule("SCR_LoadingHints");
		if (cont)
		{
			cont.Get("m_aReadHints", m_aReadHints);
			if (m_aReadHints)
				m_iHintCount = m_aReadHints.Count();
		}
		
		Resource resource = BaseContainerTools.LoadContainer(m_Config);
		
		if (!resource)
			return;
		
		BaseContainer entries = resource.GetResource().ToBaseContainer();
		
		if (!entries)
			return;
		
		entries.Get("m_aHints", m_aAllHints);

		// Show first hint
		ShowHint(HINT_INDEX);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnLoadingFinished()
	{
		// Save hints to the settings module
		UserSettings settings = GetGame().GetGameUserSettings();
		if (!settings)
			return;
		
		BaseContainer cont = settings.GetModule("SCR_LoadingHints");
		if (!cont || !m_aReadHints || m_aReadHints.Count() == m_iHintCount)
			return;
		
		cont.Set("m_aReadHints", m_aReadHints);
		GetGame().UserSettingsChanged();
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		// Mark the current hint read if shown for infinite time
		if (SWITCH_TIME < 0)
			MarkHintRead();
	}
	
	//------------------------------------------------------------------------------------------------
	void Update(float timeSlice)
	{
		m_fTime += timeSlice;
		
		if (m_fTime < SWITCH_TIME)
			return;
		
		// Switch to a new hint
		MarkHintRead();
		ShowHint(HINT_INDEX);
		m_fTime = 0;
	}

	//------------------------------------------------------------------------------------------------
	void ShowHint(int entryIndex = -1)
	{
		if (!m_wText || !m_aAllHints)
			return;
		
		if (!m_aReadHints)
			m_aReadHints = {};

		string hint;
		if (entryIndex == -1)
		{
			array<string> unread = {};
			int count = GetUnreadHints(unread);
			if (count == 0)
				return;
			
			int i = Math.RandomInt(0, count);
			hint = unread[i];
			m_iHintIndex = m_aAllHints.Find(hint);
		}
		else if (entryIndex < m_aAllHints.Count())
		{
			hint = m_aAllHints[entryIndex];
		}

		m_wText.SetTextFormat(hint);
	}

	//------------------------------------------------------------------------------------------------
	void MarkHintRead()
	{
		if (m_iHintIndex < 0 || m_iHintIndex >= m_aAllHints.Count())
			return;

		m_aReadHints.Insert(m_aAllHints[m_iHintIndex]);
		m_iHintIndex = -1;
	}

	// Get all hints which were not read yet
	//------------------------------------------------------------------------------------------------
	protected int GetUnreadHints(array<string> hints)
	{
		hints.Clear();
		
		foreach (string hint : m_aAllHints)
		{
			if (!m_aReadHints.Contains(hint))
				hints.Insert(hint);
		}

		// If there is no unread hint, reset the read array and start it again
		if (hints.Count() == 0)
			hints.InsertAll(m_aAllHints);
		
		return hints.Count();
	}
};
