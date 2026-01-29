enum ESoundMapType
{
	MEADOW,
	FOREST,
	HOUSES,
	SEA,
	POND,
}

//! Spawns sounds arround camera based on define behaviour
//! Basic building blocks are:
//! SCR_SoundGroup defines a group of sounds. E.g. birds playing on trees close to the camera, or insects, or birds playing in the distance
//! SCR_SoundType defines a group of sounds that can be played on a given location or entity. The definition varies based on the used ESpawnMethod
//! SCR_SoundDef defines basic sound behaviour such as the number of repetitions, or daytime/wind behaviour
[BaseContainerProps(configRoot: true)]
class SCR_RandomPositionalSounds : SCR_AmbientSoundsEffect
{
	[Attribute()]
	private ref array<ref SCR_RandomPositionalSoundsDef> m_aRandomPositionalSoundsDef;
	
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "SoundInfo to ambient groups selector config", params: "conf")]
	private ResourceName m_TerrainDefinitionResource;
		
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Daytime curves config", params: "conf")]
	private ResourceName m_DayTimeCurveResource;
	
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Wind modifier curves config", params: "conf")]
	private ResourceName m_WindCurveResource;
	
	// Configuration
	private ref array<ref SCR_SoundGroup> m_aSoundGroup = {};
	private ref array<ref SCR_SpawnDef> m_aSpawnDef = {};
	private ref array<ref SCR_TerrainDef> m_aTerrainDef;
	private ref array<ref SCR_DayTimeCurveDef> m_aDayTimeCurve;
	private ref array<ref SCR_WindCurveDef> m_aWindModifier;

	// Components
	private GameSignalsManager m_GlobalSignalsManager;
	private ChimeraWorld m_World;
	//private SoundWorld m_SoundWorld;
	
	// Constants
	private const int INVALID = -1;
	private const int GLOBAL_MODIFIERS_UPDATE_TIME = 10000;	
	private const string SUN_ANGLE_SIGNAL_NAME = "SunAngle";
	private const string SOUND_NAME_SIGNAL_NAME = "SoundName";
	private const string DISTANCE_SIGNAL_NAME = "Distance";
	private const string RAIN_INTENSITY_SIGNAL_NAME = "RainIntensity";
	private const string WIND_SPEED_SIGNAL_NAME = "WindSpeed";
	private const string ENVIRONMENT_TYPE_SIGNAL_NAME = "EnvironmentType";
	
	// Signal Idxs
	private int m_iSunAngleSignalIdx;
	private int m_iSoundNameSignalIdx;
	private int m_iDistanceSignalIdx;
	private int m_iRainIntensitySignalIdx;
	private int m_iWindSpeedSignalIdx;
	private int m_iEnvironmentTypeSignalIdx;
	
	private int m_iUpdatedSoundGroupIdx;
	private int m_iEnvironmentTypeSignalValue;
	private float m_fGlobalModifiersTimer;
	private ref array<int> m_aDensity = {};
	private ref array<float> m_aDensityModifier = {};
	private ref array<int> m_aAngleOffset = {};
	private float m_aDayTimeCurveValue[DAY_TIME_CURVE_COUNT];
	private float m_aWindCurveValue[WIND_CURVE_COUNT];
	
	private ref array<ref SCR_SoundHandle> m_aSoundHandle = {};

	//------------------------------------------------------------------------------------------------
	ChimeraWorld GetWorld()
	{
		return m_World;
	}
	
	//------------------------------------------------------------------------------------------------	
	ETerrainType GetTerrainType(int soundInfo)
	{		
		foreach(SCR_TerrainDef terrainDef : m_aTerrainDef)
		{
			if (soundInfo >= terrainDef.m_iRangeMin && soundInfo < terrainDef.m_iRangeMax)
				return terrainDef.m_eTerrainType;
		}
		
		return INVALID;		
	}
	
	//------------------------------------------------------------------------------------------------
	// Called by SCR_AmbientSoundComponent in UpdateSoundJob()
	override void Update(float worldTime, vector cameraPos)
	{
		// Get environment type
		m_iEnvironmentTypeSignalValue = m_LocalSignalsManager.GetSignalValue(m_iEnvironmentTypeSignalIdx);
		
		ProcessSoundGroups(worldTime, cameraPos);
		UpdateSoundSequence(worldTime, cameraPos);
		
		if (worldTime > m_fGlobalModifiersTimer)
		{
			UpdateGlobalModifiers();
			RemoveSoundsBasedOnTimeAndWeather();
			m_fGlobalModifiersTimer = worldTime + GLOBAL_MODIFIERS_UPDATE_TIME;
		}
	}
	
#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------		
	override void UpdateDebug(float worldTime)	
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_RANDOM_POSITIONAL_SOUNDS))
		{
			AmbientSignalsDebug();
			RandomPositionalSoundDebug(worldTime);
			DensityDebug();				
			PlaySound(worldTime);
		}	
	}
