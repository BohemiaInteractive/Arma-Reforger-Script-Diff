enum ELocalAmbientSignal
{
	SoundName,
	EntitySize,
	AboveTerrain,
	SunAngle,
	EnvironmentType,
	Seed,
	Distance
}

[EntityEditorProps(category: "GameScripted/Sound", description: "Component handling ambient Insects", color: "0 0 255 255")]
class SCR_AmbientInsectsComponentClass : ScriptComponentClass
{
}

//------------------------------------------------------------------------------------------------
class SCR_AmbientInsectsComponent : ScriptComponent
{
	[Attribute(defvalue: "4", UIWidgets.Slider, params: "0 60 1", desc: "How frequently are Insects updated around the camera")]
	protected int m_iUpdateRate;

	[Attribute()]
	protected ref array<ref SCR_AmbientInsectsEffect> m_aAmbientInsectsEffect;

	// Components
	protected SignalsManagerComponent m_LocalSignalsManager;
	protected GameSignalsManager m_GlobalSignalsManager;
	protected SCR_AmbientSoundsComponent m_AmbientSoundsComponent;

	// Misc
	protected int m_iEnvironmentTypeSignalValue;

	//! AmbientEntity Signal's manager signals indexes
	protected ref array<int> m_aLocalAmbientSignalIndex = {};

	protected float m_fTimeOfDay;
	protected float m_fRainIntensity;
	protected float m_fWindSpeed;

	protected int m_iSunAngleSignalIdx;
	protected int m_iRainIntensitySignalIdx;
	protected int m_iWindSpeedSignalIdx;

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetEnvironmentType()
	{
		return m_iEnvironmentTypeSignalValue;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetTimeOfDay()
	{
		return m_fTimeOfDay;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetRainIntensity()
	{
		return m_fRainIntensity;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetWindSpeed()
	{
		return m_fWindSpeed;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] effect
	void RemoveInsectEffect(SCR_AmbientInsectsEffect effect)
	{
		m_aAmbientInsectsEffect.RemoveItem(effect);
	}

	//------------------------------------------------------------------------------------------------
	protected void Update(IEntity owner)
	{
		float worldTime = owner.GetWorld().GetWorldTime();
		vector cameraPos = m_AmbientSoundsComponent.GetCameraOrigin();
		
		// Get spawn preset = EnvironmentType
		if (m_aLocalAmbientSignalIndex[ELocalAmbientSignal.EnvironmentType] != -1)
			m_iEnvironmentTypeSignalValue = m_LocalSignalsManager.GetSignalValue(m_aLocalAmbientSignalIndex[ELocalAmbientSignal.EnvironmentType]);

		// Get time of day based on sun position
		m_fTimeOfDay = m_LocalSignalsManager.GetSignalValue(m_iSunAngleSignalIdx) / 360;
		m_fRainIntensity = m_GlobalSignalsManager.GetSignalValue(m_iRainIntensitySignalIdx);
		m_fWindSpeed = m_GlobalSignalsManager.GetSignalValue(m_iWindSpeedSignalIdx);
		
		//We are using for loop because these effects can get removed during run-time from the update loop
		for (int i = m_aAmbientInsectsEffect.Count() - 1; i >= 0; i--)
		{
			if (m_aAmbientInsectsEffect.IsIndexValid(i))
				m_aAmbientInsectsEffect[i].Update(worldTime, cameraPos, m_fTimeOfDay, m_fRainIntensity);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		if (RplSession.Mode() == RplMode.Dedicated)
			return;

		// Get local signals component
		m_LocalSignalsManager = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
		if (!m_LocalSignalsManager)
		{
			Print("AMBIENT LIFE: SCR_AmbientInsectsComponent: Missing SignalsManagerComponent", LogLevel.WARNING);
			return;
		}

		m_GlobalSignalsManager = GetGame().GetSignalsManager();

		// Get Local Ambient Signal Idx
		typename enumType = ELocalAmbientSignal;
		int size = enumType.GetVariableCount();

		for (int i = 0; i < size; i++)
		{
			m_aLocalAmbientSignalIndex.Insert(m_LocalSignalsManager.AddOrFindSignal(typename.EnumToString(ELocalAmbientSignal, i)));
		}

		m_iSunAngleSignalIdx = m_LocalSignalsManager.AddOrFindSignal("SunAngle");
		m_iRainIntensitySignalIdx = m_GlobalSignalsManager.AddOrFindSignal("RainIntensity");
		m_iWindSpeedSignalIdx = m_GlobalSignalsManager.AddOrFindSignal("WindSpeed");

		m_AmbientSoundsComponent = SCR_AmbientSoundsComponent.Cast(owner.FindComponent(SCR_AmbientSoundsComponent));
		if (!m_AmbientSoundsComponent)
		{
			Print("AMBIENT LIFE: SCR_AmbientInsectsComponent: Missing SCR_AmbientSoundsComponent", LogLevel.WARNING);
			return;
		}

		foreach (SCR_AmbientInsectsEffect ambientInsectsEffect : m_aAmbientInsectsEffect)
		{
			ambientInsectsEffect.OnPostInit(m_AmbientSoundsComponent, this, m_LocalSignalsManager);
		}

		Update(owner);
		GetGame().GetCallqueue().CallLater(Update, 1000 * m_iUpdateRate, true, owner);
	}

#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_AmbientInsectsComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AMBIENT_LIFE, "", "Ambient Life", "World");
	}
#endif
}
