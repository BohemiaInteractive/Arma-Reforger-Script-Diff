//------------------------------------------------------------------------------------------------
class SCR_CareerUI: Managed
{	
	//------------------------------------------------------------------------------------------------
	private void SCR_CareerUI()
	{}
	
	//! Create a stat entry using the m_StatsLayout layout
	//------------------------------------------------------------------------------------------------
	static Widget CreateStatEntry(Widget container, ResourceName statsLayout, string text = "", string localizedValue = "", string value1 = "", string value2 = "", string value3 = "")
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return null;
		
		Widget StatEntry = workspace.CreateWidgets(statsLayout, container);
		if (!StatEntry)
			return null;
		
		RichTextWidget statName, statValue;
		
		statName = RichTextWidget.Cast(StatEntry.FindAnyWidget("StatName"));
		statValue = RichTextWidget.Cast(StatEntry.FindAnyWidget("StatValue"));
		if (!statName || !statValue)
			return null;
		
		statName.SetText(text);
		statValue.SetTextFormat(localizedValue, value1, value2, value3);
		
		return StatEntry;
	}
	
	//! Create a stat entry using the m_ProgresionStatsLayout layout
	//------------------------------------------------------------------------------------------------
	static Widget CreateProgressionStatEntry(Widget container, ResourceName statsProgressionLayout, string text = "", float progressionMinus = 0, float progression = 0, string localizedValue = "", string value1 = "", string value2 = "", string value3 = "")
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return null;
		
		Widget StatProgressionEntry = workspace.CreateWidgets(statsProgressionLayout, container);
		if (!StatProgressionEntry)
			return null;
		
		RichTextWidget statName, statProgressionMinus, statProgression, statValue;
		
		statName = RichTextWidget.Cast(StatProgressionEntry.FindAnyWidget("StatName"));
		statProgressionMinus = RichTextWidget.Cast(StatProgressionEntry.FindAnyWidget("StatProgressionMinus"));
		statProgression = RichTextWidget.Cast(StatProgressionEntry.FindAnyWidget("StatProgression"));
		statValue = RichTextWidget.Cast(StatProgressionEntry.FindAnyWidget("StatValue"));
		if (!statName || !statProgressionMinus || !statProgression || !statValue)
			return null;
		
		statName.SetText(text);
		
		if (progressionMinus != 0)
			statProgressionMinus.SetText("-"+progressionMinus+" | ");
		else
			statProgressionMinus.SetText("");
		
		if (progression != 0)
			statProgression.SetText("+"+ progression);
		else
			statProgression.SetText("");
		
		statValue.SetTextFormat(localizedValue, value1, value2, value3);
		
		return StatProgressionEntry;
	}
	
	//! Create a Header of stats (A stat entry with different visuals meant for categorizing stats below this one) using the m_HeaderStatsLayout layout
	//------------------------------------------------------------------------------------------------
	static Widget CreateHeaderStatEntry(Widget container, ResourceName headerStatsLayout, string text = "")
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return null;
		
		Widget HeaderEntry = workspace.CreateWidgets(headerStatsLayout, container);
		if (!HeaderEntry)
			return null;
		
		RichTextWidget textWidget;
		
		textWidget = RichTextWidget.Cast(HeaderEntry.FindAnyWidget("HeaderStatText"));
		if (!textWidget)
			return null;
		
		textWidget.SetText(text);
		
		return HeaderEntry;
	}	
	
	//! Update an already created stat widget
	//------------------------------------------------------------------------------------------------
	static void UpdateStatEntry(Widget StatEntry, string name, string localizedValue, string value1 = "", string value2 = "", string value3 = "")
	{
		if (!StatEntry)
			return;
		
		RichTextWidget statName, statValue;
		
		statName = RichTextWidget.Cast(StatEntry.FindAnyWidget("StatName"));
		statValue = RichTextWidget.Cast(StatEntry.FindAnyWidget("StatValue"));
		if (!statName || !statValue)
			return;
		
		statName.SetText(name);
		statValue.SetTextFormat(localizedValue, value1, value2, value3);
	}
	
	//! Update an already created progressionStat widget
	//------------------------------------------------------------------------------------------------
	static void UpdateStatProgressionEntry(Widget StatProgressionEntry, string name, float progressionMinus, float progression, string localizedValue, string value1 = "", string value2 = "", string value3 = "")
	{
		if (!StatProgressionEntry)
			return;
		
		RichTextWidget statName, statProgressionMinus, statProgression, statValue;
		
		statName = RichTextWidget.Cast(StatProgressionEntry.FindAnyWidget("StatName"));
		statProgressionMinus = RichTextWidget.Cast(StatProgressionEntry.FindAnyWidget("StatProgressionMinus"));
		statProgression = RichTextWidget.Cast(StatProgressionEntry.FindAnyWidget("StatProgression"));
		statValue = RichTextWidget.Cast(StatProgressionEntry.FindAnyWidget("StatValue"));
		if (!statName || !statProgressionMinus || !statProgression || !statValue)
			return;
		
		statName.SetText(name);
		
		if (progressionMinus != 0)
			statProgressionMinus.SetText(" | -"+progressionMinus);
		else
			statProgressionMinus.SetText("");
		
		if (progression != 0)
			statProgression.SetText("+"+ progression);
		else
			statProgression.SetText("");
		
		statValue.SetTextFormat(localizedValue, value1, value2, value3);
	}
};