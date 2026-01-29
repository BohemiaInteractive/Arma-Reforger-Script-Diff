class SCR_SimpleMessageComponent : ScriptedWidgetComponent
{
	[Attribute()]
	protected string m_sImageWidgetName;
	
	[Attribute()]
	protected string m_sTitleWidgetName;
	
	[Attribute()]
	protected string m_sDescriptionWidgetName;
	
	[Attribute("Loading")]
	protected string m_sLoadingName;
	
	[Attribute()]
	protected string m_sDefaultPreset;
	
	[Attribute()]
	ref SCR_SimpleMessagePresets m_aContentPresets;
	
	protected ImageWidget m_wImage;
	protected TextWidget m_wTitle;
	protected TextWidget m_wDescription;
	protected Widget m_wLoading;
	
	protected Widget m_wRoot;
	
	//----------------------------------------------------------------------------------
	protected override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		
		if (!m_sImageWidgetName.IsEmpty())
			m_wImage = ImageWidget.Cast(w.FindAnyWidget(m_sImageWidgetName));
		
		if (!m_sTitleWidgetName.IsEmpty())
			m_wTitle = TextWidget.Cast(w.FindAnyWidget(m_sTitleWidgetName));
		
		if (!m_sDescriptionWidgetName.IsEmpty())
			m_wDescription = TextWidget.Cast(w.FindAnyWidget(m_sDescriptionWidgetName));
		
		if (!m_sLoadingName.IsEmpty())
			m_wLoading = w.FindAnyWidget(m_sLoadingName);
		
		SetContentFromPreset(m_sDefaultPreset);
	}
		
	//----------------------------------------------------------------------------------
	/*!
	Sets content from preset stored within this component
	\param presetTag id of preset to set
	*/
	void SetContentFromPreset(string presetTag)
	{
		if (!m_aContentPresets || presetTag.IsEmpty())
			return;
		
		SCR_SimpleMessagePreset preset = m_aContentPresets.GetPreset(presetTag);
		
		if (!preset)
			return;
		
		if (!preset.m_UIInfo)
			return;
		
		SetContentFromPreset(preset.m_UIInfo);
	}
	
	//----------------------------------------------------------------------------------
	//! Sets content from preset provided externally
	protected void SetContentFromPreset(SCR_SimpleMessageUIInfo uiInfo)
	{
		array<Widget> mainWidgets = {m_wImage, m_wTitle, m_wDescription};
		bool loading = uiInfo.GetDisplayLoading();
		foreach (Widget w : mainWidgets)
		{
			if (w)
				w.SetVisible(!loading);
		}
		
		if (m_wLoading)
			m_wLoading.SetVisible(loading);
		
		if (loading)
		{
			if (m_wLoading)
			{
				SCR_LoadingOverlay loadingComp = SCR_LoadingOverlay.Cast(m_wLoading.FindHandler(SCR_LoadingOverlay));
				if (loadingComp)
				{
					if (!uiInfo.GetName().IsEmpty())
						loadingComp.SetText(uiInfo.GetName());
					else if (!uiInfo.GetDescription().IsEmpty())
						loadingComp.SetText(uiInfo.GetDescription());
				}
			}
		}
		else
		{
			if (m_wImage)
			{
				if (uiInfo.HasIcon())
				{
					m_wImage.SetColor(uiInfo.GetImageColor());
					uiInfo.SetIconTo(m_wImage);
					m_wImage.SetVisible(true);
				}
				else
				{
					m_wImage.SetVisible(false);
				}
			}
			
			if (m_wTitle)
			{
				if (!uiInfo.GetName().IsEmpty())
				{
					uiInfo.SetNameTo(m_wTitle);
					m_wTitle.SetVisible(true);
				}
				else 
				{
					m_wTitle.SetVisible(false);
				}	
			}
			
			if (m_wDescription)
			{
				if (!uiInfo.GetName().IsEmpty())
				{
					uiInfo.SetDescriptionTo(m_wDescription);
					m_wDescription.SetVisible(true);
				}
				else 
				{
					m_wDescription.SetVisible(false);
				}	
			}
		}
	}
	
	//----------------------------------------------------------------------------------
	Widget GetRootWidget()
	{
		return m_wRoot;
	}
	
	//----------------------------------------------------------------------------------
	void SetVisible(bool visible)
	{
		if (m_wRoot)
		{
			m_wRoot.SetVisible(visible);
			m_wRoot.Update();
		}
	}
};

[BaseContainerProps(), BaseContainerCustomTitleField("m_sTag")]
class SCR_SimpleMessagePreset : Managed
{
	[Attribute(desc: "The ID of the Preset used to set the correct visual information")]
	string m_sTag;
	
	[Attribute()]
	ref SCR_SimpleMessageUIInfo m_UIInfo;
};

[BaseContainerProps(configRoot: true)]
class SCR_SimpleMessagePresets : Managed
{	
	[Attribute()]
	ref array<ref SCR_SimpleMessagePreset> m_aPresets;
	
	//----------------------------------------------------------------------------------
	SCR_SimpleMessagePreset GetPreset(string tag)
	{
		foreach (SCR_SimpleMessagePreset preset : m_aPresets)
		{
			if (preset.m_sTag == tag)
				return preset;
		}
		
		Print(string.Format("SCR_SimpleMessagePresets: preset not found: %1", tag), LogLevel.ERROR);
		return null;
	}
};