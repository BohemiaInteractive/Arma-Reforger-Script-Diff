[EntityEditorProps(category: "GameScripted", color: "0 0 255 255")]
class SCR_FlareEffectComponentClass : SCR_BaseInteractiveLightComponentClass
{
	[Attribute("0", UIWidgets.EditBox, "Lifetime of light in seconds")]
	protected float m_iLightLifetime;

	//------------------------------------------------------------------------------------------------
	float GetLightLifeTime()
	{
		return m_iLightLifetime;
	}
}

class SCR_FlareEffectComponent : SCR_BaseInteractiveLightComponent
{
	protected SCR_BaseInteractiveLightComponentClass m_ComponentData;
	protected float m_fTimer;
	protected float m_fCompDataLV;
	protected float m_fBottomLightLV;
	
	protected static const float FLICKER_STEP = 0.15;
	protected static const float FLICKER_THRESHOLD = 0.15;
	protected static const float FLICKER_FREQUENCY = 1 / 20;
	
	//------------------------------------------------------------------------------------------------
	override void ToggleLight(bool turnOn, bool skipTransition = false, bool playSound = true)
	{	
		if (m_bIsOn == turnOn)
			return;
		
		super.ToggleLight(turnOn, skipTransition, playSound);	
		
		// don't continue if DS server is running in console (particles, decals)
		if (System.IsConsoleApp())
			return;		
		
		if (turnOn)
		{		
			TurnLightOn();
		}	
		else
		{
			TurnLightOff();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void TurnLightOn()
	{
		SCR_FlareEffectComponentClass componentData = SCR_FlareEffectComponentClass.Cast(GetComponentData(GetOwner()));
		
		if (componentData)
		{
			m_fCompDataLV = componentData.GetLightLV();
			m_fBottomLightLV = m_fCompDataLV;
		}
		
		SetEventMask(GetOwner(), EntityEvent.FIXEDFRAME);
	}
	
	//------------------------------------------------------------------------------------------------
	void TurnLightOff()
	{		
		ClearEventMask(GetOwner(), EntityEvent.FIXEDFRAME);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{	
		m_ComponentData = SCR_BaseInteractiveLightComponentClass.Cast(GetComponentData(GetOwner()));
		if (m_ComponentData)
			m_fCurLV = m_ComponentData.GetLightLV();
		
		TurnLightOn();

		super.OnPostInit(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnVisible(IEntity owner, int frameNumber)
	{
		// needs to be empty to prevent duplicate effects. Moved into EOnFixedFrame
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFixedFrame(IEntity owner, float timeSlice)
	{
		if (!m_bIsOn || m_aLights.IsEmpty())
			return;
		
		m_fTimer += timeSlice;
		
		if (m_fTimer < FLICKER_FREQUENCY)
			return;
		
		// Needs to be redone to be more realistic and correctly replicated
		// Lowering the light intensity by flare timer
		/*
		SCR_FlareEffectComponentClass componentData = SCR_FlareEffectComponentClass.Cast(GetComponentData(GetOwner()));
		m_fBottomLightLV -= m_fCompDataLV / (componentData.GetLightLifeTime() + 1) * m_fTimer;
		*/
		
		//flicker
		if (Math.RandomIntInclusive(0, 1) == 0)
			m_fCurLV += FLICKER_STEP;
		else
			m_fCurLV -= FLICKER_STEP;
		
		m_fCurLV = Math.Clamp(m_fCurLV, m_fBottomLightLV * (1 - FLICKER_THRESHOLD),  m_fBottomLightLV * (1 + FLICKER_THRESHOLD));
		m_fCurLV = Math.Clamp(m_fCurLV, 0, 20); // needed to avoid going over engine limits for LVs
		
		//reset timer
		m_fTimer = 0;
		
		//set light
		array<ref SCR_BaseLightData> lightData = m_ComponentData.GetLightData();
		for (int i = Math.Min(m_aLights.Count(), lightData.Count()) -1; i >= 0; i--)
		{
			SCR_BaseLightData ld = lightData[i]; // make sure that data is loaded
			if (ld)
				m_aLights[i].SetColor(Color.FromVector(ld.GetLightColor()), m_fCurLV);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		
		RemoveLights();
	}
}