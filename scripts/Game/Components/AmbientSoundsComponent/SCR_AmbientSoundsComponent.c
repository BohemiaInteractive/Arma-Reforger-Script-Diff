enum EQueryType // TODO: SCR_EQueryType
{
	TreeBush,
	TreeLeafy,
	TreeLeafyDomestic,
	TreeConifer,
	TreeStump,
	TreeWithered,
	Building,
}

[EntityEditorProps(category: "GameScripted/Sound", description: "THIS IS THE SCRIPT DESCRIPTION.", color: "0 0 255 255")]
class SCR_AmbientSoundsComponentClass : AmbientSoundsComponentClass
{
}

class SCR_AmbientSoundsComponent : AmbientSoundsComponent
{
	[Attribute()]
	ref array<ref SCR_AmbientSoundsEffect> m_aAmbientSoundsEffect;

	// Components
	private SignalsManagerComponent m_LocalSignalsManager;

	// Constants
	private const int QUERY_RADIUS = 25;
	private const int QUERY_PROCESSING_INTERVAL = 2000;
	private const int QUERY_MINIMUM_MOVE_DISTANCE_SQ = 2;
	private const int QUERY_TYPES = 7;
    private const int INVALID = -1;
    const int WINDSPEED_MIN = 2;
    const int WINDSPEED_MAX = 12;
	private const int UPDATE_PROCESSING_INTERVAL = 300;
	private const int LOOPED_SOUND_MINIMUM_MOVE_DISTANCE_SQ = 2;
	
	// Timers
	private float m_fQueryTimer;
	private float m_fUpdateTimer;
	
	// Misc
	private ChimeraWorld m_World;
	private float m_fWorldTime;
	private vector m_vCameraPosFrame;
	private vector m_vCameraPosQuery;
	private vector m_vCameraPosLoopedSound;
	
	// Looped sounds pool
	private ref array<ref SCR_AudioHandleLoop> m_aAudioHandleLoop = {};

	//! Stores entity counds for all EQueryType
	private ref array<int> m_aQueryTypeCount = {};

