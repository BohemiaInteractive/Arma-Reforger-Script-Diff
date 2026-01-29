//! Spawns Insect particles around camera based on define behaviour
[BaseContainerProps(configRoot: true)]
class SCR_RandomPositionalInsects : SCR_AmbientInsectsEffect
{
	[Attribute(defvalue: "900", UIWidgets.Slider, params: "100 100000 1", desc: "Random positional insects that exceed this range (squared distance) will be despawned")]
	protected int m_iDespawnDistanceSq;
	
	[Attribute()]
	protected ref array<ref SCR_RandomPositionalInsectsDef> m_aRandomPositionalInsectsDef;

	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Daytime curves config", params: "conf")]
	protected ResourceName m_sDayTimeCurve;

	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Wind modifier curves config", params: "conf")]
	protected ResourceName m_sWindCurve;

	// Configuration
	protected ref array<ref SCR_InsectGroup> m_aInsectGroup = {};
	protected ref array<ref SCR_InsectSpawnDef> m_aSpawnDef = {};
	protected ref array<ref SCR_DayTimeCurveDef> m_aDayTimeCurve = {};
	protected ref array<ref SCR_WindCurveDef> m_aWindModifier = {};

	// Components
	protected SoundWorld m_SoundWorld;

	protected const int INVALID = -1;
	protected int m_iUpdatedInsectGroupIdx;
	protected ref array<int> m_aDensity = {};
	protected ref array<float> m_aDensityModifier = {};
	protected ref array<int> m_aAngleOffset = {};
	protected float m_aDayTimeCurveValue[DAY_TIME_CURVE_COUNT];
	protected float m_aWindCurveValue[WIND_CURVE_COUNT];

	protected ref array<ref SCR_InsectParticle> m_aParticles = {};

	//------------------------------------------------------------------------------------------------
	// Called by SCR_AmbientInsectsComponent.Update()
	override void Update(float worldTime, vector cameraPos, float timeOfDay, float rainIntensity)
	{

		RemoveOutOfRangeParticles(cameraPos);
		RemoveParticlesBasedOnTimeAndWeather(timeOfDay, rainIntensity);
		UpdateInsectGroup(worldTime, cameraPos, m_iUpdatedInsectGroupIdx);

		m_iUpdatedInsectGroupIdx ++;
		if (m_iUpdatedInsectGroupIdx < m_aInsectGroup.Count())
			return;

		m_iUpdatedInsectGroupIdx = 0;

		UpdateGlobalModifiers(timeOfDay, rainIntensity);
	}

