[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_CollimatorSightsComponentClass : BaseCollimatorSightsComponentClass
{
};

[BaseContainerProps()]
class SCR_ReticleColor
{
	[Attribute("1 1 1", UIWidgets.ColorPicker, desc: "Reticle Color", category: "Reticle")]
	vector m_vReticleColor;
	[Attribute("1 1 1", UIWidgets.ColorPicker, desc: "Reticle Glow Color", category: "Reticle")]
	vector m_vGlowColor;
};

class SCR_CollimatorSightsComponent : BaseCollimatorSightsComponent
{
	// For the Input manager
	const string ACTION_ILLUMINATION = "WeaponToggleSightsIllumination";
	
	// Attributes
	[Attribute("9", UIWidgets.Slider, "Reticle Daylight Brightness", params: "0 15 0.1", category: "Collimator")]
	protected float m_fDaylightBrightness;
	
	[Attribute("2", UIWidgets.Slider, "Reticle Night Brightness", params: "0 15 0.1", category: "Collimator")]
	protected float m_fNightBrightness;
	
	//------------------------------------------------------------------------------------------------
	SCR_CollimatorControllerComponent m_managerComponent;
	bool m_bReticleDaylight = true;
	int m_iColorIndex = -1;
	//------------------------------------------------------------------------------------------------
	override float GetReticleBrightnessDay()
	{
		return m_fDaylightBrightness;
	}

	override float GetReticleBrightnessNight()
	{
		return m_fNightBrightness;
	}
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		m_managerComponent = SCR_CollimatorControllerComponent.Cast(FindComponent( SCR_CollimatorControllerComponent ));
		if (!m_managerComponent)
		{
			Print("Warning: No Collimator Controller on this sight", LogLevel.WARNING);
		}
			
		if (m_managerComponent)
		{
			m_managerComponent.SetReticleBrightness(m_fDaylightBrightness);
		}
	}
	
	override void OnSightADSActivate()
	{
		GetGame().GetInputManager().AddActionListener(ACTION_ILLUMINATION, EActionTrigger.DOWN, ToggleIllumination);
	}
	
	override void OnSightADSDeactivated()
	{
		GetGame().GetInputManager().RemoveActionListener(ACTION_ILLUMINATION, EActionTrigger.DOWN, ToggleIllumination);
	}
	
	//------------------------------------------------------------------------------------------------
	
	protected void ToggleIllumination(float value, EActionTrigger trigger)
	{
		ReticleToggleIllumination();	
	}
	
	//------------------------------------------------------------------------------------------------
	
	void ReticleToggleIllumination()
	{
		m_bReticleDaylight = !m_bReticleDaylight;
		
		if (m_managerComponent)
		{
			if (m_bReticleDaylight)
				m_managerComponent.SetReticleBrightness(m_fDaylightBrightness);
			else
				m_managerComponent.SetReticleBrightness(m_fNightBrightness);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateReticlePosition(float u, float v, float uScale, float vScale)
	{
		if (m_managerComponent)
			m_managerComponent.UpdateUVCoordinates(u, v, uScale, vScale);
	}
	
	override void UpdateReticleShapeIndex(int index)
	{
		if (m_managerComponent)
			m_managerComponent.SetReticleIndex(index);
	}
	
	override void UpdateReticleColor(vector inner, vector glow)
	{
		if (m_managerComponent && inner && glow)
			m_managerComponent.SetReticleColors(inner, glow);
	}
	
	override void UpdateReticleBrightnessScale(float scale)
	{
		if (m_managerComponent)
			m_managerComponent.SetReticleBrightnessScale(scale);
	}

	override void UpdateReticleBrightness(float lvFactor, bool useOwn)
	{
		if (m_managerComponent)
		{
			if (useOwn)
			{
				if (m_bReticleDaylight)
					m_managerComponent.SetReticleBrightness(m_fDaylightBrightness);
				else
					m_managerComponent.SetReticleBrightness(m_fNightBrightness);
			}
			else
				m_managerComponent.SetReticleBrightness(lvFactor);
		}
	}
};
