class SCR_MapJournalUIButton : SCR_ScriptedWidgetComponent
{	
	protected ref ScriptInvoker m_OnToggled;
	
	protected SCR_JournalEntry m_Entry;
	protected int m_iEntryId;
	
	protected ref SCR_JournalButtonWidgets m_Widgets = new SCR_JournalButtonWidgets();
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_Widgets.Init(w);
		
		if (m_Widgets.m_ButtonComponent0)
			m_Widgets.m_ButtonComponent0.m_OnToggled.Insert(OnToggled);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entry
	//! \param[in] id
	void SetEntry(SCR_JournalEntry entry, int id)
	{
		m_Entry = entry;
		m_iEntryId = id;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] target
	void ShowEntry(Widget target)
	{	
		m_Entry.SetEntryLayoutTo(target);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetId()
	{
		return m_iEntryId;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return SCR_JournalEntry entry
	SCR_JournalEntry GetEntry()
	{
		return m_Entry;
	}

	//------------------------------------------------------------------------------------------------
	void SetContent(string text)
	{
		TextWidget content = TextWidget.Cast(m_Widgets.m_wContent);
		if (content)
		{
			content.SetText(text);
			return;
		}

		ImageWidget img = ImageWidget.Cast(m_Widgets.m_wContent);
		if (img)
		{
			img.LoadImageTexture(0, text);
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetToggled(bool newToggled)
	{
		m_Widgets.m_ButtonComponent0.SetToggled(newToggled, false);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnToggled()
	{
		if (!m_OnToggled)
			m_OnToggled = new ScriptInvoker();
		
		return m_OnToggled;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnToggled(SCR_ModularButtonComponent comp, bool newToggled)
	{
		if (m_OnToggled)
			m_OnToggled.Invoke(this);
	}
}