	//------------------------------------------------------------------------------------------------
	//! Updates time and weather for respective Insect groups
	//! \param[in] timeOfDay
	//! \param[in] rainIntensity
	protected void UpdateGlobalModifiers(float timeOfDay, float rainIntensity)
	{
		UpdateDensity();

		//Update values of wind modifiers curves
		float windSpeedScaled = Math.InverseLerp(SCR_AmbientSoundsComponent.WINDSPEED_MIN, SCR_AmbientSoundsComponent.WINDSPEED_MAX, m_AmbientInsectsComponent.GetWindSpeed());

		for (int i = 0; i < WIND_CURVE_COUNT; i++)
		{
			m_aWindCurveValue[i] = SCR_AmbientSoundsComponent.GetPoint(windSpeedScaled, m_aWindModifier[i].m_Curve);
		}

		// Get modifiers for all sound types
		int environmentType = m_AmbientInsectsComponent.GetEnvironmentType();

		foreach (int i, SCR_InsectSpawnDef spawnDef : m_aSpawnDef)
		{
			float modifier = SCR_AmbientSoundsComponent.GetPoint(timeOfDay, spawnDef.m_TimeModifier);
			modifier = modifier * SCR_AmbientSoundsComponent.GetPoint(rainIntensity, spawnDef.m_RainModifier);
			m_aDensityModifier[i] = modifier * m_aWindCurveValue[spawnDef.m_eWindModifier];
		}

		// Update values of day time curves
		for (int i = 0; i < DAY_TIME_CURVE_COUNT; i++)
		{
			m_aDayTimeCurveValue[i] = SCR_AmbientSoundsComponent.GetPoint(timeOfDay, m_aDayTimeCurve[i].m_Curve);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Get maximal density per insect group and sound map type
	//! \return density
	protected int GetDensityMax(int insectGroup, int soundMapType)
	{
		if (soundMapType == 0)
			return m_aSpawnDef[insectGroup].m_iSeaDensityMax;
		else if (soundMapType == 1)
			return m_aSpawnDef[insectGroup].m_iForestDensityMax;
		else if (soundMapType == 2)
			return m_aSpawnDef[insectGroup].m_iCityDensityMax;
		else if (soundMapType == 3)
			return m_aSpawnDef[insectGroup].m_iMeadowDensityMax;
		else if (soundMapType == 4)
			return m_aSpawnDef[insectGroup].m_iRiverDensityMax;
		else if (soundMapType == 5)
			return m_aSpawnDef[insectGroup].m_iRiverSlopeDensityMax;
		else if (soundMapType == 6)
			return m_aSpawnDef[insectGroup].m_iLakeDensityMax;
		else
			return m_aSpawnDef[insectGroup].m_iCoastDensityMax;
	}

	//------------------------------------------------------------------------------------------------
	//! Deletes Insects that are too far away
	//! \param[in] cameraPos
	protected void RemoveOutOfRangeParticles(vector cameraPos)
	{
		for (int i = m_aParticles.Count() - 1; i >= 0; i--)
		{
			if (vector.DistanceSqXZ(m_aParticles[i].m_vPosition, cameraPos) >= m_iDespawnDistanceSq)
			{
				if (m_aParticles[i].m_AudioHandle)
					m_AmbientSoundsComponent.TerminateLooped(m_aParticles[i].m_AudioHandle);

				m_aParticles.Remove(i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Deletes Insects that are not supposed to be spawned based on time and weather conditions
	//! \param[in] timeOfDay
	//! \param[in] rainIntensity
	protected void RemoveParticlesBasedOnTimeAndWeather(float timeOfDay, float rainIntensity)
	{
		for (int i = m_aParticles.Count() - 1; i >= 0; i--)
		{
			float dayTimeCurve = SCR_AmbientSoundsComponent.GetPoint(timeOfDay, m_aSpawnDef[m_aParticles[i].m_iSoundGroup].m_TimeModifier);
			float rainIntensityCurve = SCR_AmbientSoundsComponent.GetPoint(rainIntensity, m_aSpawnDef[m_aParticles[i].m_iSoundGroup].m_RainModifier);
			if (dayTimeCurve <= 0 || rainIntensityCurve <= 0)
			{
				if (m_aParticles[i].m_AudioHandle)
					m_AmbientSoundsComponent.TerminateLooped(m_aParticles[i].m_AudioHandle);

				m_aParticles.Remove(i);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Randomizes animation for Insect
	//! \param[in] entity
	protected void RandomizeAnimation(IEntity entity)
	{
		AnimationPlayerComponent animComponent = AnimationPlayerComponent.Cast(entity.FindComponent(AnimationPlayerComponent));
		if (!animComponent)
			return;
		
		ResourceName animationResource = animComponent.GetAnimation();
		if (!animationResource || SCR_StringHelper.IsEmptyOrWhiteSpace(animationResource))
			return;
			
		animComponent.Prepare(animationResource, Math.RandomFloatInclusive(1, 5), 1, true);
		animComponent.Play();
	}

	//------------------------------------------------------------------------------------------------
	//! Attempts to find new position to play sound based on defined ESpawnMethod
	//! \param[in] worldTime
	//! \param[in] camPos
	//! \param[in] insectGroup
	protected void UpdateInsectGroup(float worldTime, vector camPos, int insectGroup)
	{
		// Generate sound position
		vector vPos;
		int insectType = INVALID;
		vPos = GenerateRandomPosition(insectGroup, camPos);
		insectType = GetSoundMapTypeFromTerrain(vPos, camPos);
		if (insectType == INVALID)
			return;

		// Return, if density was reached
		// Config density modified by time, wind and rain modifiers
		if (m_aDensity[insectGroup] > GetDensityMax(insectGroup, insectType) * m_aDensityModifier[insectGroup])
			return;

		// Get random sound definition
		SCR_InsectDef insectDef;
		if (m_aInsectGroup[insectGroup].m_aInsectType.IsIndexValid(insectType))
			insectDef = GetRandomInsectDef(m_aInsectGroup[insectGroup].m_aInsectType[insectType]);
		
		if (!insectDef)
			return;
		
		string particleName = insectDef.m_sParticleEffect;
		string soundName = insectDef.m_sSoundName;
		ResourceName prefabName = insectDef.m_sPrefabName;

		ParticleEffectEntitySpawnParams spawnParams = new ParticleEffectEntitySpawnParams();
		spawnParams.Transform[3] = vPos;

		SCR_InsectParticle particle = new SCR_InsectParticle();
		particle.m_iSoundGroup = insectGroup;
		particle.m_vPosition = vPos;
		if (Math.RandomIntInclusive(0, 100) <= m_aRandomPositionalInsectsDef[insectGroup].m_iSpawnChance)
		{
			if (!soundName.IsEmpty())
				particle.m_AudioHandle = m_AmbientSoundsComponent.SoundEventLooped(soundName, spawnParams.Transform);

			if (!prefabName.IsEmpty())
			{
				Resource resource = Resource.Load(insectDef.m_sPrefabName);
				if (resource)
				{
					EntitySpawnParams entitySpawnParams = new EntitySpawnParams;
					entitySpawnParams.Transform = spawnParams.Transform;
					entitySpawnParams.TransformMode = spawnParams.TransformMode;
					
					particle.m_InsectEntity = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), entitySpawnParams);
					if (particle.m_InsectEntity)
					{
						particle.m_InsectEntity.SetVComponentFlags(VCFlags.DYNAMICBBOX);
						RandomizeAnimation(particle.m_InsectEntity);		
					}
				}
			}
			else if (!particleName.IsEmpty())
			{
				particle.m_ParticleEffect = ParticleEffectEntity.SpawnParticleEffect(insectDef.m_sParticleEffect, spawnParams);
			}

#ifdef ENABLE_DIAG
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AMBIENT_LIFE))
			{
				if (!soundName.IsEmpty())
					particle.m_DebugShape = Shape.CreateSphere(COLOR_GREEN, ShapeFlags.NOZWRITE | ShapeFlags.WIREFRAME, spawnParams.Transform[3], 0.2);
				else
					particle.m_DebugShape = Shape.CreateSphere(COLOR_BLUE, ShapeFlags.NOZWRITE | ShapeFlags.WIREFRAME, spawnParams.Transform[3], 0.2);
			}
#endif
		}

		m_aParticles.Insert(particle);
	}

	//------------------------------------------------------------------------------------------------
	//! Generates random position for the selected insect group around camera position
	//! \param[in] insectGroup
	//! \param[in] camPos
	//! \return
	protected vector GenerateRandomPosition(int insectGroup, vector camPos)
	{
		// Angle for random position is rotated by 40 deg for each GenerateRandomPosition() call
		float angle = m_aAngleOffset[insectGroup] * 40;

		m_aAngleOffset[insectGroup] = m_aAngleOffset[insectGroup] + 1;
		if (m_aAngleOffset[insectGroup] > 9)
			m_aAngleOffset[insectGroup] = 0;

		vector vPos = vector.FromYaw(angle) * Math.RandomFloat(10, 25);

		return vPos + camPos;
	}

	//------------------------------------------------------------------------------------------------
	//! Gets random insect def from the insect group
	//! \param[in] insectType
	protected SCR_InsectDef GetRandomInsectDef(SCR_InsectType insectType)
	{
		if (!insectType)
			return null;

		int size = insectType.m_aInsectDef.Count();
		if (size == 0)
			return null;

		int offset = Math.RandomIntInclusive(0, size - 1);

		for (int i = 0; i < size; i++)
		{
			int idx = i + offset;
			if (idx >= size)
				idx -= size;

			if (m_aDayTimeCurveValue[insectType.m_aInsectDef[idx].m_eDayTimeCurve] > 0)
				return insectType.m_aInsectDef[idx];
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Gets sound map type from terrain
	//! \param[in] worldPos
	//! \param[in] camPos
	protected int GetSoundMapTypeFromTerrain(inout vector worldPos, vector camPos)
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(m_AmbientInsectsComponent.GetOwner().GetWorld());
		if (!world)
			return INVALID;

		SoundWorld soundWorld = world.GetSoundWorld();
		if (!soundWorld)
			return INVALID;

		// We are getting these values twice - once for randomly found position around player, second on the camera position
		float seaCam, forestCam, cityCam, meadowCam;
		soundWorld.GetMapValuesAtPos(camPos, seaCam, forestCam, cityCam, meadowCam);

		float seaPos, forestPos, cityPos, meadowPos;
		soundWorld.GetMapValuesAtPos(worldPos, seaPos, forestPos, cityPos, meadowPos);

		float riverCam, riverSlopeCam, lakeCam, coastCam;
		soundWorld.GetMapValuesAdvAtPos(camPos, riverCam, riverSlopeCam, lakeCam, coastCam);

		float riverPos, riverSlopePos, lakePos, coastPos;
		soundWorld.GetMapValuesAdvAtPos(worldPos, riverPos, riverSlopePos, lakePos, coastPos);

		array<float> valuesPos = {seaPos, forestPos, cityPos, meadowPos, riverPos, riverSlopePos, lakePos, coastPos};
		array<float> valuesCam = {seaCam, forestCam, cityCam, meadowCam, riverCam, riverSlopeCam, lakeCam, coastCam};

		// We want to work with higher number
		for (int i; i < 8; i++)
		{
			if (valuesCam[i] > valuesPos[i])
				valuesPos[i] = valuesCam[i];
		}

		// Get maximum
		float v = valuesPos[0];
		int idx;
		int i = 1;
		for (i; i < 8; i++)
		{
			if (valuesPos[i] >= v)
			{
				v = valuesPos[i];
				idx = i;
			}
		}

		// Check for fresh water
		if (idx	== 0)
		{
			worldPos[1] = 0;
		}
		else
		{
			float surfaceHeight = world.GetSurfaceY(worldPos[0], worldPos[2]);
			worldPos[1] = surfaceHeight;
		}

		EWaterSurfaceType waterSurfaceType;
		float lakeArea;
		//Sometimes other biomes overlap and we want to prioritise water
		if (valuesPos[6] >= 0.75)
		{
			idx = 6;
			worldPos[1] = SCR_WorldTools.GetWaterSurfaceY(world, worldPos, waterSurfaceType, lakeArea);
		}

		return idx;
	}

	//------------------------------------------------------------------------------------------------
	//! Updates insect density per group and active particles
	protected void UpdateDensity()
	{
		m_aDensity.Clear();
		m_aDensity.Resize(m_aSpawnDef.Count());
		foreach (SCR_InsectParticle particle : m_aParticles)
		{
			m_aDensity[particle.m_iSoundGroup] = m_aDensity[particle.m_iSoundGroup] + m_aSpawnDef[particle.m_iSoundGroup].m_iDensityConsumption;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Loads configs where necessary values are stored
	protected void LoadConfigs()
	{
		foreach (SCR_RandomPositionalInsectsDef def : m_aRandomPositionalInsectsDef)
		{
			SCR_InsectGroup insectGroup;
			if (def.m_sInsectGroup != string.Empty)
			{
				Resource holder = BaseContainerTools.LoadContainer(def.m_sInsectGroup);
				if (holder)
					insectGroup = SCR_InsectGroup.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
			}

			if (!insectGroup)
			{
				Print("AMBIENT LIFE: SCR_AmbientInsectsComponent: Missing Ambience Sound Gropu Config", LogLevel.WARNING);
				m_AmbientInsectsComponent.RemoveInsectEffect(this);
				
				return;
			}

			m_aInsectGroup.Insert(insectGroup);

			// ---
			SCR_InsectSpawnDef spawnDef;
			if (def.m_sSpawnDef != string.Empty)
			{
				Resource holder = BaseContainerTools.LoadContainer(def.m_sSpawnDef);
				if (holder)
					spawnDef = SCR_InsectSpawnDef.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
			}

			if (!spawnDef)
			{
				Print("AMBIENT LIFE: SCR_AmbientInsectsComponent: Missing Spawn Definition Config", LogLevel.WARNING);
				m_AmbientInsectsComponent.RemoveInsectEffect(this);
				
				return;
			}

			m_aSpawnDef.Insert(spawnDef);
		}

		// ---
		SCR_DayTimeCurveDefConfig ambientSoundsDayTimeCurveDefinitionConfig;
		if (m_sDayTimeCurve != string.Empty)
		{
			Resource holder = BaseContainerTools.LoadContainer(m_sDayTimeCurve);
			if (holder)
				ambientSoundsDayTimeCurveDefinitionConfig = SCR_DayTimeCurveDefConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
		}

		if (!ambientSoundsDayTimeCurveDefinitionConfig)
		{
			Print("AMBIENT LIFE: SCR_AmbientInsectsComponent: Missing Daytime Curve Deffinition Config", LogLevel.WARNING);
			m_AmbientInsectsComponent.RemoveInsectEffect(this);
			
			return;
		}

		m_aDayTimeCurve = ambientSoundsDayTimeCurveDefinitionConfig.m_aDayTimeCurve;

		// ---
		SCR_WindCurveDefConfig ambientSoundsWindDefinitionConfig;
		if (m_sWindCurve != string.Empty)
		{
			Resource holder = BaseContainerTools.LoadContainer(m_sWindCurve);
			if (holder)
				ambientSoundsWindDefinitionConfig = SCR_WindCurveDefConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
		}

		if (!ambientSoundsWindDefinitionConfig)
		{
			Print("AMBIENT LIFE: SCR_AmbientInsectsComponent: Missing Wind Curve Definition Config", LogLevel.WARNING);
			m_AmbientInsectsComponent.RemoveInsectEffect(this);
			
			return;
		}

		m_aWindModifier = ambientSoundsWindDefinitionConfig.m_aWindModifier;
	}

	//------------------------------------------------------------------------------------------------
	// Called by SCR_AmbientSoundComponent in OnPostInit()
	override void OnPostInit(SCR_AmbientSoundsComponent ambientSoundsComponent, SCR_AmbientInsectsComponent ambientInsectsComponent, SignalsManagerComponent signalsManagerComponent)
	{
		super.OnPostInit(ambientSoundsComponent, ambientInsectsComponent, signalsManagerComponent);

		LoadConfigs();

		int count = m_aSpawnDef.Count();
		m_aDensity.Resize(count);
		m_aAngleOffset.Resize(count);
		m_aDensityModifier.Resize(count);
	}
}
