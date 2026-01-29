//------------------------------------------------------------------------------------------------
//! Base zone element widget for icons/images
[BaseContainerProps(), SCR_NameTagElementTitle()]
class SCR_NTIconBase : SCR_NTElementBase
{	
	[Attribute("{70E828A2F6EBE7D0}UI/Textures/Nametags/nametagicons.imageset", UIWidgets.ResourceNamePicker, desc: "Imageset selection", params: "imageset")]
	ResourceName m_sImageset;
	
	[Attribute("64", UIWidgets.CheckBox, "Scaled icon size at the beginning of the zone \n pix*pix")]
	protected float m_iImageSizeMax;
		
	[Attribute("64", UIWidgets.CheckBox, "Scaled icon size at the end of the zone \n pix*pix")]
	protected float m_iImageSizeMin;
		
	//------------------------------------------------------------------------------------------------
	override protected void ScaleElement(SCR_NameTagData data, int index)
	{
		ImageWidget widget = ImageWidget.Cast( data.m_aNametagElements[index] );
		if (!widget)
			return;
		
		//At least one zone always need to be defined
		SCR_NameTagZone zone = SCR_NameTagDisplay.GetNametagZones().Get(data.m_iZoneID);
		if (!zone)
			return;
		
		float diff = m_iImageSizeMax - m_iImageSizeMin;
		
		// avoid 0 as a starting point
		int zoneStart = zone.GetZoneStart();
		if ( zoneStart < 1 )
			zoneStart = 1;
		
		float lerp = Math.InverseLerp(zoneStart, zone.m_iZoneEnd, data.m_fDistance);
		diff = diff * lerp;
		// final size
		diff = ( m_iImageSizeMax - diff );
		
		widget.SetSize(diff, diff);
	}
	
	//------------------------------------------------------------------------------------------------	
	override void SetDefaults(SCR_NameTagData data, int index)
	{
		ImageWidget iWidget = ImageWidget.Cast( data.m_aNametagElements[index] );
		if (!iWidget)
			return;
		
		SCR_NTStateIcon stateConf = SCR_NTStateIcon.Cast( GetEntityStateConfig(data) );
		if (!stateConf)
			return;
		
		if (!m_sImageset.IsEmpty() && !stateConf.m_sImagesetQuad.IsEmpty())
			iWidget.LoadImageFromSet(0, m_sImageset, stateConf.m_sImagesetQuad);
		
		if (!m_bScaleElement)
			FrameSlot.SetSize(iWidget, m_iImageSizeMin, m_iImageSizeMin);
		
		iWidget.SetColor(stateConf.m_vColor);
		
		data.SetVisibility(iWidget, stateConf.m_fOpacityDefault != 0, stateConf.m_fOpacityDefault, stateConf.m_bAnimateTransition); // transitions
	}
};

//------------------------------------------------------------------------------------------------
//! Zone element for state icon
[BaseContainerProps(), SCR_NameTagElementTitle()]
class SCR_NTIconState : SCR_NTIconBase
{
	[Attribute("0", UIWidgets.CheckBox, "Whether this element should use defined faction rank icons in default state")]
	protected bool m_bUseRanks;
	
	//------------------------------------------------------------------------------------------------
	override void SetDefaults(SCR_NameTagData data, int index)
	{		
		super.SetDefaults(data, index);
		
		// if using ranks
		if (!m_bUseRanks)
			return;
		
		ImageWidget iWidget = ImageWidget.Cast( data.m_aNametagElements[index] );
		if (iWidget)
		{
			SCR_NTStateIcon stateConf = SCR_NTStateIcon.Cast( GetEntityStateConfig(data) );
			if (stateConf)
			{
				if (stateConf.m_eEntityState == ENameTagEntityState.DEFAULT)
				{
					ResourceName rankIcon = SCR_CharacterRankComponent.GetCharacterRankInsignia(data.m_Entity);
					if (rankIcon)
						iWidget.LoadImageTexture(0, rankIcon);
				}
			}
		}
	}
};


[BaseContainerProps(), SCR_NameTagElementTitle()]
class SCR_NTIconPlatform : SCR_NTIconBase
{	
	BaseContainer m_GameplaySettings;

	//------------------------------------------------------------------------------------------------	
	override void SetDefaults(SCR_NameTagData data, int index)
	{
		ImageWidget iWidget = ImageWidget.Cast( data.m_aNametagElements[index] );
		if (!iWidget)
			return;
		
		if (!m_GameplaySettings)
			m_GameplaySettings = GetGame().GetGameUserSettings().GetModule("SCR_GameplaySettings");
		
		data.SetVisibility(iWidget, false, 0, false);
		
		if (!m_bScaleElement)
			FrameSlot.SetSize(iWidget, m_iImageSizeMin, m_iImageSizeMin);
		
		if (data.m_eType != ENameTagEntityType.PLAYER && data.m_eType != ENameTagEntityType.VEHICLE)
			return;
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;
		
		bool showOnPC;
		if (m_GameplaySettings)
			m_GameplaySettings.Get("m_bPlatformIconNametag", showOnPC);
		
		if (data.m_eType == ENameTagEntityType.PLAYER)
		{
			if (playerController.SetPlatformImageTo(data.m_iPlayerID, iWidget, showOnPC : showOnPC))
				data.SetVisibility(iWidget, true, 100, false);
		}
		else if (data.m_eType == ENameTagEntityType.VEHICLE)
		{
			SCR_VehicleTagData tagData = SCR_VehicleTagData.Cast(data);
			if (!data)
				return; 
			
			if (playerController.SetPlatformImageTo(tagData.GetMainPlayerID(), iWidget, showOnPC : showOnPC))
				data.SetVisibility(iWidget, true, 100, false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateElement(SCR_NameTagData data, int index)
	{
		super.UpdateElement(data, index);
		
		if (!data.m_aNametagElements[index])
			return;
		
		if (data.m_eType != ENameTagEntityType.PLAYER && data.m_eType != ENameTagEntityType.VEHICLE)
			return;
		
		ImageWidget image = ImageWidget.Cast(data.m_aNametagElements[index]);
		if (!image)
			return;
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;
		
		bool showOnPC;
		if (m_GameplaySettings)
			m_GameplaySettings.Get("m_bPlatformIconNametag", showOnPC);
		
		if (data.m_eType == ENameTagEntityType.PLAYER)
		{
			if (playerController.SetPlatformImageTo(data.m_iPlayerID, image, showOnPC : showOnPC))
				data.SetVisibility(image, true, 100, false);
		} 
		else if (data.m_eType == ENameTagEntityType.VEHICLE)
		{
			SCR_VehicleTagData tagData = SCR_VehicleTagData.Cast(data);
			if (!tagData)
				return; 
			
			if (playerController.SetPlatformImageTo(tagData.GetMainPlayerID(), image, showOnPC : showOnPC))
				tagData.SetVisibility(image, true, 100, false);
		}
	}
}