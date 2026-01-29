/*!
SCR_SoundManagerModule is created for playing simple one-shot sounds without need of SoundComponent on given entity.
SCR_SoundManagerModule is not present on console app.
If functions in SCR_SoundManagerModule are enough for given sound, prioritize it befor adding SoundComponent on the entity.
If entity already has SoundComponent because of some other sound, do not use SCR_SoundManagerModule.
Do not use SCR_SoundManagerModule for UI sounds.
Do not use SCR_SoundManagerModule for managing looped sounds. Looped sounds always need SoundComponent to work properly
Signals for given sound can be set only before the playback and is not possible to update them during the sound playback.
*/

class SCR_SoundManagerModule : SndBaseModule
{	
	//! Stores all playing AudioSources	with dynamic update
	private ref array<ref SCR_AudioSource> m_aAudioSource = {};
	//! SoundWorld
	private SoundWorld m_SoundWorld;
	
	//! Global signal names
	static const string DYNAMIC_RANGE_SIGNAL_NAME = "DynamicRange";
	static const string G_TINNITUS_SIGNAL_NAME = "GTinnitus";
	static const string IN_EDITOR_SIGNAL_NAME = "InEditor";
	
	//! Global signal indexes
	private static int s_iDynamicRangeIdx;
	private static int s_iGTinnitusIdx;
	private static int s_iInEditorIdx;

#ifdef ENABLE_DIAG
	private static int s_iCreatedAudioSources;
	private static int s_iMaxActiveAudioSources;
	private static int s_iPlayedAudioSources;
	private static int s_iInvalidAudioSources;
	private static int s_iInaudibleAudioSources;
	private static int s_iTerminateAudioSourceCalls;
	private static int s_iCalculateInterirorAtCount;
#endif
	
