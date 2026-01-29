/*!
Component to color progress bar based on its value.
*/
class SCR_ProgressBarColorsUIComponent: ScriptedWidgetComponent
{
	[Attribute(desc: "List of colors and their max values.\nEntries should be in ascending order.\nWhen none of the entries is valid, widget's default color will be used.")]
	protected ref array<ref SCR_ProgressBarColorsEntry> m_aColors;
	
	protected ProgressBarWidget m_ProgressBar;
	protected ref Color m_DefaultColor;
	
	//override bool OnChange(Widget w, bool finished)
	override bool OnUpdate(Widget w)
	{
		if (!m_ProgressBar) return false;
		float value = m_ProgressBar.GetCurrent();
		
		foreach (SCR_ProgressBarColorsEntry entry: m_aColors)
		{
			if (value < entry.m_fMax)
			{
				m_ProgressBar.SetColor(entry.m_Color);
				return false;
			}
		}
		m_ProgressBar.SetColor(m_DefaultColor);
		return false;
	}
	override void HandlerAttached(Widget w)
	{
		m_DefaultColor = w.GetColor();
		m_ProgressBar = ProgressBarWidget.Cast(w);
		if (!m_ProgressBar)
		{
			Print("SCR_ProgressBarColorsUIComponent must be attached to a widget of type 'ProgressBarWidget'!", LogLevel.WARNING);
		}
	}
};
[BaseContainerProps()]
class SCR_ProgressBarColorsEntry
{
	[Attribute(desc: "Max value for this color to be applied.")]
	float m_fMax;
	
	[Attribute(defvalue: "1 1 1 1", desc: "Applied color.")]
	ref Color m_Color;
};