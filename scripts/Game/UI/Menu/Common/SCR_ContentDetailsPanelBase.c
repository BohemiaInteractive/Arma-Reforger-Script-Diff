class SCR_ContentDetailsPanelBase : SCR_ScriptedWidgetComponent
{
	protected ref SCR_ContentDetailsPanelBaseWidgets m_CommonWidgets = new SCR_ContentDetailsPanelBaseWidgets;
	
	[Attribute()]
	protected ref DetailsPanelContentPresetConfig m_FallbackContent;
	
	[Attribute("500")]
	protected int m_iMaxDescriptionLenght;
	
	[Attribute("0")]
	protected bool m_bDisplayAdditionalInfo;
	
	[Attribute("{8D067F8167DB936D}UI/layouts/Menus/Common/DetailsPanel/Prefabs/AddonTypeImage.layout")]
	protected ResourceName m_sTypeDisplayLayout;
	
	[Attribute("12")]
	protected int m_iMaxTypeImages;
	
	protected const int DESCRIPTION_LENGHT_OFFSET = 10;
	
	protected SCR_SimpleWarningOverlayComponent m_WarningOverlay;
	protected ref array<Widget> m_aTypeImages = {};
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_CommonWidgets.Init(w.FindWidget("ContentDetailsPanel")); // The layout is exported from base layout which is embedded into this layout.
	
		m_CommonWidgets.m_WarningOverlayComponent.SetWarningVisible(false, false);
		m_CommonWidgets.m_wAdditionalInfo.SetVisible(m_bDisplayAdditionalInfo);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnUpdate(Widget w)
	{
		if (SCR_Global.IsEditMode())
			return super.OnUpdate(w);
		
		// This will also get called on updates of children, ignore them
		if (w != m_wRoot)
			return false;
		
		GetGame().GetCallqueue().Call(UpdateSize);
		
		return super.OnUpdate(w);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateSize()
	{
		// Resize the height of the picture, it must keep a fixed aspect ratio
		float sizex, sizey;
		m_wRoot.GetScreenSize(sizex, sizey);
		float sizexUnscaled = GetGame().GetWorkspace().DPIUnscale(sizex);
		m_CommonWidgets.m_wTopSize.EnableHeightOverride(true);
		m_CommonWidgets.m_wTopSize.SetHeightOverride(sizexUnscaled / SCR_WorkshopUiCommon.IMAGE_SIZE_RATIO);
	}
	
	//-----------------------------------------------------------------------------------
	//! Get fallback content by tag 
	protected DetailsPanelContentPreset FallbackContentByTag(string contentTag)
	{	
		foreach (DetailsPanelContentPreset content : m_FallbackContent.m_aContent)
		{
			if (content.m_sTag == contentTag)
				return content;
		}
		
		return m_FallbackContent.m_DefaultContent;
	}
	
	//-----------------------------------------------------------------------------------
	//! Set description text with enforced text lenght limit
	protected void SetDescriptionText(string text)
	{
		// Edit text if it's too long
		if (text.Length() > m_iMaxDescriptionLenght + DESCRIPTION_LENGHT_OFFSET)
		{
			text = text.Substring(0, m_iMaxDescriptionLenght);
			text += "...";
		}
		
		m_CommonWidgets.m_wDescriptionText.SetText(text);
	}
	
	//-----------------------------------------------------------------------------------
	protected Widget AddTypeDisplay(string image, ResourceName imageset, ResourceName glowImageset)
	{
		if (m_aTypeImages.Count() >= m_iMaxTypeImages)
			return null;
		
		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sTypeDisplayLayout, m_CommonWidgets.m_wTypeImages);
		if (!w)
			return null;
		
		m_aTypeImages.Insert(w);
		
		SCR_DynamicIconComponent comp = SCR_DynamicIconComponent.Cast(w.FindHandler(SCR_DynamicIconComponent));
		if (comp)
			comp.SetImage(image, imageset, glowImageset);
		
		return w;
	}
	
	//-----------------------------------------------------------------------------------
	protected void ClearTypeDisplays()
	{
		foreach (Widget w : m_aTypeImages)
		{
			w.RemoveFromHierarchy();
		}
		
		m_aTypeImages.Clear();
	}
}

//-----------------------------------------------------------------------------------
[BaseContainerProps(configRoot : true)]
class DetailsPanelContentPresetConfig
{
	[Attribute()]
	ref DetailsPanelContentPreset m_DefaultContent;
	
	[Attribute()]
	ref array<ref DetailsPanelContentPreset> m_aContent;
}

//-----------------------------------------------------------------------------------
[BaseContainerProps(), BaseContainerCustomTitleField("m_sTag")]
class DetailsPanelContentPreset
{
	[Attribute()]
	string m_sTag;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Layout", "edds")]
	ResourceName m_sImage;
	
	[Attribute()]
	string m_sTitle;
	
	[Attribute()]
	string m_sTitleImageName;
	
	[Attribute(UIColors.GetColorAttribute(UIColors.NEUTRAL_INFORMATION))]
	ref Color m_sTitleImageColor;
	
	[Attribute()]
	string m_sDescription;
}