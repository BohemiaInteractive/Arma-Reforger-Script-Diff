//! Base component for widgets displaying content data 
// Base for Mods & Scenario entries 

//------------------------------------------------------------------------------------------------
class SCR_ContentEntryComponent : SCR_ButtonBaseComponent
{
	//const string WIDGET_ = "";
	const string WIDGET_IMAGE_THUMBNAIL = "ImgThumbnail";
	const string WIDGET_TEXT_LABEL = "TxtLabel";
	const string WIDGET_TEXT_DESCRIPTION = "TxtDescription"; 
	const string WIDGET_BTN_DOWNLOAD = "BtnDownload";
	
	// Widgets
	protected ImageWidget m_wImgThumbnail;  
	protected TextWidget m_wTxtLabel;
	protected TextWidget m_wTxtDescription;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		// Get widgets 
		m_wImgThumbnail = ImageWidget.Cast(w.FindAnyWidget(WIDGET_IMAGE_THUMBNAIL));
		m_wTxtLabel = TextWidget.Cast(w.FindAnyWidget(WIDGET_TEXT_LABEL));
		m_wTxtDescription = TextWidget.Cast(w.FindAnyWidget(WIDGET_TEXT_DESCRIPTION));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Just simple on line text setting
	protected void SetTextSafe(TextWidget txt, string str)
	{
		if (txt)
			txt.SetText(str);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLabelText(string str) { SetTextSafe(m_wTxtLabel, str); }
	
	//------------------------------------------------------------------------------------------------
	void SetDescriptionText(string str) { SetTextSafe(m_wTxtDescription, str); }
	
	//------------------------------------------------------------------------------------------------
	void SetThumbnail(ResourceName image) 
	{
		if (image.IsEmpty())
			return;
		
		if (m_wImgThumbnail)
		{
			m_wImgThumbnail.LoadImageTexture(0, image, false, true);
			int sx, sy;
			m_wImgThumbnail.GetImageSize(0, sx, sy);
			m_wImgThumbnail.SetSize(sx, sy);
		}	
	}
	
	
};