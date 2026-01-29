class SCR_RadioTuningUserAction : SCR_InventoryAction
{
	protected SCR_RadioComponent m_RadioComp;

	//------------------------------------------------------------------------------------------------
	[Attribute("0")]
	protected bool m_bTuneUp;

	override bool CanBeShownScript(IEntity user)
	{
		if (!m_RadioComp)
			return false;

		CharacterControllerComponent charComp = CharacterControllerComponent.Cast(user.FindComponent(CharacterControllerComponent));
		return charComp.GetInspect();
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		bool changeFreq;
		if (CanBePerformed(pUserEntity))
			changeFreq = m_RadioComp.ChangeFrequencyStep(m_bTuneUp);

		if (!changeFreq)
		{
			SCR_SoundManagerModule.CreateAndPlayAudioSource(pOwnerEntity, SCR_SoundEvent.SOUND_ITEM_RADIO_TUNE_ERROR);
		}
		else
		{
			if (m_bTuneUp)
				SCR_SoundManagerModule.CreateAndPlayAudioSource(pOwnerEntity, SCR_SoundEvent.SOUND_ITEM_RADIO_TUNE_UP);
			else
				SCR_SoundManagerModule.CreateAndPlayAudioSource(pOwnerEntity, SCR_SoundEvent.SOUND_ITEM_RADIO_TUNE_DOWN);
		}
	}

	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}

	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_RadioComp = SCR_RadioComponent.Cast(pOwnerEntity.FindComponent(SCR_RadioComponent));
	}

	override bool GetActionNameScript(out string outName)
	{
		BaseRadioComponent radioComp = m_RadioComp.GetRadioComponent();
		if (!radioComp)
			return false;

		// Get the first transceiver, but may become problematic for manipulation of multiple channels of the radio
		BaseTransceiver transceiver = radioComp.GetTransceiver(0);
		if (!transceiver)
			return false;

		float targetFreq = transceiver.GetFrequency() + transceiver.GetFrequencyResolution();
		if (!m_bTuneUp)
			targetFreq = transceiver.GetFrequency() - transceiver.GetFrequencyResolution();

		outName = WidgetManager.Translate("#AR-UserAction_TuneRadioToFrequency", targetFreq / 1000);
		return true;
	}
};