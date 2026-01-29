// Class for unifiying simple details widget behavior 
// Widget is displaying label, icons and additional data. Also it's appearance can be change on run to some extend.
//------------------------------------------------------------------------------------------------

class SCR_ServerDetailEntryComponent : SCR_WLibComponentBase
{	
	const string WIDGET_LABEL = "Content";
	const string WIDGET_ICON = "Icon";
	const string WIDGET_DATA = "Data";
	
	const string WIDGET_BACKGROUND = "Background"; 
	
	[Attribute("true")]
	protected bool m_bVisibleBackground;
	
	[Attribute("true")]
	protected bool m_bVisibleIcon; 
	
	[Attribute("true")]
	protected bool m_bVisibleData;  
	
	[Attribute()]
	protected string m_sLabel;
	
	[Attribute(UIConstants.ICONS_IMAGE_SET, UIWidgets.ResourceNamePicker)]
	protected ResourceName m_ImageSetDefault;
	
	[Attribute()]
	protected string m_sImage;
	
	protected TextWidget m_wTxtLabel;
	protected ImageWidget m_wImgIcon;
	protected TextWidget m_wTxtData;
	
	protected ImageWidget m_wImgBackground;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		// Find widget elements 
		m_wTxtLabel = TextWidget.Cast(w.FindAnyWidget(WIDGET_LABEL));
		m_wImgIcon = ImageWidget.Cast(w.FindAnyWidget(WIDGET_ICON));
		m_wTxtData = TextWidget.Cast(w.FindAnyWidget(WIDGET_DATA));
		
		m_wImgBackground =  ImageWidget.Cast(w.FindAnyWidget(WIDGET_BACKGROUND));
		
		// Visual setup 
		VisualSetup();
		SetIconFromImageSet(m_sImage, m_ImageSetDefault);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void VisualSetup()
	{
		// Background 
		if (m_wImgBackground)
		{
			m_wImgBackground.SetVisible(m_bVisibleBackground);
		}
		
		// Icon 
		if (m_wImgIcon)
		{
			m_wImgIcon.SetVisible(m_bVisibleIcon);
		}
		
		// Data 
		if (m_wTxtData)
		{
			m_wTxtData.SetVisible(m_bVisibleData);
		}
		
		// Label 
		if (m_wTxtLabel && !m_sLabel.IsEmpty())
		{
			m_wTxtLabel.SetText(m_sLabel);
		}
	}
	
	// User API
	//------------------------------------------------------------------------------------------------
	void SetLabelText(string text) 
	{
		if (m_wTxtLabel)
			m_wTxtLabel.SetText(text);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIconFromImageSet(string text, ResourceName imageSet = string.Empty) 
	{
		if (!imageSet)
			imageSet = m_ImageSetDefault;
		
		if (!imageSet)
			return;
		
		m_sImage = text;
		
		if (m_wImgIcon)
			m_wImgIcon.LoadImageFromSet(0, imageSet, m_sImage);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDataText(string text) 
	{
		if (m_wTxtData)
			m_wTxtData.SetText(text);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetElementsVisible(bool label, bool icon, bool data)
	{
		if (m_wTxtLabel)
			m_wTxtLabel.SetVisible(label);
		
		if (m_wImgIcon)
			m_wImgIcon.SetVisible(icon);
		
		if (m_wTxtData)
			m_wTxtData.SetVisible(data);
			
	}
	
	//------------------------------------------------------------------------------------------------
	void SetBackroundVisible(bool visible)
	{
		if (m_wImgBackground)
		{
			m_wImgBackground.SetVisible(visible);
		}
	}	
};