	//------------------------------------------------------------------------------------------------	
	/*!
	When creating AudioSource, basic distance and loudness check is performed to establish, if sound would be audible or not
	Use when having SCR_SoundDataComponent on the owner entity
	\param owner Entity soundEvent is linked to
	\param eventName is sound event to play
	*/
	SCR_AudioSource CreateAudioSource(notnull IEntity owner, string eventName)
	{		
		// Get SCR_AudioSourceConfiguration prefab data
		SCR_SoundDataComponent soundDataComponent = SCR_SoundDataComponent.Cast(owner.FindComponent(SCR_SoundDataComponent));
		if (!soundDataComponent)
			return null;
		
		SCR_AudioSourceConfiguration audioSourceConfiguration = soundDataComponent.GetAudioSourceConfiguration(eventName);
		if (!audioSourceConfiguration)
			return null;
					
		return CreateAudioSource(owner, audioSourceConfiguration);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	When creating AudioSource, basic distance and loudness check is performed to establish, if sound would be audible or not
	Use, when you have custom audioSourceConfiguration
	\param owner Entity soundEvent is linked to
	\param audioSourceConfiguration Custom audio source configuration
	*/
	SCR_AudioSource CreateAudioSource(notnull IEntity owner, SCR_AudioSourceConfiguration audioSourceConfiguration)
	{		
#ifdef ENABLE_DIAG
		s_iCreatedAudioSources++;
#endif
		
		// Is sound source in audible range?
		vector mat[4];
		owner.GetTransform(mat);
			
		// Has offset
		if (audioSourceConfiguration.HasOffset())
		{
			mat[3] = owner.CoordToParent(audioSourceConfiguration.m_vOffset);
		}
					
		float distance = AudioSystem.IsAudible(audioSourceConfiguration.m_sSoundProject, audioSourceConfiguration.m_sSoundEventName, mat[3]);
		if (distance < 0)
		{
#ifdef ENABLE_DIAG
			s_iInaudibleAudioSources++;
#endif
			
			return null;
		}
		
		// Create audio source
		SCR_AudioSource audioSource = new SCR_AudioSource(audioSourceConfiguration, mat);
		
		// Set distance signal
		audioSource.SetSignalValue(SCR_AudioSource.DISTANCE_SINAL_NAME, distance);
		
		// Set owner
		audioSource.m_Owner = owner;
						
		return audioSource;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	When creating AudioSource, basic distance and loudness check is performed to establish, if sound would be audible or not
	Use, when you have custom audioSourceConfiguration
	Use, when you need define custom position offset
	\param owner Entity soundEvent is linked to
	\param audioSourceConfiguration Custom audio source configuration
	\param worldPosition Position in world space sound will be triggered at
	*/
	SCR_AudioSource CreateAudioSource(notnull IEntity owner, SCR_AudioSourceConfiguration audioSourceConfiguration, vector worldPosition)
	{		
#ifdef ENABLE_DIAG
		s_iCreatedAudioSources++;
#endif
		
		// Is sound source in audible range?
		vector mat[4];
					
		// Get orientation matrix
		owner.GetTransform(mat);
				
		// Use worldPosition as initial sound position
		mat[3] = worldPosition;
						
		float distance = AudioSystem.IsAudible(audioSourceConfiguration.m_sSoundProject, audioSourceConfiguration.m_sSoundEventName, mat[3]);
		if (distance < 0)
		{
#ifdef ENABLE_DIAG
			s_iInaudibleAudioSources++;
#endif
			
			return null;
		}
						
		// Create audio source
		SCR_AudioSource audioSource = new SCR_AudioSource(audioSourceConfiguration, mat);
		
		// Set distance signal
		audioSource.SetSignalValue(SCR_AudioSource.DISTANCE_SINAL_NAME, distance);
		
		// Set owner
		audioSource.m_Owner = owner;
		
		// Set offset
		vector offset = owner.CoordToLocal(worldPosition);
		audioSource.m_vOffset = audioSource.m_vOffset + offset;
		SCR_Enum.SetFlag(audioSource.m_eFlags, EAudioSourceFlag.HasOffset);
						
		return audioSource;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	When creating AudioSource, basic distance and loudness check is performed to establish, if sound would be audible or not
	Use, when there is no owner entity for audio source
	\param audioSourceConfiguration Custom audio source configuration
	\param worldPosition Position in world space sound will play from
	*/
	SCR_AudioSource CreateAudioSource(SCR_AudioSourceConfiguration audioSourceConfiguration, vector worldPosition)
	{		
#ifdef ENABLE_DIAG
		s_iCreatedAudioSources++;
#endif
		
		// Is sound source in audible range?
		vector mat[4];
					
		// Set default orientation matrix
		Math3D.MatrixIdentity4(mat);
				
		// Use worldPosition as initial sound position
		mat[3] = worldPosition;			
					
		float distance = AudioSystem.IsAudible(audioSourceConfiguration.m_sSoundProject, audioSourceConfiguration.m_sSoundEventName, mat[3]);
		if (distance < 0)
		{
#ifdef ENABLE_DIAG
			s_iInaudibleAudioSources++;
#endif
			
			return null;
		}
		
		// Create audio source
		SCR_AudioSource audioSource = new SCR_AudioSource(audioSourceConfiguration, mat);
		
		// Set distance signal
		audioSource.SetSignalValue(SCR_AudioSource.DISTANCE_SINAL_NAME, distance);
		
		// Disable flags that make sense only when owner entity exists
		SCR_Enum.SetFlag(audioSource.m_eFlags, EAudioSourceFlag.FinishWhenEntityDestroyed);
		SCR_Enum.SetFlag(audioSource.m_eFlags, EAudioSourceFlag.Static);
					
		return audioSource;
	}
			
	//------------------------------------------------------------------------------------------------
	/*!
	Creates and plays audiosource
	Use when having SCR_SoundDataComponent on the owner EntityID
	\param owner Entity soundEvent is linked to
	\param eventName Sound event that will be played
	*/
	static void CreateAndPlayAudioSource(notnull IEntity owner, string eventName)
	{
		SCR_SoundManagerModule soundManager = SCR_SoundManagerModule.GetInstance(owner.GetWorld());
		if (!soundManager)
			return;
		
		SCR_AudioSource audioSource = soundManager.CreateAudioSource(owner, eventName);		
		if (!audioSource)
			return;
		
		soundManager.PlayAudioSource(audioSource);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Creates and plays audiosource
	Use when having custom AudioSourceConfiguration
	\param owner Entity soundEvent is linked to
	\param audioSourceConfiguration Custom audio source configuration
	*/
	static void CreateAndPlayAudioSource(notnull IEntity owner, SCR_AudioSourceConfiguration audioSourceConfiguration)
	{
		if (!audioSourceConfiguration || !audioSourceConfiguration.IsValid())
			return;
		
		SCR_SoundManagerModule soundManager = SCR_SoundManagerModule.GetInstance(owner.GetWorld());
		if (!soundManager)
			return;
				
		SCR_AudioSource audioSource = soundManager.CreateAudioSource(owner, audioSourceConfiguration);	
		if (!audioSource)
			return;
		
		soundManager.PlayAudioSource(audioSource);
	}
		
	//------------------------------------------------------------------------------------------------
	/*!
	Plays audiosource
	AudioSource playback follows after AudioSource creation and signals setup (optional)
	Use for sounds, when sound is linked to owner entity position
	\param audioSource AudioSource that is supposed to be played
	*/
	void PlayAudioSource(notnull SCR_AudioSource audioSource)
	{			
		// Set environmental signals
		audioSource.SetEnvironmentalSignals(m_SoundWorld);
												
		if (m_SoundWorld && !SCR_Enum.HasFlag(audioSource.m_eFlags, EAudioSourceFlag.ExteriorSource))
		{
			audioSource.CalculateInteriror(m_SoundWorld);
			
#ifdef ENABLE_DIAG			
			s_iCalculateInterirorAtCount++;
#endif

			// Inset audio source to audio source pool
			m_aAudioSource.Insert(audioSource);
		}
		else
		{			
			// Inset audio source to audio source pool
			if (audioSource.Play())
				m_aAudioSource.Insert(audioSource);
		}
	}
					
	//------------------------------------------------------------------------------------------------
	/*!
	Terminates all playing sounds on given entity 
	\param entity All sound events on this entity are terminated
	*/
	void TerminateAudioSource(IEntity entity)
	{	
#ifdef ENABLE_DIAG
		s_iTerminateAudioSourceCalls++;
#endif
					
		foreach(SCR_AudioSource audioSource : m_aAudioSource)
		{
			if (audioSource.m_Owner == entity)
			{
				audioSource.Terminate();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Terminates given audio source 
	\param audioSource AudioSource to terminate
	*/
	void TerminateAudioSource(SCR_AudioSource audioSource)
	{				
		// AudioSource was already played and removed
		if (!audioSource)
			return;	
		
		// Terminate sound
		audioSource.Terminate();
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Terminates all playing sounds that are not static
	*/
	void TerminateAll()
	{		
		foreach (SCR_AudioSource audioSource : m_aAudioSource)
		{
			audioSource.Terminate(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] world to search SCR_SoundManagerModule in
	//! \return instance of SCR_SoundManagerModule
	static SCR_SoundManagerModule GetInstance(World world)
	{
		SndSystem sndSystem = SndSystem.Cast(world.FindSystem(SndSystem));
		
		if (!sndSystem)
		{
			return null;
		}
		
		return SCR_SoundManagerModule.Cast(sndSystem.FindModule(SCR_SoundManagerModule));
	}
			
	//------------------------------------------------------------------------------------------------
	/*!
	Manages active AudioSources
	*/
	private void HandleAudioSources()
	{							
		for (int i = m_aAudioSource.Count() - 1; i >= 0; i--)
		{
			SCR_AudioSource audioSource = m_aAudioSource[i];
							
			// Remove audio source if sound finished playing
			if (AudioSystem.IsSoundPlayed(audioSource.m_AudioHandle) && !audioSource.m_InteriorRequestCallback)
			{
				m_aAudioSource.Remove(i);
				continue;
			}
					
			if (audioSource.m_Owner)
			{
				// Update audio source position				
				if (!SCR_Enum.HasFlag(audioSource.m_eFlags, EAudioSourceFlag.Static))
					audioSource.UpdateSoundTransformation();			
			}
			else
			{
				// Remove audio source if parent entity was deleted
				if (!SCR_Enum.HasFlag(audioSource.m_eFlags, EAudioSourceFlag.FinishWhenEntityDestroyed) && !audioSource.m_InteriorRequestCallback)
				{
					audioSource.Terminate();
					m_aAudioSource.Remove(i);
				}
			}
		}
		
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_SOUND_MANAGER))
		{
			int count = m_aAudioSource.Count();
			DbgUI.Begin("Sound Manager");
			DbgUI.Text("Active Audio Sources          :" + count);
			if (count > s_iMaxActiveAudioSources)
				s_iMaxActiveAudioSources = count;
			DbgUI.Text("Maximum Active Audio Sources  :" + s_iMaxActiveAudioSources);
			DbgUI.Text("Created Audio Sources         :" + s_iCreatedAudioSources);
			DbgUI.Text("Played Audio Sources          :" + s_iPlayedAudioSources);
			DbgUI.Text("Invalid Audio Sources         :" + s_iInvalidAudioSources);
			DbgUI.Text("Inaudible Audio Sources       :" + s_iInaudibleAudioSources);
			DbgUI.Text("TerminateAudioSource() Calls  :" + s_iTerminateAudioSourceCalls);
			DbgUI.Text("CalculateInterirorAt() Count  :" + s_iCalculateInterirorAtCount);
			DbgUI.End();
			
			// Print events for active audio sources
			if (m_aAudioSource.Count() != 0)
			{
				Print("--- " + m_aAudioSource.Count().ToString() + " playing sound events ---");
				foreach (SCR_AudioSource audioSource : m_aAudioSource)
				{
					Print(audioSource.m_sSoundEventName);
				}
			}
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_GLOBAL_VARIABLES))
		{					
			Resource holder = BaseContainerTools.LoadContainer("{A60F08955792B575}Sounds/_SharedData/Variables/GlobalVariables.conf");			
			if (!holder)
				return;
			
			BaseContainer baseContainer = holder.GetResource().ToBaseContainer();
			if (!baseContainer)
				return;
						
			BaseContainerList list = baseContainer.GetObjectArray("Variables");
			if (!list)
				return;		
			
			DbgUI.Begin("Global Variables");
			for (int i, count = list.Count(); i < count; i++)
			{
				string name = list[i].GetName();
				DbgUI.Text(name + ": " + AudioSystem.GetVariableValue(name, "{A60F08955792B575}Sounds/_SharedData/Variables/GlobalVariables.conf").ToString(-1,2));
			}
			DbgUI.End();
		}		
#endif
	}
			
	//------------------------------------------------------------------------------------------------
	/*!
	Sets GDynamicRange signal value based on game settings
	*/	
	protected void SetDynamicRangeSignalValue()
	{			
		BaseContainer settings = GetGame().GetGameUserSettings().GetModule("SCR_AudioSettings");    		
		if (!settings)
			return;    

		float value;
		settings.Get("m_fDynamicRange", value);
		
		// DynamicRange has range <-1, 1>
		GetGame().GetSignalsManager().SetSignalValue(s_iDynamicRangeIdx, value * 0.01 - 1);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Sets EnableTinnitus signal value based on game settings
	*/	
	protected void SetTinnitusSignalValue()
	{
		BaseContainer settings = GetGame().GetGameUserSettings().GetModule("SCR_AudioSettings");
		if (!settings)
			return;

		bool value;
		settings.Get("m_bGTinnitus", value);

		GetGame().GetSignalsManager().SetSignalValue(s_iGTinnitusIdx, value);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Sets InEditor signal to 1
	*/
	protected void OnEditorOpen()
	{
		GetGame().GetSignalsManager().SetSignalValue(s_iInEditorIdx, 1);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Sets InEditor signal to 0
	*/
	protected void OnEditorClose()
	{
		GetGame().GetSignalsManager().SetSignalValue(s_iInEditorIdx, 0);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Registers to open and close editor event listenets
	*/	
	protected void OnEditorManagerInit(SCR_EditorManagerEntity editorManager)
	{
		editorManager.GetOnOpened().Insert(OnEditorOpen);
		editorManager.GetOnClosed().Insert(OnEditorClose);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Unregisters from open and close editor event listenets
	*/	
	protected void OnEditorManagerExit(SCR_EditorManagerEntity editorManager)
	{
		editorManager.GetOnOpened().Remove(OnEditorOpen);
		editorManager.GetOnClosed().Remove(OnEditorClose);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Editor manager events init
	*/	
	protected void EditorManagerEventsInit()
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
			OnEditorManagerInit(editorManager);
		else
		{
			SCR_EditorManagerCore editorManagerCore = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			if (editorManagerCore)
				editorManagerCore.Event_OnEditorManagerInitOwner.Insert(OnEditorManagerInit);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Editor manager events exit
	*/
	protected void EditorManagerEventsExit()
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
			OnEditorManagerExit(editorManager);
		
		SCR_EditorManagerCore editorManagerCore = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (editorManagerCore)
			editorManagerCore.Event_OnEditorManagerInitOwner.Remove(OnEditorManagerInit);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnUpdate(float timeslice)
	{		
		HandleAudioSources();
	}
		
	//------------------------------------------------------------------------------------------------
	override void OnInit()
	{			
		// Get global signals indexes
		GameSignalsManager gameSignalsManager = GetGame().GetSignalsManager();
		
		s_iDynamicRangeIdx = gameSignalsManager.AddOrFindSignal(DYNAMIC_RANGE_SIGNAL_NAME);
		s_iGTinnitusIdx = gameSignalsManager.AddOrFindSignal(G_TINNITUS_SIGNAL_NAME);
		s_iInEditorIdx = gameSignalsManager.AddOrFindSignal(IN_EDITOR_SIGNAL_NAME);
		
		// Event listeners for audio game settings
		GetGame().OnUserSettingsChangedInvoker().Insert(SetDynamicRangeSignalValue);
		GetGame().OnUserSettingsChangedInvoker().Insert(SetTinnitusSignalValue);
		// Update DynamicRange signal
		SetDynamicRangeSignalValue();
		// Update EnableTinnitus signal
		SetTinnitusSignalValue();
		
		//Event listeners for editor manager
		EditorManagerEventsInit();
		
		// GetSoundWorld
		ChimeraWorld chimeraWorld = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (chimeraWorld)
			m_SoundWorld = chimeraWorld.GetSoundWorld();
		
#ifdef ENABLE_DIAG	
		s_iCreatedAudioSources = 0;
		s_iMaxActiveAudioSources = 0;
		s_iPlayedAudioSources = 0;
		s_iInvalidAudioSources = 0;
		s_iInaudibleAudioSources = 0;
		s_iTerminateAudioSourceCalls = 0;
		s_iCalculateInterirorAtCount = 0;
#endif	
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_SoundManagerModule(IEntitySource src, IEntity parent)
	{		
#ifdef ENABLE_DIAG
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_SOUND_MANAGER, "", "Sound Manager", "Sounds");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_GLOBAL_VARIABLES, "", "Global Variables", "Sounds");
#endif	
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_SoundManagerModule()
	{
		// Terminate all playing sounds
		TerminateAll();
		
		// Remove from audio game settings event
		GetGame().OnUserSettingsChangedInvoker().Remove(SetDynamicRangeSignalValue);
		
		// Remove from editor manager events
		EditorManagerEventsExit();
		
#ifdef ENABLE_DIAG				
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SOUNDS_SOUND_MANAGER);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SOUNDS_GLOBAL_VARIABLES);
#endif
	}
};