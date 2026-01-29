//------------------------------------------------------------------------------------------------
//! Base nametag element for text
[BaseContainerProps(), SCR_NameTagElementTitle()]
class SCR_NTTextBase : SCR_NTElementBase
{			
	[Attribute("14", UIWidgets.CheckBox, "Scaled text size at the beginning of the zone \n pixels")]
	protected int m_fTextSizeMax;
	
	[Attribute("14", UIWidgets.CheckBox, "Scaled text size at the end of the zone \n pixels")]
	protected int m_fTextSizeMin;
	
	[Attribute("{E2CBA6F76AAA42AF}UI/Fonts/Roboto/Roboto_Regular.fnt", UIWidgets.ResourceNamePicker, desc: "Font type", params: "fnt")]
	protected ResourceName m_FontResource;
		
	//------------------------------------------------------------------------------------------------
	//!  Get text from this element
	//! \param data is nametag struct
	//! \param[out] name Name or formatting of name
	//! \param[out] names If uses formating: Firstname, Alias and Surname (Alias can be an empty string)
	protected void GetText(SCR_NameTagData data, out string name, out notnull array<string> nameParams)
	{				
		return;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set text for this element
	//! \param data is nametag struct
	protected void SetText(SCR_NameTagData data, string text, notnull array<string> nameParams, int index)
	{
		if (!data.m_aNametagElements[index])
			return;
		
		int paramsCount = nameParams.Count();
		
		if (paramsCount == 0)
			TextWidget.Cast( data.m_aNametagElements[index] ).SetText(text);
		else if (paramsCount == 3)
			TextWidget.Cast( data.m_aNametagElements[index] ).SetTextFormat(text, nameParams[0], nameParams[1], nameParams[2]);
		else 
		{
			Print(string.Format("'SCR_NTTextBase' does not support nameParams of count '%1'!", paramsCount), LogLevel.ERROR);
			TextWidget.Cast( data.m_aNametagElements[index] ).SetText(text);
		}
			
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void ScaleElement(SCR_NameTagData data, int index)
	{
		Widget widget = data.m_aNametagElements[index];
		if (!widget)
			return;
		
		//At least one zone always need to be defined
		SCR_NameTagZone zone = SCR_NameTagDisplay.GetNametagZones().Get(data.m_iZoneID);
		if (!zone)
			return;
		
		float diff = m_fTextSizeMax - m_fTextSizeMin;	
		
		// avoid 0 as a starting point
		int zoneStart = zone.GetZoneStart();
		if ( zoneStart < 1 )
			zoneStart = 1;
	
		float lerp = Math.InverseLerp(zoneStart, zone.m_iZoneEnd, data.m_fDistance);
		diff = diff * lerp;
		
		float x = FrameSlot.GetSizeX(widget);
		float y = FrameSlot.GetSizeY(widget);
		float ratio = x / y;
		
		float scaledY = m_fTextSizeMax - diff;
		
		FrameSlot.SetSizeY(widget, scaledY);
		FrameSlot.SetSizeX(widget, scaledY * ratio);
	}
	
	//------------------------------------------------------------------------------------------------	
	override void SetDefaults(SCR_NameTagData data, int index)
	{
		TextWidget tWidget = TextWidget.Cast( data.m_aNametagElements[index] );
		if (!tWidget)
			return;
		
		SCR_NTStateText stateConf = SCR_NTStateText.Cast( GetEntityStateConfig(data) );
		if (!stateConf)
			return;
		
		tWidget.SetFont(m_FontResource);
		
		if (!m_bScaleElement)
			tWidget.SetExactFontSize(m_fTextSizeMax);
		
		tWidget.SetColor(stateConf.m_vColor);
		tWidget.SetShadow( stateConf.m_fShadowSize, stateConf.m_vShadowColor.PackToInt(), stateConf.m_fShadowOpacity, 0, 0);
		
		data.SetVisibility(tWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition); // transitions		
	}
};

//------------------------------------------------------------------------------------------------
//! Nametag element for name text
[BaseContainerProps(), SCR_NameTagElementTitle()]
class SCR_NTName : SCR_NTTextBase
{		
	//------------------------------------------------------------------------------------------------
 	override void GetText(SCR_NameTagData data, out string name, out notnull array<string> nameParams)
	{	
		data.UpdateEntityType();
		data.GetName(name, nameParams);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{
		super.UpdateElement(data, index);
		
		if (data.m_Flags & ENameTagFlags.NAME_UPDATE || data.m_Flags & ENameTagFlags.ENT_TYPE_UPDATE)
		{
			string name;
			array<string> nameParams = {};
			
			GetText(data, name, nameParams);
			
			if (name.IsEmpty())
				SetText(data, "GETNAME_ERROR", nameParams, index);
			else
			{
				SetText(data, name, nameParams, index);
				data.m_Flags &= ~ENameTagFlags.NAME_UPDATE;
				data.m_Flags &= ~ENameTagFlags.ENT_TYPE_UPDATE;
			}
		}
	}
};

//------------------------------------------------------------------------------------------------
//! Nametag element for distance debug text
[BaseContainerProps(), SCR_NameTagElementTitle()]
class SCR_NTTextDebug : SCR_NTTextBase
{	
	//------------------------------------------------------------------------------------------------
 	override void GetText(SCR_NameTagData data, out string name, out notnull array<string> nameParams)
	{
		name = Math.Round(Math.Sqrt(data.m_fDistance)).ToString();
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{		
		super.UpdateElement(data, index);
		
		string name;
		array<string> nameParams = {};
			
		GetText(data, name, nameParams);
		SetText(data, name, nameParams, index);	
	}
};