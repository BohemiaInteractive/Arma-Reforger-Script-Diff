[BaseContainerProps(configRoot: true)]
class SCR_LightManagerInfoEntry : Managed
{
	[Attribute(SCR_Enum.GetDefault(ELightType.NoLight), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ELightType))]
	ELightType m_eLightType;

	[Attribute("-1", uiwidget: UIWidgets.ComboBox, enums: { ParamEnum("Either", "-1"), ParamEnum("Left", "0"), ParamEnum("Right", "1")})]
	int m_iLightSide;
	
	[Attribute("", UIWidgets.EditBox, "Indicator icon to be displayed.")]
	string m_sIcon;
	
	[Attribute(SCR_Enum.GetDefault(EVehicleInfoColor.WHITE), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EVehicleInfoColor))]
	EVehicleInfoColor m_eColor;	
};

class SCR_LightManagerInfoMulti : SCR_BaseVehicleInfo
{
	[Attribute("", UIWidgets.Auto)]
	protected ref array<ref SCR_LightManagerInfoEntry> m_aLights;
	
	protected BaseLightManagerComponent m_pLightManager;
	protected SCR_LightManagerInfoEntry m_pActiveLight;
	protected bool m_bActiveLightChanged;

	//------------------------------------------------------------------------------------------------
	//! Can be overridden to get state of actual system or linked to an event
	override EVehicleInfoState GetState()
	{
		bool enabled;
		SCR_LightManagerInfoEntry lightPrev = m_pActiveLight;
		
		foreach (SCR_LightManagerInfoEntry light : m_aLights)
		{
			if (!light || light.m_eLightType == ELightType.NoLight)
				continue;
			
			enabled = m_pLightManager && m_pLightManager.GetLightsState(light.m_eLightType, light.m_iLightSide);
			
			if (enabled)
			{
				m_pActiveLight = light;
				break;
			}
		}		
		
		if (m_pActiveLight != lightPrev)
			m_bActiveLightChanged = true;
		
		if (enabled)
		{
			return EVehicleInfoState.ENABLED;
		}
		else
		{
			m_pActiveLight = null;
			return EVehicleInfoState.DISABLED;
		}
			
	}

	//------------------------------------------------------------------------------------------------
	override void SetIcon(string icon)
	{
		if (m_pActiveLight)
			icon = m_pActiveLight.m_sIcon;
		
		super.SetIcon(icon);
	}	
	
	//------------------------------------------------------------------------------------------------
	override void SetColor(EVehicleInfoState state, EVehicleInfoColor color)
	{
		if (m_pActiveLight)
			color = m_pActiveLight.m_eColor;
		
		super.SetColor(state, color);
	}	
	
	//------------------------------------------------------------------------------------------------
	override bool UpdateRequired(EVehicleInfoState state)
	{
		bool isRequired = m_eState != state || m_bActiveLightChanged;
		
		m_bActiveLightChanged = false;
		
		return isRequired;
	}		
	
	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{
		// Terminate if there is no light manager
		if (!m_pLightManager)
			return false;

		return super.DisplayStartDrawInit(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! Init the UI, runs 1x at the start of the game
	override void DisplayInit(IEntity owner)
	{
		super.DisplayInit(owner);

		m_pLightManager = BaseLightManagerComponent.Cast(owner.FindComponent(BaseLightManagerComponent));
	}
};
