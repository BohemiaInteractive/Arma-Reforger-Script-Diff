class SCR_RestrictionZoneWarningHUDComponent : SCR_InfoDisplay
{
	[Attribute("Holds array of warning info's to display the proper warning visuals.")] 
	protected ref array<ref SCR_RestrictionZoneWarningInfo> m_aWarningUIInfos; //~ Add new entries in SCR_HUDManagerComponent on PlayerController 
	
	[Attribute("Icon")]
	protected string m_sIconWidgetName;
	
	[Attribute("Warning")]
	protected string m_sWarningWidgetName;
	
	[Attribute("Explanation")]
	protected string m_sExplanationWidgetName;
	
	[Attribute("WarningZoneDistanceVisuals")]
	protected string m_sWarningZoneDistanceVisualsName;
	
	[Attribute("WarningVisualsHolder")]
	protected string m_sWarningVisualsHolderName;
	
	[Attribute("0.05", desc: "If zone is active how fast does the zone visuals update depending on the distance? In seconds.")]
	protected float m_fWarningVisualsUpdateTime;
	
	[Attribute("0", desc: "Min waning visuals opacity. Opacity becomes stronger when moving closer to edge of the zone.", uiwidget: UIWidgets.Slider, params: "0 1 0.00001")]
	protected float m_fMinWarningDistanceVisualsOpacity;
	
	[Attribute("1", desc: "Max waning visuals opacity. Opacity becomes stronger when moving closer to edge of the zone.", uiwidget: UIWidgets.Slider, params: "0 1 0.00001")]
	protected float m_fMaxWarningDistanceVisualsOpacity;
	
	[Attribute("0.4", desc: "The minimum opacity of the warning text", uiwidget: UIWidgets.Slider, params: "0 1 0.00001")]
	protected float m_fWarningVisualsMinOpacity;
	
	protected Widget m_WarningZoneDistanceVisuals;
	protected Widget m_WarningVisualsHolder;
	
	protected ERestrictionZoneWarningType m_iCurrentDisplayedWarning = -1;
	
	protected vector m_vZoneCenter;
	protected float m_fWarningRadiusSq;
	protected float m_fZoneMinusWarningSq;

	
	/*!
	Show the restriction zone warning and set the correct UIInfo
	\param show If the warning should be shown or hidden
	\param speed fadeSpeed
	*/
	void ShowZoneWarning(bool show, ERestrictionZoneWarningType warningType, vector zoneCenter, float warningRadiusSq, float zoneRadiusSq, float speed = UIConstants.FADE_RATE_DEFAULT)
	{		
		if (!show)
		{
			if (m_bShown)
				GetGame().GetCallqueue().Remove(UpdateVisualWarning);
			
			Show(false, speed);
			return;
		}
		
		m_vZoneCenter = zoneCenter;
		m_fWarningRadiusSq = warningRadiusSq;
		m_fZoneMinusWarningSq = zoneRadiusSq - warningRadiusSq;
		
		if (m_iCurrentDisplayedWarning != warningType)
		{
			foreach (SCR_RestrictionZoneWarningInfo warningInfo: m_aWarningUIInfos)
			{
				if (warningInfo.m_iWarningType == warningType)
				{
					if (warningInfo.m_UIInfo)
					{
						warningInfo.m_UIInfo.SetIconTo(ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sIconWidgetName)));
						warningInfo.m_UIInfo.SetNameTo(TextWidget.Cast(m_wRoot.FindAnyWidget(m_sWarningWidgetName)));
						warningInfo.m_UIInfo.SetDescriptionTo(TextWidget.Cast(m_wRoot.FindAnyWidget(m_sExplanationWidgetName)));
					}
					break;
				}
			}
		}
		
		m_iCurrentDisplayedWarning = warningType;
		
		if (!m_bShown)
			GetGame().GetCallqueue().CallLater(UpdateVisualWarning, m_fWarningVisualsUpdateTime * 1000, true);
		
		Show(true, speed);

	}
	
	protected void UpdateVisualWarning()
	{
		IEntity playerEntity = SCR_PlayerController.GetLocalControlledEntity();
		vector playerPosition = playerEntity.GetOrigin();
		
		//~ Calculate opacity Strenght depending on distance
		float opacityCalc = ((vector.DistanceSqXZ(playerPosition, m_vZoneCenter) - m_fWarningRadiusSq) / m_fZoneMinusWarningSq) + m_fMinWarningDistanceVisualsOpacity;
		float opacity = Math.Clamp(opacityCalc, m_fMinWarningDistanceVisualsOpacity, m_fMaxWarningDistanceVisualsOpacity);
		
		float ratio = 0;
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (cameraManager)
		{
			CameraBase camera = cameraManager.CurrentCamera();
			if (camera)
			{
				//~ Calculate opacity multiplier depending on where the player is looking (Looking towards center == less opacity)
				vector cameraTransform[4];
				camera.GetWorldTransform(cameraTransform);
				
				//~ Grab the ratio between zone and center. Looking at zone == 1 and looking fully away from zone == -1
				ratio = vector.DotXZ((m_vZoneCenter - playerPosition).Normalized(), cameraTransform[2]);
			}
		}
		
		//~ If looking less the 180 degrees towards the center start reducing the opacity using a multiplier
		float opacityLookDirectionMultiplier = 1;
		if (ratio > 0)
			opacityLookDirectionMultiplier = 1 - ratio;

		//~ Set final Opacity
		m_WarningZoneDistanceVisuals.SetOpacity(opacity * opacityLookDirectionMultiplier);
		
		//~ Warning visuals opacity
		if (opacityLookDirectionMultiplier > m_fWarningVisualsMinOpacity)
			m_WarningVisualsHolder.SetOpacity(opacityLookDirectionMultiplier);
		else 
			m_WarningVisualsHolder.SetOpacity(m_fWarningVisualsMinOpacity);
	}
	
	
	override event void OnStartDraw(IEntity owner)
	{
		super.OnStartDraw(owner);
		
		if (!m_wRoot)
			return;
		
		if (!m_WarningZoneDistanceVisuals)
		{
			if (m_fMinWarningDistanceVisualsOpacity > m_fMaxWarningDistanceVisualsOpacity)
			{
				Print("'SCR_RestrictionZoneWarningHUDComponent' m_fMinWarningDistanceVisualsOpacity is greater then m_fMaxWarningDistanceVisualsOpacity! This should never be greater then max and is set the same.", LogLevel.WARNING);
				m_fMinWarningDistanceVisualsOpacity = m_fMaxWarningDistanceVisualsOpacity;
			}
			
			m_WarningZoneDistanceVisuals = m_wRoot.FindAnyWidget(m_sWarningZoneDistanceVisualsName);
			m_WarningVisualsHolder = m_wRoot.FindAnyWidget(m_sWarningVisualsHolderName);
			
			if (m_WarningZoneDistanceVisuals)
				m_WarningZoneDistanceVisuals.SetOpacity(0);
			else 
				Print("'SCR_RestrictionZoneWarningHUDComponent' could not find m_WarningZoneDistanceVisuals. This will break the zone restriction warning!", LogLevel.ERROR);
			
			if (m_WarningVisualsHolder)
				m_WarningVisualsHolder.SetOpacity(0);
			else
				Print("'SCR_RestrictionZoneWarningHUDComponent' could not find m_WarningVisualsHolder. This will break the zone restriction warning!", LogLevel.ERROR);
		}
	}
	
	
	void ~SCR_RestrictionZoneWarningHUDComponent()
	{
		if (m_bShown)
				GetGame().GetCallqueue().Remove(UpdateVisualWarning);
	}
};

/*!
Class that holds RestrictionZone Warning UIInfos
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ERestrictionZoneWarningType, "m_iWarningType")]
class SCR_RestrictionZoneWarningInfo
{
	[Attribute("0", desc: "Which warning UIinfo needs to be shown on the screen as defined in 'SCR_RestrictionZoneWarningHUDComponent'", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(ERestrictionZoneWarningType))]
	ERestrictionZoneWarningType m_iWarningType;
	
	[Attribute(desc: "UI info to dictate what to display in warning")]
	ref SCR_UIInfo m_UIInfo;
}
