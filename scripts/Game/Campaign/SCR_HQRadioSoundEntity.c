class SCR_HQRadioSoundEntityClass : GenericEntityClass
{
}

class SCR_HQRadioSoundEntity : GenericEntity
{
	protected SimpleSoundComponent m_SimpleSoundComponent;
	protected AudioHandle m_PlayedRadio = AudioHandle.Invalid;

	protected static SCR_HQRadioSoundEntity s_Instance = null;

	//------------------------------------------------------------------------------------------------
	//! \return instance of SCR_HQRadioSoundEntity
	static SCR_HQRadioSoundEntity GetInstance()
	{
		if (!s_Instance)
		{
			SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
			if (!campaign)
				return null;

			BaseWorld world = GetGame().GetWorld();

			if (world)
			{
				Resource resource = Resource.Load(campaign.GetHQRadioSoundEntityPrefab());
				if (!resource || !resource.IsValid())
					return null;

				s_Instance = SCR_HQRadioSoundEntity.Cast(GetGame().SpawnEntityPrefab(resource, world));
			}
		}

		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	//! Play radio sound
	//! \param[in] soundEventName
	void PlayRadioSound(string soundEventName)
	{
		AudioSystem.TerminateSound(m_PlayedRadio);
		m_PlayedRadio = m_SimpleSoundComponent.PlayStr(soundEventName);

		#ifdef ENABLE_DIAG
		if (m_PlayedRadio == AudioHandle.Invalid)
			Print("[SCR_HQRadioSoundEntity.PlayRadioSound] soundEventName:"+soundEventName+" not configured.", LogLevel.WARNING);
		#endif
	}

	//------------------------------------------------------------------------------------------------
	SimpleSoundComponent GetSimpleSoundComponent()
	{
		if (!m_SimpleSoundComponent)
			m_SimpleSoundComponent = SimpleSoundComponent.Cast(FindComponent(SimpleSoundComponent));

		return m_SimpleSoundComponent;
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_HQRadioSoundEntity()
	{
		s_Instance = null;
	}
}