#endif	
	
	//------------------------------------------------------------------------------------------------		
	private void ProcessSoundGroups(float worldTime, vector cameraPos)
	{				
		UpdateSoundGroup(worldTime, cameraPos, m_iUpdatedSoundGroupIdx);
		
		m_iUpdatedSoundGroupIdx ++;		
		if (m_iUpdatedSoundGroupIdx < m_aSoundGroup.Count())
			return;
		
		m_iUpdatedSoundGroupIdx = 0;
		
		RemoveOutOfRangeSound(cameraPos);		
		UpdateDensity();									
	}
	
	//------------------------------------------------------------------------------------------------	
	//! Loops through all playing sounds and removes the ones that do not pass time and weather check
	private void RemoveSoundsBasedOnTimeAndWeather()
	{
		for (int i = m_aSoundHandle.Count() - 1; i >= 0; i--)
		{
			SCR_SoundHandle soundHandle = m_aSoundHandle[i];
			
			if (m_aDayTimeCurveValue[soundHandle.m_SoundDef.m_eDayTimeCurve] == 0 || m_aDensityModifier[soundHandle.m_iSoundGroup] == 0)
				m_aSoundHandle.Remove(i);
		}
	}
	
	//------------------------------------------------------------------------------------------------	
	//! Updates wind, rain and time modifiers based on current WindSpeed, RainIntensity and SunAngle
	private void UpdateGlobalModifiers()
	{							
		//Update values of wind modifiers curves
		float windSpeedScaled = Interpolate01(m_GlobalSignalsManager.GetSignalValue(m_iWindSpeedSignalIdx), SCR_AmbientSoundsComponent.WINDSPEED_MIN, SCR_AmbientSoundsComponent.WINDSPEED_MAX);			
		for (int i = 0; i < WIND_CURVE_COUNT; i++)
		{
			m_aWindCurveValue[i] = SCR_AmbientSoundsComponent.GetPoint(windSpeedScaled, m_aWindModifier[i].m_Curve);
		}

		float timeOfDay = m_LocalSignalsManager.GetSignalValue(m_iSunAngleSignalIdx) / 360;
					
		foreach (int i, SCR_SpawnDef spawnDef : m_aSpawnDef)
		{
			float modifier = SCR_AmbientSoundsComponent.GetPoint(timeOfDay, spawnDef.m_TimeModifier);
			modifier = modifier * SCR_AmbientSoundsComponent.GetPoint(m_GlobalSignalsManager.GetSignalValue(m_iRainIntensitySignalIdx), spawnDef.m_RainModifier);
			m_aDensityModifier[i] = modifier * m_aWindCurveValue[spawnDef.m_eWindModifier];
		}
		
		// Update values of day time curves
		for (int i = 0; i < DAY_TIME_CURVE_COUNT; i++)
		{
			m_aDayTimeCurveValue[i] = SCR_AmbientSoundsComponent.GetPoint(timeOfDay, m_aDayTimeCurve[i].m_Curve);
		}
	}
	
	//------------------------------------------------------------------------------------------------	
	private int GetDensityMax(int soundGroup)
	{
		if (m_iEnvironmentTypeSignalValue == EEnvironmentType.MEADOW)
			return m_aSpawnDef[soundGroup].m_iMeadowDensityMax;
		else if (m_iEnvironmentTypeSignalValue == EEnvironmentType.FOREST)
			return m_aSpawnDef[soundGroup].m_iForestDensityMax;		
		else if (m_iEnvironmentTypeSignalValue == EEnvironmentType.HOUSES)
			return m_aSpawnDef[soundGroup].m_iHousesDensityMax;
		else
			return m_aSpawnDef[soundGroup].m_iSeaDensityMax;
	}
	
	//------------------------------------------------------------------------------------------------	
	private void UpdateSoundSequence(float worldTime, vector cameraPos)
	{	
		for (int i = m_aSoundHandle.Count() - 1; i >= 0; i--)
		{
			SCR_SoundHandle soundHandle = m_aSoundHandle[i];
			
			if (worldTime > soundHandle.m_aRepTime[soundHandle.m_iRepTimeIdx])
			{
				// Set SoundName signal
				if (m_iSoundNameSignalIdx != INVALID)
					m_LocalSignalsManager.SetSignalValue(m_iSoundNameSignalIdx, soundHandle.m_SoundDef.m_eSoundName);
																																																			
				// Set Distance signal
				if (m_iDistanceSignalIdx != INVALID)
					m_LocalSignalsManager.SetSignalValue(m_iDistanceSignalIdx, vector.Distance(cameraPos, soundHandle.m_aMat[3]));
				
				// Play sound event	
				m_AmbientSoundsComponent.SoundEventTransform(m_aSoundGroup[soundHandle.m_iSoundGroup].m_sSoundEvent, soundHandle.m_aMat);

#ifdef ENABLE_DIAG				
				soundHandle.m_SoundDef.m_iCount++;
#endif
							
				// Remove if last event in the sequence	
				soundHandle.m_iRepTimeIdx ++;										
				if (soundHandle.m_aRepTime.Count() == soundHandle.m_iRepTimeIdx)
					m_aSoundHandle.Remove(i);											
			}						
		}
	}

	//------------------------------------------------------------------------------------------------			
	private void RemoveOutOfRangeSound(vector cameraPos)
	{				
		for (int i = m_aSoundHandle.Count() - 1; i >= 0; i--)
		{
			SCR_SoundHandle soundHandle = m_aSoundHandle[i];
			SCR_SpawnDef spawnDef = m_aSpawnDef[soundHandle.m_iSoundGroup];
			
			if (!Math.IsInRange(vector.Distance(cameraPos, soundHandle.m_aMat[3]), spawnDef.m_iPlayDistMin, spawnDef.m_iPlayDistMax))					
				m_aSoundHandle.Remove(i);							
		}		
	}

	//------------------------------------------------------------------------------------------------	
	//! Attempts to find new position to play sound based on defined ESpawnMethod
	//! \param[in] worldTime
	//! \param[in] camPos
	//! \param[in] soundGroup
	private void UpdateSoundGroup(float worldTime, vector camPos, int soundGroup)
	{
		// Check if density modified by rain, wind and time of day was reached
		if (m_aDensity[soundGroup] >= GetDensityMax(soundGroup) * m_aDensityModifier[soundGroup])
			return;
		
		// Generate sound position		
		vector vPos;								
		int soundType = INVALID;
				
		m_aSoundGroup[soundGroup].GetSoundTypeAndPosition(this, camPos, m_aSpawnDef[soundGroup].m_iSpawnDist, soundType, vPos);
				
		if (soundType == INVALID)
			return;
									
		// Get random sound definition
		int soundDef = GetRandomSoundDef(m_aSoundGroup[soundGroup].m_aSoundType[soundType], worldTime);
						
		if (soundDef == INVALID)
			return;
						
		// Create sound event sequence					
		vector mat[4];
		Math3D.MatrixIdentity4(mat);
		mat[3] = vPos;					
				
		SCR_SoundHandle soundHandle = new SCR_SoundHandle(soundGroup, soundType, soundDef, mat, m_aSoundGroup, worldTime);
		m_aSoundHandle.Insert(soundHandle);
													
		// Set CoolDownEnd time
		float timeOfDayFactor = m_aDayTimeCurveValue[soundHandle.m_SoundDef.m_eDayTimeCurve];								
		timeOfDayFactor = (1 - Math.Clamp(timeOfDayFactor, 0, 1));
		timeOfDayFactor = Math.Pow(timeOfDayFactor, 3) * 4 + 1;
											
		if (soundHandle.m_SoundDef.m_bAddSequenceLength)
			soundHandle.m_SoundDef.m_fCoolDownEnd = worldTime + soundHandle.GetSequenceLenght(worldTime) + soundHandle.m_SoundDef.m_iCoolDown * timeOfDayFactor;
		else
			soundHandle.m_SoundDef.m_fCoolDownEnd = worldTime + soundHandle.m_SoundDef.m_iCoolDown * timeOfDayFactor;							
	}
	
	//------------------------------------------------------------------------------------------------						
	private int GetRandomSoundDef(SCR_SoundType soundType, float worldTime)
	{
		int size = soundType.m_aSoundDef.Count();
		if (size == 0)
			return INVALID;
		
		int offset = Math.RandomIntInclusive(0, size - 1);
		
		for (int i = 0; i < size; i++)
		{
			int idx = i + offset;
			if (idx >= size)
				idx -= size;
					
			if (m_aDayTimeCurveValue[soundType.m_aSoundDef[idx].m_eDayTimeCurve] > 0 && soundType.m_aSoundDef[idx].m_fCoolDownEnd < worldTime)
				return idx;
		}			
		
		return INVALID;
	}

	//------------------------------------------------------------------------------------------------
	private void UpdateDensity()
	{		
		foreach (SCR_SoundHandle soundHandle : m_aSoundHandle)
		{
			m_aDensity[soundHandle.m_iSoundGroup] = m_aDensity[soundHandle.m_iSoundGroup] + soundHandle.m_fDensity;
		}
		
		for (int i = 0, count = m_aSpawnDef.Count(); i < count; i++)
		{
			m_aDensity[i] = m_aDensity[i] * 0.5;
		}
	}

	//------------------------------------------------------------------------------------------------	
	private float Interpolate01(float in, float Xmin, float Xmax)
	{
		if (in <= Xmin)
			return 0;
		
		if (in >= Xmax)
			return 1;
		
		return (in - Xmin) / (Xmax - Xmin);
	}

	//------------------------------------------------------------------------------------------------	
	private void UpdateSignlsIdx()
	{
		m_iSunAngleSignalIdx = m_LocalSignalsManager.AddOrFindSignal(SUN_ANGLE_SIGNAL_NAME);
		m_iSoundNameSignalIdx = m_LocalSignalsManager.AddOrFindSignal(SOUND_NAME_SIGNAL_NAME);
		m_iDistanceSignalIdx = m_LocalSignalsManager.AddOrFindSignal(DISTANCE_SIGNAL_NAME);
		m_iEnvironmentTypeSignalIdx = m_LocalSignalsManager.AddOrFindSignal(ENVIRONMENT_TYPE_SIGNAL_NAME);
		
		m_iRainIntensitySignalIdx = m_GlobalSignalsManager.AddOrFindSignal(RAIN_INTENSITY_SIGNAL_NAME);
		m_iWindSpeedSignalIdx = m_GlobalSignalsManager.AddOrFindSignal(WIND_SPEED_SIGNAL_NAME);	
	}

	//------------------------------------------------------------------------------------------------
	private void LoadConfigs(SCR_AmbientSoundsComponent ambientSoundsComponent)
	{
		foreach (SCR_RandomPositionalSoundsDef def: m_aRandomPositionalSoundsDef)
		{
			SCR_SoundGroup soundGrop;
			if (def.m_SoundGroupResource != string.Empty)
			{			
				Resource holder = BaseContainerTools.LoadContainer(def.m_SoundGroupResource);
				if (holder)
					soundGrop = SCR_SoundGroup.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));	
			}
			
			if (!soundGrop)
			{
				ambientSoundsComponent.SetScriptedMethodsCall(false);
				Print("AUDIO: SCR_AmbientSoundsComponent: Missing Ambience Sound Gropu Config", LogLevel.WARNING);
				return;
			}
						
			m_aSoundGroup.Insert(soundGrop);
			
			// ---
			SCR_SpawnDef spawnDef;
			if (def.m_SpawnDefResource != string.Empty)
			{			
				Resource holder = BaseContainerTools.LoadContainer(def.m_SpawnDefResource);
				if (holder)
					spawnDef = SCR_SpawnDef.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));	
			}
			
			if (!spawnDef)
			{
				ambientSoundsComponent.SetScriptedMethodsCall(false);
				Print("AUDIO: SCR_AmbientSoundsComponent: Missing Spawn Definition Config", LogLevel.WARNING);
				return;
			}
			
			m_aSpawnDef.Insert(spawnDef);
		}
		
		// ---
		SCR_TerrainDefConfig soundTerrainDefinitionConfig;
		if (m_TerrainDefinitionResource != string.Empty)
		{
			Resource holder = BaseContainerTools.LoadContainer(m_TerrainDefinitionResource);
			if (holder)
				soundTerrainDefinitionConfig = SCR_TerrainDefConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
		}			

		if (!soundTerrainDefinitionConfig)
		{
			ambientSoundsComponent.SetScriptedMethodsCall(false);
			Print("AUDIO: SCR_AmbientSoundsComponent: Missing Sound Definition Terrain Config", LogLevel.WARNING);
			return;
		}
		
		m_aTerrainDef = soundTerrainDefinitionConfig.m_aTerrainDef;

		// ---
		SCR_DayTimeCurveDefConfig ambientSoundsDayTimeCurveDefinitionConfig;
		if (m_DayTimeCurveResource != string.Empty)
		{			
			Resource holder = BaseContainerTools.LoadContainer(m_DayTimeCurveResource);
			if (holder)
				ambientSoundsDayTimeCurveDefinitionConfig = SCR_DayTimeCurveDefConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));	
		}
		
		if (!ambientSoundsDayTimeCurveDefinitionConfig)
		{
			ambientSoundsComponent.SetScriptedMethodsCall(false);
			Print("AUDIO: SCR_AmbientSoundsComponent: Missing Daytime Curve Deffinition Config", LogLevel.WARNING);
			return;
		}
		
		m_aDayTimeCurve = ambientSoundsDayTimeCurveDefinitionConfig.m_aDayTimeCurve;
		
		// ---
		SCR_WindCurveDefConfig ambientSoundsWindDefinitionConfig;
		if (m_WindCurveResource != string.Empty)
		{			
			Resource holder = BaseContainerTools.LoadContainer(m_WindCurveResource);
			if (holder)
				ambientSoundsWindDefinitionConfig = SCR_WindCurveDefConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));	
		}
		
		if (!ambientSoundsWindDefinitionConfig)
		{
			ambientSoundsComponent.SetScriptedMethodsCall(false);
			Print("AUDIO: SCR_AmbientSoundsComponent: Missing Wind Curve Definition Config", LogLevel.WARNING);
			return;
		}
		
		m_aWindModifier = ambientSoundsWindDefinitionConfig.m_aWindModifier;
	}
		
	//------------------------------------------------------------------------------------------------
