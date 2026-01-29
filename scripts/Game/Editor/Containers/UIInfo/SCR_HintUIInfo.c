[BaseContainerProps(configRoot: true), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_HintUIInfo : SCR_BlockUIInfo
{
	[Attribute(SCR_Enum.GetDefault(EHint.UNDEFINED), UIWidgets.SearchComboBox, "When defined, the hint can be shown only certain amount of times (defined by Show Limit attribute).\nAfter that, calls to show the hint will be ignored.", enums: ParamEnumArray.FromEnum(EHint))]
	protected EHint m_Type;
	
	[Attribute("1", desc: "How many times can the hint be shown. Valid only if Type is defined!")]
	protected int m_iShowLimit;
	
	[Attribute()]
	protected int m_iPriority;
	
	[Attribute("0", desc: "For how long should the hint be shown.\nWhen 0, default duration from hint manager will be used.\nWhen -1, the hint will be shown until cleared manually.")]
	protected float m_fDuration;
	
	[Attribute(desc: "Widgets to be highlighted when this hint is displayed")]
	protected ref array<string> m_aHighlightWidgetsNames;
	
	[Attribute("0", desc: "Should the hint duration be animated.\nWhen false, the duration will not be visualised.")]
	protected bool m_bIsTimerVisible;
	
	[Attribute(SCR_Enum.GetDefault(EFieldManualEntryId.NONE), UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EFieldManualEntryId))]
	protected EFieldManualEntryId m_FieldManualLink;
	
	[Attribute("0", desc: "True will make hint visible across menues. E.g. hint opened in inventory will stay will be reopened in hud once player leave inventory.")]
	protected bool m_bPersistent;
	
	[Attribute("0", desc: "Can this hint be set to don't show again?")]
	protected bool m_bIsDontShowAgainVisible;
	
	protected int m_iSequencePage;
	protected int m_iSequenceCount;
	protected WorldTimestamp m_iTimeHintStarted;
	
	//------------------------------------------------------------------------------------------------
	bool GetIsDontShowAgainVisible()
	{
		return m_bIsDontShowAgainVisible;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get hint type. When defined, the hint will be persistently saved and not shown again.
	//! \return hint type
	EHint GetType()
	{
		return m_Type;
	}

	//------------------------------------------------------------------------------------------------
	//! Get how many times can the hint be shown.
	//! Valid only if Type is defined!
	//! \return display limit
	int GetShowLimit()
	{
		return m_iShowLimit;
	}

	//------------------------------------------------------------------------------------------------
	//! Get if the animated timer should be visible.
	//! \return true if the timer is visible, false otherwise
	bool IsTimerVisible()
	{
		return m_bIsTimerVisible;
	}

	//------------------------------------------------------------------------------------------------
	//! Get priority of the hint. When another hint is already shown, this one will replace it only if it has the same or higher priority.
	//! \return priority
	int GetPriority()
	{
		return m_iPriority;
	}

	//------------------------------------------------------------------------------------------------
	//! Get duration for which the hint should be displayed (in seconds)
	//! \return duration
	float GetDuration()
	{
		return m_fDuration;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the time of Hint start, needed for proper Map implementation. Returns Replication.Time() of hint creation;
	//! \return
	WorldTimestamp GetTimeStarted()
	{
		return m_iTimeHintStarted;
	}

	//------------------------------------------------------------------------------------------------
	//! Set the time stamp of Hint start. Needed for hint timer timekeeping.
	void SetTimeStamp()
	{
		ChimeraWorld world = GetGame().GetWorld();
		m_iTimeHintStarted = world.GetServerTimestamp();
	}

	//------------------------------------------------------------------------------------------------
	//! Get names of widgets that should be highlighted.
	//! \param[out] Array to be filled with widget names
	//! \return number of names
	int GetHighlightWidgetNames(out notnull array<string> outWidgetNames)
	{
		if (m_aHighlightWidgetsNames)
			return outWidgetNames.Copy(m_aHighlightWidgetsNames);
		else
			return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get link to associated field manual entry.
	//! \return Field manual entry ID
	EFieldManualEntryId GetFieldManualLink()
	{
		return m_FieldManualLink;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initialize hint sequence
	//! \param page Page number of this hint
	//! \param count Total number of hints in the sequence
	void InitSequence(int page, int count)
	{
		m_iSequencePage = page;
		m_iSequenceCount = count;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Sequence page number of this hint
	int GetSequencePage()
	{
		return m_iSequencePage;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Total number of hints in the sequence
	int GetSequenceCount()
	{
		return m_iSequenceCount;
	}

	//------------------------------------------------------------------------------------------------
	//! \return True whent he hint is a part of a sequence.
	bool IsInSequence()
	{
		return m_iSequenceCount != 0;
	}

	//------------------------------------------------------------------------------------------------
	bool IsPersistent()
	{
		return m_bPersistent;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPersistent(bool persistent)
	{
		m_bPersistent = persistent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	//! \param description
	//! \param name
	//! \param duration
	//! \param type
	//! \param fieldManualEntry
	//! \param isTimerVisible
	//! \return the created SCR_HintUIInfo from the provided information
	static SCR_HintUIInfo CreateInfo(string description, string name, float duration, EHint type, EFieldManualEntryId fieldManualEntry, bool isTimerVisible)
	{
		SCR_HintUIInfo info = new SCR_HintUIInfo();
		info.Name = name;
		info.Description = description;
		info.m_fDuration = duration;
		info.m_Type = type;
		info.m_FieldManualLink = fieldManualEntry;
		info.m_bIsTimerVisible = isTimerVisible;
		return info;
	}

	//------------------------------------------------------------------------------------------------
	override bool SetDescriptionTo(TextWidget textWidget)
	{
		if (!textWidget)
			return false;
		
		Color sRGBA = Color.FromInt(UIColors.CONTRAST_COLOR.PackToInt());
		
		if (m_aHighlightWidgetsNames && !m_aHighlightWidgetsNames.IsEmpty())
			textWidget.SetTextFormat(GetDescription(), string.Format("color rgba=%1", UIColors.FormatColor(sRGBA)), "/color"); //--- ToDo: Don't hardcode
		else
			textWidget.SetText(GetDescription());
		return true;
	}
}
