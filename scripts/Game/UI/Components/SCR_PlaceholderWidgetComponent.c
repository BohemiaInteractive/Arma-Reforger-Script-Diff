//! Meant to be used on text wdgets which have localization disabled, as it will fill them with localized text if they are empty
//! For example when we are waiting for some network data
//! Use with caution as this doesn't not handle updating of the string when language is changed, thus this should be used only on elements which will be rebuilt when language is changed
class SCR_PlaceholderWidgetComponent : ScriptedWidgetComponent
{
	[Attribute(defvalue: "", uiwidget: UIWidgets.EditBoxMultiline, desc: "Localized text which is going to be set only if the TextWidget to which this is attached doesn't already have any text set")]
	protected LocalizedString m_sPlaceholderText;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		TextWidget text = TextWidget.Cast(w);
		if (!text)
			return;

		if (text.GetText().IsEmpty())
			text.SetText(WidgetManager.Translate(m_sPlaceholderText)); // manually translate to compensate for disabled localization on given widget
	}
}