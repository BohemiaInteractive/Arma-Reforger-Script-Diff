modded enum ECommunicationType
{
	RALLY_CROWD_LINE
}

modded class SCR_AISoundHandling
{
	//------------------------------------------------------------------------------------------------
	override static bool SetSignalsAndTransmit(SCR_AITalkRequest request, IEntity speakerEntity, VoNComponent von, SignalsManagerComponent signalsMgr)
	{
		// Only handle rally lines here, everything else belongs to the base implementation
		if (request.m_eCommType != ECommunicationType.RALLY_CROWD_LINE)
			return super.SetSignalsAndTransmit(request, speakerEntity, von, signalsMgr);

		array<float> signals = {};
		SCR_CallsignCharacterComponent callsignComponent;
		int SN_Seed = signalsMgr.FindSignal("Seed");
		// randomize variant of the signal
		SetSignal(SN_Seed, Math.RandomFloat(0, 1), signals);
		// different signals depending on communication type
		if (request.m_Entity)
			callsignComponent = SCR_CallsignCharacterComponent.Cast(request.m_Entity.FindComponent(SCR_CallsignCharacterComponent));

		ECommunicationType commType = request.m_eCommType;
		switch (commType)
		{
			case ECommunicationType.RALLY_CROWD_LINE:
			{
				// enum in request is int
				const float mood = 0.01 * request.m_EnumSignal;
				const int SN_Mood = signalsMgr.FindSignal("PlayerRank");

				SetSignal(SN_Mood, mood, signals);
				von.SoundEventPriority(SCR_SoundEvent.SOUND_RALLY_CHEERING_LINES, signals, NORMAL_PRIORITY);
				break;
			}
		}

		return true;
	}
}