#ifdef ENABLE_DIAG
	private const int STRING_LENGTH = 20;
	private const int PANEL_WIDTH = 20;
	private const int PANEL_HEIGHT = 20;
	
	ref array<bool> m_soundGroupCheck;
	ref array<ref array<bool>> m_soundTypeCheck;
	
	//------------------------------------------------------------------------------------------------	
	//!
	void AmbientSignalsDebug()
	{	
		DbgUI.Begin("Ambient Sounds Signals", 178, 0);					
		DbgUI.Text(SUN_ANGLE_SIGNAL_NAME + ": " + m_LocalSignalsManager.GetSignalValue(m_iSunAngleSignalIdx).ToString());
		DbgUI.Text(SOUND_NAME_SIGNAL_NAME + ": " + m_LocalSignalsManager.GetSignalValue(m_iSoundNameSignalIdx).ToString());
		DbgUI.Text(DISTANCE_SIGNAL_NAME + ": " + m_LocalSignalsManager.GetSignalValue(m_iDistanceSignalIdx).ToString());		
		DbgUI.Text("Environent Type : " + typename.EnumToString(EEnvironmentType, m_iEnvironmentTypeSignalValue));		
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------	
	//!
	void DensityDebug()
	{	
		DbgUI.Begin("Density", 420, 0);							
		for (int i = 0, count = m_aSpawnDef.Count(); i < count; i++)
		{
			DbgUI.Text(m_aSoundGroup[i].m_sSoundEvent + ": " + m_aDensity[i].ToString() + " / " + (Math.Round(GetDensityMax(i) * m_aDensityModifier[i])).ToString());
		}
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] worldTime
	void RandomPositionalSoundDebug(float worldTime)
	{				
		int defaultColor = ARGB(255, 50, 50, 50);
		int blackColor = ARGB(255, 0, 0, 0);
		int availableColor = ARGB(255, 0, 0, 255);
		int playingColor = ARGB(255, 255, 255, 0);
		int wrongTimeColor = ARGB(255, 125, 125, 125);
		int inCooldownColor = ARGB(255, 255, 0, 255);
		
		DbgUI.Begin("Random Ambient Sounds", 0, 185);	
		
		DbgUI.Panel("av", PANEL_WIDTH, PANEL_HEIGHT, availableColor);
		DbgUI.SameLine();
		DbgUI.Text("Available");
		DbgUI.SameLine();
		DbgUI.Panel("pl", PANEL_WIDTH, PANEL_HEIGHT, playingColor);
		DbgUI.SameLine();
		DbgUI.Text("Playing");	
		DbgUI.SameLine();
		DbgUI.Panel("wt", PANEL_WIDTH, PANEL_HEIGHT, wrongTimeColor);
		DbgUI.SameLine();
		DbgUI.Text("WrongTime");	
		DbgUI.SameLine();
		DbgUI.Panel("in", PANEL_WIDTH, PANEL_HEIGHT, inCooldownColor);
		DbgUI.SameLine();
		DbgUI.Text("InCooldown");
		
		// Prepare arrays for check widgets
		if (!m_soundGroupCheck)
		{
			m_soundGroupCheck = {};
			m_soundGroupCheck.Resize(m_aSpawnDef.Count());
		}
		if (!m_soundTypeCheck)
		{
			m_soundTypeCheck = {};
			m_soundTypeCheck.Resize(m_aSpawnDef.Count());
			
			for (int soundGroup = 0; soundGroup < m_aSpawnDef.Count(); soundGroup++)
			{
				m_soundTypeCheck[soundGroup] = {};
				int soundTypeCount = m_aSoundGroup[soundGroup].m_aSoundType.Count();						
				m_soundTypeCheck[soundGroup].Resize(soundTypeCount);
			}
		}
				
		for (int soundGroup = 0; soundGroup < m_aSpawnDef.Count(); soundGroup++)
		{				
			// Get SoundGropu name
			string name = m_aSoundGroup[soundGroup].m_sSoundEvent;
			name = NormalizeLength(name);			
			name = soundGroup.ToString() + ". " + name;
			
			// Show number of playing sounds
			const int limitCount = 8; // TODO: check for good const usage
			int playingCount = 0;
			
			bool bSoundTypeChecked = m_soundGroupCheck[soundGroup];					
			DbgUI.Check(name, bSoundTypeChecked);
			m_soundGroupCheck[soundGroup] = bSoundTypeChecked;
			
			for (int i = 0; i < m_aSoundHandle.Count(); i++)
			{
				if (m_aSoundHandle[i].m_iSoundGroup == soundGroup)
					playingCount ++;
			}
			
			for (int i = 0; i < limitCount; i++)
			{
				int color = defaultColor;
				
				if (i < playingCount)
				{
					color = playingColor;
				}
				
				DbgUI.SameLine();		
				DbgUI.Panel(soundGroup.ToString() + "." + i.ToString(), PANEL_WIDTH, PANEL_HEIGHT, color);
			}
				
			if (bSoundTypeChecked)
			{					
				int soundTypeCount = m_aSoundGroup[soundGroup].m_aSoundType.Count();
								
						
				for (int soundType = 0; soundType < soundTypeCount; soundType++)
				{									
					// Get SoundType name
					string soundTypeName;
					if (SCR_TreeSoundGroup.Cast(m_aSoundGroup[soundGroup]))
					{
						if (soundType == 0)
							soundTypeName = "CONIFER";
						else
							soundTypeName = "LEAFY";	
					}
					else if (SCR_TerrainSoundGroup.Cast(m_aSoundGroup[soundGroup]))
						soundTypeName = typename.EnumToString(ETerrainType, soundType);
					else
						soundTypeName = typename.EnumToString(ESoundMapType, soundType);
									
					soundTypeName = NormalizeLength(soundTypeName);
					
					DbgUI.Panel(soundGroup.ToString() + soundType.ToString(), PANEL_WIDTH, PANEL_HEIGHT, blackColor);
					DbgUI.SameLine();
					
					bool bSoundGroupChecked = m_soundTypeCheck[soundGroup][soundType];
					DbgUI.Check(soundGroup.ToString() + "." + soundType.ToString() + ". " + soundTypeName, bSoundGroupChecked);
					m_soundTypeCheck[soundGroup][soundType] = bSoundGroupChecked;
					
					DbgUI.SameLine();							
					
					int size = m_aSoundGroup[soundGroup].m_aSoundType[soundType].m_aSoundDef.Count();					
					int available = 0;
					for (int idx = 0; idx < size; idx++)
					{			
						if (m_aDayTimeCurveValue[m_aSoundGroup[soundGroup].m_aSoundType[soundType].m_aSoundDef[idx].m_eDayTimeCurve] > 0 && m_aSoundGroup[soundGroup].m_aSoundType[soundType].m_aSoundDef[idx].m_fCoolDownEnd < worldTime)
							available++;
					}			

					DbgUI.Text(soundGroup.ToString() + "." + soundType.ToString() + ". " + "Available :" + available.ToString() + " of (" + m_aSoundGroup[soundGroup].m_aSoundType[soundType].m_aSoundDef.Count() + ")");					
					
					if (bSoundGroupChecked)
					{
						int soundDefCount = m_aSoundGroup[soundGroup].m_aSoundType[soundType].m_aSoundDef.Count();
						
						for (int soundDef = 0; soundDef < soundDefCount; soundDef++)
						{
							int color = defaultColor;
							string aditionalInfo = "";
							
							// -----------------------
							// -----Status panel------
							// -----------------------
														
							// Tag if sound is in cooldown
							if (m_aSoundGroup[soundGroup].m_aSoundType[soundType].m_aSoundDef[soundDef].m_fCoolDownEnd > worldTime)
							{
								color = inCooldownColor;
								aditionalInfo = " | Coldown ends in : " + Math.Round((m_aSoundGroup[soundGroup].m_aSoundType[soundType].m_aSoundDef[soundDef].m_fCoolDownEnd - worldTime) * 0.001).ToString();
								
								float timeOfDayFactor = m_aDayTimeCurveValue[m_aSoundGroup[soundGroup].m_aSoundType[soundType].m_aSoundDef[soundDef].m_eDayTimeCurve];
								timeOfDayFactor = (1 - Math.Clamp(timeOfDayFactor, 0, 1));
								timeOfDayFactor = Math.Pow(timeOfDayFactor, 3) * 4 + 1;
								timeOfDayFactor = Math.Round(timeOfDayFactor * 100) * 0.01;
								
								aditionalInfo = aditionalInfo + " | TFactor : " + timeOfDayFactor.ToString();							
							}
							
							EDayTimeCurve m_eDayTimeCurve = m_aSoundGroup[soundGroup].m_aSoundType[soundType].m_aSoundDef[soundDef].m_eDayTimeCurve;						
													
							// Tag sounds that can not play in given time of day
							if (m_aDayTimeCurveValue[m_eDayTimeCurve] <= 0)
								color = wrongTimeColor;
							
							// Tag available
							if (m_aDayTimeCurveValue[m_eDayTimeCurve] > 0 && m_aSoundGroup[soundGroup].m_aSoundType[soundType].m_aSoundDef[soundDef].m_fCoolDownEnd <= worldTime)
								color = availableColor;
							
							DbgUI.Panel(soundGroup.ToString() + "." + soundType.ToString() + "." + soundDef.ToString(), PANEL_WIDTH * 2 + 5, PANEL_HEIGHT, color);
							
							// -----------------------
							// -Sound definition name-
							// -----------------------
							
							string soundDefinitionName = typename.EnumToString(ESoundName, m_aSoundGroup[soundGroup].m_aSoundType[soundType].m_aSoundDef[soundDef].m_eSoundName);
							soundDefinitionName = NormalizeLength(soundDefinitionName);							
							DbgUI.SameLine();				
							DbgUI.Text(soundGroup.ToString() + "." + soundType.ToString() + "." + soundDef.ToString() + ". " + soundDefinitionName + " ");
							
							// -----------------------
							// - Number of instances -
							// -----------------------
							
							int instanceCount = 0;
							float playbackEnd = 0;
							
							foreach (SCR_SoundHandle sh: m_aSoundHandle)
							{
								if (sh.m_iSoundGroup == soundGroup && sh.m_iSoundType == soundType && sh.m_iSoundDef == soundDef)
								{
									instanceCount ++;
									
									if (playbackEnd < Math.Round(sh.m_aRepTime[sh.m_aRepTime.Count() - 1] - worldTime))
									{
										playbackEnd = Math.Round(sh.m_aRepTime[sh.m_aRepTime.Count() - 1] - worldTime);
									}
								}
							}
							
							playbackEnd = Math.Round(playbackEnd * 0.001);
							
							if (playbackEnd > 0)
							{
								aditionalInfo = aditionalInfo + " | Playback ends in : " + playbackEnd.ToString();
							}
							
							for (int j = 0; j < instanceCount; j++)
							{
								DbgUI.SameLine();
								DbgUI.Panel(soundGroup.ToString() + "." + soundType.ToString() + "." + soundDef.ToString() + "Play" + j.ToString(), PANEL_WIDTH, PANEL_HEIGHT, playingColor);
							}
							
							// -----------------------
							// -----Aditional info----
							// -----------------------
							
							DbgUI.SameLine();																
							DbgUI.Text(aditionalInfo);																		
						}						
					}					
				}
			}
		}
		
		DbgUI.End();
	}

	//------------------------------------------------------------------------------------------------
	private string NormalizeLength(string name)
	{
		int length = name.Length();
		string s = name;
			
		if (length <= STRING_LENGTH)
		{
			for (int i = 0; i < (STRING_LENGTH - length); i++)
			{
				s = s + " ";
			}
		}
		else
			s = s.Substring(0, STRING_LENGTH * 0.5 - 2) + "...." + s.Substring(length - STRING_LENGTH * 0.5, STRING_LENGTH * 0.5);
				
		return s;
	}
		
	//------------------------------------------------------------------------------------------------
	override void ReloadConfig()
	{
		super.ReloadConfig();
		
		m_aSoundGroup.Clear();
		m_aSpawnDef.Clear();
		
		foreach (SCR_RandomPositionalSoundsDef def: m_aRandomPositionalSoundsDef)
		{
			SCR_SoundGroup soundGrop;
			if (def.m_SoundGroupResource != string.Empty)
			{			
				Resource holder = BaseContainerTools.LoadContainer(def.m_SoundGroupResource);
				if (holder)
					soundGrop = SCR_SoundGroup.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));	
			}
			
			if (soundGrop)						
				m_aSoundGroup.Insert(soundGrop);
			
			// ---
			SCR_SpawnDef spawnDef;
			if (def.m_SpawnDefResource != string.Empty)
			{			
				Resource holder = BaseContainerTools.LoadContainer(def.m_SpawnDefResource);
				if (holder)
					spawnDef = SCR_SpawnDef.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));	
			}
			
			if (spawnDef)	
				m_aSpawnDef.Insert(spawnDef);
		}
		
		// ---
		SCR_TerrainDefConfig soundTerrainDefinitionConfig;
		if (m_TerrainDefinitionResource != string.Empty)
		{
			Resource holder = BaseContainerTools.LoadContainer(m_TerrainDefinitionResource);
			if (holder)
				soundTerrainDefinitionConfig = SCR_TerrainDefConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
		}			

		if (soundTerrainDefinitionConfig)
			m_aTerrainDef = soundTerrainDefinitionConfig.m_aTerrainDef;

		// ---
		SCR_DayTimeCurveDefConfig ambientSoundsDayTimeCurveDefinitionConfig;
		if (m_DayTimeCurveResource != string.Empty)
		{			
			Resource holder = BaseContainerTools.LoadContainer(m_DayTimeCurveResource);
			if (holder)
				ambientSoundsDayTimeCurveDefinitionConfig = SCR_DayTimeCurveDefConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));	
		}
		
		if (!ambientSoundsDayTimeCurveDefinitionConfig)
			m_aDayTimeCurve = ambientSoundsDayTimeCurveDefinitionConfig.m_aDayTimeCurve;
		
		// ---
		SCR_WindCurveDefConfig ambientSoundsWindDefinitionConfig;
		if (m_WindCurveResource != string.Empty)
		{			
			Resource holder = BaseContainerTools.LoadContainer(m_WindCurveResource);
			if (holder)
				ambientSoundsWindDefinitionConfig = SCR_WindCurveDefConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));	
		}
		
		if (ambientSoundsWindDefinitionConfig)
			m_aWindModifier = ambientSoundsWindDefinitionConfig.m_aWindModifier;
		
		m_aSoundHandle.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	private void PlaySound(float worldTime)
	{
			DbgUI.Begin("Play Sound", 0, 0);
		
			int soundGroup = 3;
			int soundType = 2;
			int soundDef = 0;
			float distanceFactor = 0.15;
			DbgUI.InputInt("Sound Group:", soundGroup, 50);
			DbgUI.InputInt("Sound Type :", soundType, 50);
			DbgUI.InputInt("Sound Def  :", soundDef, 50);
			DbgUI.InputFloat("Dist Factor:", distanceFactor, 50);
			
			bool play = DbgUI.Button("Play"); 	
			DbgUI.SameLine();
			bool clear = DbgUI.Button("Clear");
			
			if (play)
			{
				// Check if sound definition is correct				
				if (soundGroup >= m_aSoundGroup.Count() || soundType >= m_aSoundGroup[soundGroup].m_aSoundType.Count() || soundDef >= m_aSoundGroup[soundGroup].m_aSoundType[soundType].m_aSoundDef.Count())
					return;
				
				// Get camera transf
				vector mat[4];
				GetGame().GetCameraManager().CurrentCamera().GetTransform(mat);
				
				// Get direction
				vector yaw = mat[0];
				float angle = yaw.ToYaw();
				angle -= 90;
				yaw = vector.FromYaw(angle);
				
				// Get sound position
				float dist =  Math.Lerp(m_aSpawnDef[soundGroup].m_iPlayDistMin, m_aSpawnDef[soundGroup].m_iPlayDistMax, distanceFactor);
				mat[3] = mat[3] + yaw * dist;		
				
				// Add sound to pool 
				SCR_SoundHandle soundHandle = new SCR_SoundHandle(soundGroup, soundType, soundDef, mat, m_aSoundGroup, worldTime);
				m_aSoundHandle.Insert(soundHandle);				
			}
			
			if (clear)
				m_aSoundHandle.Clear();
		
			DbgUI.End();
	}
	
