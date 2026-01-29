/*
Selsection menu entry widget component for displaying 3d rendered objects
*/

//------------------------------------------------------------------------------------------------
class SCR_SelectionMenuEntryPreviewComponent : SCR_SelectionMenuEntryComponent
{
	protected const float DEFAULT_FOV = 10;
	
	[Attribute("ItemPreview")]
	protected string m_sPreviewItem;
	
	[Attribute("ItemShadow", desc: "Reference to Item Shadow")]
	protected string m_sItemShadow;
	
	[Attribute("FallbackIcon")]
	protected string m_sFallbackIcon;
	
	[Attribute(UIColors.GetColorAttribute(UIColors.NEUTRAL_INFORMATION), UIWidgets.ColorPicker, desc: "Default color for Item Shadow")]
	protected ref Color m_DefaulItemColor;
	
	[Attribute(UIColors.GetColorAttribute(UIColors.NEUTRAL_INFORMATION), UIWidgets.ColorPicker, desc: "Medical color for Item Shadow")]
	protected ref Color m_MedicalItemColor;	
	
	
	protected ItemPreviewWidget m_wPreviewItem;
	protected ImageWidget m_wItemShadow;
	protected ImageWidget m_wFallbackIcon;	
	
	protected IEntity m_Item;
	protected PreviewRenderAttributes m_PreviewAttributes;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wPreviewItem = ItemPreviewWidget.Cast(m_wRoot.FindAnyWidget(m_sPreviewItem));
		m_wItemShadow = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sItemShadow));
		m_wFallbackIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sFallbackIcon));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Prepare preview item and render given entity 
	void SetPreviewItem(IEntity item)
	{
		if (!m_wPreviewItem || ! m_wFallbackIcon)
		{
			Print("[SCR_SelectionMenuEntryPreviewComponent] - missing widgets!", LogLevel.DEBUG);
			return;
		}
		
		// Visibility 
		m_wPreviewItem.SetVisible(item != null);
		m_wItemShadow.SetVisible(item != null);
		m_wFallbackIcon.SetVisible(!item);
		
		m_Item = item;
		
		if (!item)
			return;
		

		// Get manager and render preview 
		ChimeraWorld world = ChimeraWorld.CastFrom(item.GetWorld());
		if (!world)
			return;
		
		ItemPreviewManagerEntity manager = world.GetItemPreviewManager();
		if (!manager)
			return;
		
		// Set rendering and preview properties 
		SetupPreviewAttributes(m_PreviewAttributes);
		manager.SetPreviewItem(m_wPreviewItem, item, m_PreviewAttributes);
		m_wPreviewItem.SetResolutionScale(1, 1);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Find and setup preview attributes that should be used in previwe
	protected void SetupPreviewAttributes(out PreviewRenderAttributes preview)
	{
		if (!m_Item)
			return;
		
		// Inventory item cmp
		InventoryItemComponent invetoryItem = InventoryItemComponent.Cast(
			m_Item.FindComponent(InventoryItemComponent));
		
		if (!invetoryItem)
			return;
				
		preview = PreviewRenderAttributes.Cast(
			invetoryItem.FindAttribute(PreviewRenderAttributes));
		
		// Get custom radial attribute 
		SCR_SelectionMenuPreviewAttributes menuPreview = SCR_SelectionMenuPreviewAttributes.Cast(
			invetoryItem.FindAttribute(SCR_SelectionMenuPreviewAttributes));
		
		if (preview && menuPreview)
		{
			preview.ZoomCamera(10, menuPreview.m_fCustomFov, menuPreview.m_fCustomFov);
						
			// Change widget size	
			if (preview && m_wSizeLayout && menuPreview.m_fIconSizeXMultiplier != -1)
			{
				float size = m_fOriginalSize * menuPreview.m_fIconSizeXMultiplier;
				FrameSlot.SetSize(m_wSizeLayout, size, size);
				
				m_fAdjustedSize = size;
			}
			
			// Colorize Item shadow
			if (menuPreview.m_bShowMedicalColor == true)			
				m_wItemShadow.SetColor(m_MedicalItemColor);			
			else			
				m_wItemShadow.SetColor(m_DefaulItemColor);			
		}
		else
		{
			m_wItemShadow.SetColor(m_DefaulItemColor);
		}
	}
}