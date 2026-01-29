//------------------------------------------------------------------------------------------------
class NewsDialogUI: DialogUI
{
	string m_sWidgetAuthor = "Author";
	string m_sWidgetDate = "Date";
	string m_sWidgetImage = "Image";
	TextWidget m_wDate;
	TextWidget m_wAuthor;
	ImageWidget m_wImage;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

	}

	//------------------------------------------------------------------------------------------------
	void SetAuthor(string text)
	{
		TextWidget w = TextWidget.Cast(GetRootWidget().FindAnyWidget(m_sWidgetAuthor));
		if (!w)
			return;

		w.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	void SetDate(string text)
	{
		TextWidget w = TextWidget.Cast(GetRootWidget().FindAnyWidget(m_sWidgetDate));
		if (!w)
			return;

		w.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	void SetImage(string texture)
	{
		ImageWidget w = ImageWidget.Cast(GetRootWidget().FindAnyWidget(m_sWidgetImage));
		if (!w)
			return;

		w.LoadImageTexture(0, texture);
	}

	//------------------------------------------------------------------------------------------------
	string GetAuthor()
	{
		TextWidget w = TextWidget.Cast(GetRootWidget().FindAnyWidget(m_sWidgetAuthor));
		if (!w)
			return string.Empty;
			
		return w.GetText();
	}

	//------------------------------------------------------------------------------------------------
	string GetDate()
	{
		TextWidget w = TextWidget.Cast(GetRootWidget().FindAnyWidget(m_sWidgetDate));
		if (!w)
			return string.Empty;

		return w.GetText();
	}
};