#endif

	//------------------------------------------------------------------------------------------------		
	override void OnPostInit(SCR_AmbientSoundsComponent ambientSoundsComponent, SignalsManagerComponent signalsManagerComponent)
	{
		super.OnPostInit(ambientSoundsComponent, signalsManagerComponent);
				
		LoadConfigs(ambientSoundsComponent);
			
		m_LocalSignalsManager = signalsManagerComponent;
		m_GlobalSignalsManager = GetGame().GetSignalsManager();
		m_AmbientSoundsComponent = ambientSoundsComponent;

		int count = m_aSpawnDef.Count();		
		m_aDensity.Resize(count);
		m_aAngleOffset.Resize(count);
		m_aDensityModifier.Resize(count);
	
		UpdateSignlsIdx();
		m_World = ChimeraWorld.CastFrom(GetGame().GetWorld());
		
#ifdef ENABLE_DIAG
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_RANDOM_POSITIONAL_SOUNDS, "", "Random Ambient Sounds", "Sounds");
#endif		
	}
	
	//------------------------------------------------------------------------------------------------	
	override void OnInit()
	{
		super.OnInit();
		
		UpdateGlobalModifiers();
	}

	//------------------------------------------------------------------------------------------------		
	// destructor
	void ~SCR_RandomPositionalSounds()
	{
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_RANDOM_POSITIONAL_SOUNDS))
		{
			foreach (SCR_SoundGroup soundGroup : m_aSoundGroup)
			{
				foreach (SCR_SoundType soundType : soundGroup.m_aSoundType)
				{
					foreach (SCR_SoundDef soundDef : soundType.m_aSoundDef)
					{
						Print(typename.EnumToString(ESoundName, soundDef.m_eSoundName) + ": " + soundDef.m_iCount, LogLevel.NORMAL);
					}
				}
			}
		}
		
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SOUNDS_RANDOM_POSITIONAL_SOUNDS);
#endif	
	}
}
