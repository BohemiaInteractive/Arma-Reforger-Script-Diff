enum EAudioSourceFlag
{
	Static						= 1 << 0,
	EnvironmentSignals 			= 1 << 1,
	FinishWhenEntityDestroyed 	= 1 << 2,
	BoundingVolume 				= 1 << 3,
	ExteriorSource 				= 1 << 4,
	Terminated					= 1 << 5,
	HasOffset		 			= 1 << 6
}

class SCR_AudioSource
{	 
	//! Stores valid Audio handle
	AudioHandle m_AudioHandle = AudioHandle.Invalid;
	//! Paren entity audio source is linked to
	IEntity m_Owner;
	//! AudioSource position
	protected vector m_aMat[4];
	//! Playback position offset
	vector m_vOffset;
	//! Audio project resource name
	ResourceName m_sSoundProject;
	//! Sound event name
	string m_sSoundEventName;
	//! Signal names
	protected ref array<string> m_aSignalName;
	//! Signal values
	protected ref array<float> m_aSignalValue;
	//! Interior callback
	ref SCR_InteriorRequestCallback m_InteriorRequestCallback;
	//! Flags describing runtime AudioSource state or behaviour
	EAudioSourceFlag m_eFlags;
	
	//! Local signal names
	static const string INTERIOR_SIGNAL_NAME = "Interior";
	static const string SURFACE_SIGNAL_NAME = "Surface";
	static const string ENTITY_SIZE_SIGNAL_NAME = "EntitySize";
	static const string PHASES_TO_DESTROYED_PHASE_SIGNAL_NAME = "PhasesToDestroyed";
	static const string COLLISION_D_V_SIGNAL_NAME = "CollisionDV";
	static const string DISTANCE_SINAL_NAME = "Distance";
	static const string ROOM_SIZE_SIGNAL_NAME = "RoomSize";
	static const string FOREST_SIGNAL_NAME = "Forest";
	static const string HOUSES_SIGNAL_NAME = "Houses";
	static const string MEADOWS_SIGNAL_NAME = "Meadows";
	static const string SEA_SIGNAL_NAME = "Sea";
	
	//------------------------------------------------------------------------------------------------	
	/*!
	Sets value for given signal name 
	\name Signal name
	\value Signal value
	*/
	void SetSignalValue(string name, float value)
	{
		if (!m_aSignalName)
			m_aSignalName = {};
		
		if (!m_aSignalValue)
			m_aSignalValue = {};
		
		m_aSignalName.Insert(name);
		m_aSignalValue.Insert(value);
	}
		
	//------------------------------------------------------------------------------------------------
	/*!
	Sets environmental signals based on SoundMap
	*/
	void SetEnvironmentalSignals(SoundWorld soundWorld)
	{
		if (!soundWorld || !SCR_Enum.HasFlag(m_eFlags, EAudioSourceFlag.EnvironmentSignals))
			return;
		
		float sea, forest, city, meadow;	
		soundWorld.GetMapValuesAtPos(m_aMat[3], sea, forest, city, meadow);
		
		SetSignalValue(SEA_SIGNAL_NAME, sea);
		SetSignalValue(FOREST_SIGNAL_NAME, forest);
		SetSignalValue(HOUSES_SIGNAL_NAME, city);	
		SetSignalValue(MEADOWS_SIGNAL_NAME, meadow);
	}
	
	//------------------------------------------------------------------------------------------------
	bool Play()
	{						
		// Play event
		m_AudioHandle = AudioSystem.PlayEvent(m_sSoundProject, m_sSoundEventName, m_aMat, m_aSignalName, m_aSignalValue);

		// Check if AudioHandle is valid
		if (m_AudioHandle == AudioHandle.Invalid)
			return false;
		
		// Set bounding volume size
		if (m_Owner && SCR_Enum.HasFlag(m_eFlags, EAudioSourceFlag.BoundingVolume))
		{
			// Get world bounding box
			vector mins, maxs;			
			m_Owner.GetWorldBounds(mins, maxs);													
			AudioSystem.SetBoundingVolumeParams(m_AudioHandle, AudioSystem.BV_Box, maxs[0] - mins[0], maxs[1] - mins[1], maxs[2] - mins[2]);
		}	
						
		return true;
	}
			
	//------------------------------------------------------------------------------------------------
	void UpdateSoundTransformation()
	{	
		m_Owner.GetTransform(m_aMat);
		
		// Apply position offset
		if (SCR_Enum.HasFlag(m_eFlags, EAudioSourceFlag.HasOffset))
		{
			m_aMat[3] = m_Owner.CoordToParent(m_vOffset);
		}
						
		AudioSystem.SetSoundTransformation(m_AudioHandle, m_aMat);	
	}
			
	//------------------------------------------------------------------------------------------------
	void CalculateInteriror(SoundWorld soundWorld)
	{
		m_InteriorRequestCallback = new SCR_InteriorRequestCallback(this);	
		soundWorld.CalculateInterirorAt(m_aMat[3], m_InteriorRequestCallback);
	}
	
	//------------------------------------------------------------------------------------------------
	void Terminate(bool fadeOut = true)
	{
		if (fadeOut)
			AudioSystem.TerminateSound(m_AudioHandle);
		else
			AudioSystem.TerminateSoundFadeOut(m_AudioHandle, false, 0);	
		
		SCR_Enum.SetFlag(m_eFlags, EAudioSourceFlag.Terminated);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_AudioSource(SCR_AudioSourceConfiguration audioSourceConfiguration, vector mat[4])
	{
		m_sSoundProject = audioSourceConfiguration.m_sSoundProject;
		m_sSoundEventName = audioSourceConfiguration.m_sSoundEventName;
		m_eFlags = audioSourceConfiguration.m_eFlags;
		m_aMat = mat;
		
		if (audioSourceConfiguration.HasOffset())
		{
			m_vOffset = audioSourceConfiguration.m_vOffset;
			SCR_Enum.SetFlag(m_eFlags, EAudioSourceFlag.HasOffset);
		}
	}
}