	//------------------------------------------------------------------------------------------------
	//! Does QueryBySphere and stores all needed data into prepared structures
	private void HandleQueryEntities()
	{
		// Limit processing by time and moved distance
		if (m_fWorldTime < m_fQueryTimer)
			return;
		
		if (vector.DistanceSqXZ(m_vCameraPosQuery, m_vCameraPosFrame) < QUERY_MINIMUM_MOVE_DISTANCE_SQ)
			return;
		
		m_fQueryTimer = m_fWorldTime + QUERY_PROCESSING_INTERVAL;							
		m_vCameraPosQuery = m_vCameraPosFrame;

		// Get new query values		
		QueryAmbientSoundsBySphere(QUERY_RADIUS);
		GetAmbientSoundsCountPerType(m_aQueryTypeCount);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	EQueryType GetDominantTree()
	{
		if (m_aQueryTypeCount[EQueryType.TreeLeafy] + m_aQueryTypeCount[EQueryType.TreeConifer] == 0)
			return INVALID;

		if (m_aQueryTypeCount[EQueryType.TreeLeafy] > m_aQueryTypeCount[EQueryType.TreeConifer])
			return EQueryType.TreeLeafy;

		return EQueryType.TreeConifer;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Uses linear interpolation to get value from curve
	//! \param[in] x Range <0, 1>
	//! \param[in] curve Curve
	//! \return value from curve
	static float GetPoint(float x, Curve curve)
	{				
		if (x <= curve[0][0])
			return curve[0][1];
		
		int lastIdx = curve.Count() - 1;
		
		if (x >= curve[lastIdx][0])
			return curve[lastIdx][1];
		
		int i;		
		for (i = 1; i < lastIdx; i++)
		{
			if (curve[i][0] > x)
				break; 
		}
		
		if (curve[i-1][1] == curve[i][1])
			return curve[i][1];
		else
			return Math.Lerp(curve[i-1][1], curve[i][1], (x - curve[i-1][0]) / (curve[i][0] - curve[i-1][0]));
	}

	//------------------------------------------------------------------------------------------------	
	//! Use to play sound events that has looped banks
	//! \param[in] soundEvent Sound event name
	//! \param[in] transformation Sound position
	//! \return
	SCR_AudioHandleLoop SoundEventLooped(string soundEvent, vector transformation[4])
	{
		SCR_AudioHandleLoop audioHandleLoop = new SCR_AudioHandleLoop;
		
		audioHandleLoop.m_aMat = transformation;
		audioHandleLoop.m_sSoundEvent = soundEvent;
		
		m_aAudioHandleLoop.Insert(audioHandleLoop);

		return audioHandleLoop;
	}
	
	//------------------------------------------------------------------------------------------------	
	//! Use to terminate looped sounds that were triggered using SoundEventLooped()
	//! \param[in] audioHandleLoop
	void TerminateLooped(SCR_AudioHandleLoop audioHandleLoop)
	{
		if (!audioHandleLoop)
			return;
		
		Terminate(audioHandleLoop.m_AudioHandle);
		m_aAudioHandleLoop.RemoveItem(audioHandleLoop);
	}
		
	//------------------------------------------------------------------------------------------------	
	private void UpdateLoopedSounds()
	{
		foreach (SCR_AudioHandleLoop audioHandleLoop : m_aAudioHandleLoop)
		{
			if (IsFinishedPlaying(audioHandleLoop.m_AudioHandle))
				audioHandleLoop.m_AudioHandle = SoundEventTransform(audioHandleLoop.m_sSoundEvent, audioHandleLoop.m_aMat);
		}
	}
	
	//------------------------------------------------------------------------------------------------	
	override void UpdateSoundJob(IEntity owner, float timeSlice)
	{
		super.UpdateSoundJob(owner, timeSlice);
		
		m_vCameraPosFrame = GetCameraOrigin();	
		m_fWorldTime = m_World.GetWorldTime();
		
		// Update effects
		if (m_fWorldTime > m_fUpdateTimer)
		{
			// Handle looped sounds
			if (vector.DistanceSqXZ(m_vCameraPosLoopedSound, m_vCameraPosFrame) > LOOPED_SOUND_MINIMUM_MOVE_DISTANCE_SQ)
			{
				m_vCameraPosLoopedSound = m_vCameraPosFrame;
				UpdateLoopedSounds()
			}

			foreach (SCR_AmbientSoundsEffect ambientSoundsEffect : m_aAmbientSoundsEffect)
			{
				ambientSoundsEffect.Update(m_fWorldTime, m_vCameraPosFrame);
			}
			
			m_fUpdateTimer = m_fWorldTime + UPDATE_PROCESSING_INTERVAL;
		}
		
		HandleQueryEntities();

#ifdef ENABLE_DIAG
		foreach (SCR_AmbientSoundsEffect ambientSoundsEffect : m_aAmbientSoundsEffect)
		{
			ambientSoundsEffect.UpdateDebug(m_fWorldTime);
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_RELOAD_AMBIENT_SOUNDS_CONFIGS))
		{
			foreach (SCR_AmbientSoundsEffect ambientSoundsEffect : m_aAmbientSoundsEffect)
			{
				ambientSoundsEffect.ReloadConfig();
			}
			
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SOUNDS_RELOAD_AMBIENT_SOUNDS_CONFIGS, false);
		}
#endif		
	}
		
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{		
		super.OnPostInit(owner);
		
		// Get world
		m_World = owner.GetWorld();	
		if (!m_World)
		{
			SetScriptedMethodsCall(false);	
			return;
		}

		// Get local signals component		
		m_LocalSignalsManager = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));		
		if (!m_LocalSignalsManager)
		{
			SetScriptedMethodsCall(false);
			Print("AUDIO: SCR_AmbientSoundsComponent: Missing SignalsManagerComponent", LogLevel.WARNING);
			return;
		}

		foreach (SCR_AmbientSoundsEffect ambientSoundsEffect : m_aAmbientSoundsEffect)
		{
			ambientSoundsEffect.OnPostInit(this, m_LocalSignalsManager);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{
		super.OnInit(owner);
				
		foreach (SCR_AmbientSoundsEffect ambientSoundsEffect : m_aAmbientSoundsEffect)
		{
			ambientSoundsEffect.OnInit();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_AmbientSoundsComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SetScriptedMethodsCall(true);
	
		m_aQueryTypeCount.Resize(QUERY_TYPES);
		
#ifdef ENABLE_DIAG
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_RELOAD_AMBIENT_SOUNDS_CONFIGS, "", "Reload AmbientSounds Conf", "Sounds");
#endif	
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_AmbientSoundsComponent()
	{
#ifdef ENABLE_DIAG
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SOUNDS_RELOAD_AMBIENT_SOUNDS_CONFIGS);
#endif	
	}
}
