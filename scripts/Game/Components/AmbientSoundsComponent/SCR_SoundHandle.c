[BaseContainerProps(configRoot: true)]
class SCR_SoundHandle
{
	//! Sound transformation
	vector m_aMat[4];	

	//! Sound group defines a group of sounds. E.g. birds playing on trees close to the camera, or insects, or birds playing in the distance
	int m_iSoundGroup;

	//! SoundType defines a group of sounds that can be played on a given location or entity. The definition varies based on the used ESpawnMethod
	int m_iSoundType;

	//! SoundDef defines basic sound behaviour such as the number of repetitions, or daytime/wind behaviour
	int m_iSoundDef;

	//! SoundDefinition
	SCR_SoundDef m_SoundDef;

	//! Sequence of worldTimes, when sound events will be triggered		
	ref array<int> m_aRepTime;

	//! Index of next worldTime, when event will be trigged
	int m_iRepTimeIdx;

	//! Percentage ratio between sequence length and total length of played samples
	float m_fDensity;
	
	//------------------------------------------------------------------------------------------------
	//! Updates density
	//! \param[in] sampleLenght
	//! \param[in] worldTime
	void UpdateDensity(float sampleLenght, float worldTime)
	{		
		float sequenceLenght = GetSequenceLenght(worldTime);
		if (sequenceLenght == 0)
		{
			m_fDensity = 0;
			return;
		}

		m_fDensity = Math.Clamp(sampleLenght * m_aRepTime.Count() / sequenceLenght, 0, 1) * 100;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns sequence length
	//! \param[in] worldTime
	//! \return
	float GetSequenceLenght(float worldTime)
	{
		int size = m_aRepTime.Count();
		if (size == 0)
			return 0;
		else
			return m_aRepTime[size - 1] - worldTime + m_SoundDef.m_iSampleLength;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetRandomInt(int value, int random)
	{
		if (random == 0)
			return value;
		
		int randomValue = value + Math.RandomFloat(-random, random);
		if (randomValue < 1)
			randomValue = 1;
					
		return randomValue;
	}
	
	//------------------------------------------------------------------------------------------------	
	protected void CreateSequence(array<ref SCR_SequenceDef> sequenceDefinitions, int idx)
	{
		SCR_SequenceDef sequenceDefinition = sequenceDefinitions[idx];
		
		for (int i = 0, count = GetRandomInt(sequenceDefinition.m_iRepCount, sequenceDefinition.m_iRepCountRnd); i < count; i++)
		{
			if (i > 0)
		    	m_aRepTime.Insert(GetRandomInt(sequenceDefinition.m_iRepTime, sequenceDefinition.m_iRepTimeRnd) + m_aRepTime[m_aRepTime.Count() - 1]);
			
			int idxNew = idx - 1;
			if (idxNew >= 0)
				CreateSequence(sequenceDefinitions, idxNew);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	//! Returns sequence length
	//! \param[in] gameTime
	void UpdateRepTime(float gameTime)
	{				
		if (m_aRepTime)
			m_aRepTime.Clear();
		else
			m_aRepTime = {};
		
		m_aRepTime.Insert(gameTime + GetRandomInt(m_SoundDef.m_iStartDelay, m_SoundDef.m_iStartDelayRnd));
		
		if (m_SoundDef.m_aSequenceDef.Count() == 0)
			Print("AmbientSoundsComponent: " + typename.EnumToString(ESoundName, m_SoundDef.m_eSoundName) + " is missing sequence definition", LogLevel.WARNING);
		else	
			CreateSequence(m_SoundDef.m_aSequenceDef, m_SoundDef.m_aSequenceDef.Count() - 1);
	}
		
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] soundGroupIdx
	//! \param[in] soundTypeIdx
	//! \param[in] soundDefIdx
	//! \param[in] mat
	//! \param[in] soundGroup
	//! \param[in] worldTime
	void SCR_SoundHandle(int soundGroupIdx, int soundTypeIdx, int soundDefIdx, vector mat[4], array<ref SCR_SoundGroup> soundGroup, float worldTime)
	{
		m_iSoundGroup = soundGroupIdx;
		m_iSoundType = soundTypeIdx;
		m_iSoundDef = soundDefIdx;	
		m_aMat = mat;	
		m_SoundDef = soundGroup[soundGroupIdx].m_aSoundType[soundTypeIdx].m_aSoundDef[soundDefIdx];		
		UpdateRepTime(worldTime);				
		UpdateDensity(m_SoundDef.m_iSampleLength, worldTime);
	}	
}
