//------------------------------------------------------------------------------------------------
class SCR_SelectionMenuEntryIconComponent : SCR_SelectionMenuEntryComponent
{
	[Attribute("Icon")]
	protected string m_sIcon;
	
	[Attribute("255 255 255 255")]
	protected ref Color m_cDefaultColor;
	
	[Attribute("255 255 255 255")]
	protected ref Color m_cAltColor;
	
	protected ImageWidget m_wIcon;
	
	protected ResourceName m_sTexture;
	protected string m_sImage;
	
	//------------------------------------------------------------------------------------------------
	// Override
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sIcon));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set entry holding data driving this visuals
	override void SetEntry(SCR_SelectionMenuEntry entry)
	{
		// Clear previous referenced entry
		if (m_Entry)
		{
			m_Entry.GetOnIconChange().Remove(OnEntryIconChange);
		}
		
		// Setup new entry
		super.SetEntry(entry);
		
		if (!entry)
			return;
		
		m_Entry.GetOnIconChange().Insert(OnEntryIconChange);
	}
	
	//------------------------------------------------------------------------------------------------
	// Custom
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void SetImage(ResourceName texture, string image)
	{
		m_sTexture = texture;
		m_sImage = image;
		
		SCR_WLibComponentBase.SetTexture(m_wIcon, m_sTexture, m_sImage);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLayoutSize(float size)
	{
		if (m_wSizeLayout)
			FrameSlot.SetSize(m_wSizeLayout, size, size);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIconColor(Color color = null)
	{
		if (!m_wIcon)
			return;
		
		if (color == null)
			m_wIcon.SetColor(m_cDefaultColor);
		else
			m_wIcon.SetColor(color);
	}
	
	//------------------------------------------------------------------------------------------------
	Color GetAltColor()
	{
		return Color.FromInt(m_cAltColor.PackToInt());
	}
	
	//------------------------------------------------------------------------------------------------
	// Callback
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected void OnEntryIconChange(SCR_SelectionMenuEntry entry, ResourceName texture, string image)
	{
		SetImage(texture, image);
	}
}