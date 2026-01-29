//! Radio slot
enum ERadioCategory
{
	PERSONAL,
	MANPACK
}

//! Radio model
enum ERadioType
{
	ANPRC68,
	ANPRC77,
	R148,
	R107M
}

//! Frequency unit prefix
enum EFreqUnit
{
	KHZ = 0,
	MHZ = 1
}

//! Radio procedural animation context
class SCR_RadioProcAnimCtx
{
	int sAnimSignal;
	float fStartVal;
	float fEndVal;
	float fAnimSpeed;
	float fTimeSlice;
	bool bInProgress;
}

[EntityEditorProps(category: "GameScripted/Gadgets", description: "Radio gadget", color: "0 0 255 255")]
class SCR_RadioComponentClass : SCR_GadgetComponentClass
{
}

class SCR_RadioComponent : SCR_GadgetComponent
{
	[Attribute("0", UIWidgets.ComboBox, "Radio category", "", ParamEnumArray.FromEnum(ERadioCategory), category: "Radio")]
	protected int m_iRadioCategory;

	[Attribute("0", UIWidgets.ComboBox, "Radio type", "", ParamEnumArray.FromEnum(ERadioType), category: "Radio")]
	protected int m_iRadioType;

	protected bool m_bIsPowered = true;		// radio on/off toggle
	protected BaseRadioComponent m_BaseRadioComp;
	protected SignalsManagerComponent m_SignalManager;
	
	// signals
	protected bool m_bSignalInit = false;
	protected int m_iSignalPower = -1;
	protected int m_iSignalChannelKhz = -1;
	protected int m_iSignalChannelMhz = -1;

	// proc anims
	protected ref array<ref SCR_RadioProcAnimCtx> m_aProcAnims = new array<ref SCR_RadioProcAnimCtx>();
	protected bool m_bAnimInProgress;

	//------------------------------------------------------------------------------------------------
	//! Radio on/off toggle
	void RadioToggle()
	{
		m_BaseRadioComp.SetPower( !m_BaseRadioComp.IsPowered() );
		m_bIsPowered = m_BaseRadioComp.IsPowered();

		PlayerController pc = GetGame().GetPlayerController();
		if (pc)
		{
			SCR_VONController vonController = SCR_VONController.Cast(pc.FindComponent(SCR_VONController));
			if (vonController)
			{
				array<ref SCR_VONEntry> entries = {};
				vonController.GetVONEntries(entries);
				
				foreach (SCR_VONEntry entry : entries)
				{
					SCR_VONEntryRadio radioEntry = SCR_VONEntryRadio.Cast(entry);
					BaseRadioComponent radio = radioEntry.GetTransceiver().GetRadio();
					if (radio == m_BaseRadioComp)
						entry.SetUsable(radio.IsPowered());
				}
			}
		}
		
		if (m_bIsPowered)
		{
			if ( m_iRadioType == ERadioType.ANPRC68 || ERadioType.ANPRC77 )
				SetKnobAnim(m_iSignalPower, 1, 0.3, false);
		}
		else
		{
			if ( m_iRadioType == ERadioType.ANPRC68 || ERadioType.ANPRC77 )
				SetKnobAnim(m_iSignalPower, 0, 0.3, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Change frequency by a single step, init proc anims
	//! \param[in] direction decides whether the frequency is increased (true) / decreased (false)
	//! \return true on success, false otherwise
	bool ChangeFrequencyStep(bool direction)
	{
		if (!m_BaseRadioComp.IsPowered() || m_bAnimInProgress)
			return false;

		BaseTransceiver tsv = m_BaseRadioComp.GetTransceiver(0);
		if (!tsv)
			return false;

		int freq = tsv.GetFrequency();
		int freqMax = tsv.GetMaxFrequency();
		int freqMin = tsv.GetMinFrequency();

		if ( ( freq + tsv.GetFrequencyResolution() ) > freqMax && direction)
			return false;
		else if ( ( freq - tsv.GetFrequencyResolution() ) < freqMin && !direction)
			return false;
		else
		{
			if (direction)
				freq += tsv.GetFrequencyResolution();
			else
				freq -= tsv.GetFrequencyResolution();

			tsv.SetFrequency(freq);
			
			float targetAngle;

			// proc anims
			if ( m_iRadioType != ERadioType.ANPRC68 )
			{
				targetAngle = GetKnobAngle(EFreqUnit.MHZ);
				SetKnobAnim(m_iSignalChannelMhz, targetAngle, 0.3, true);
			}

			targetAngle = GetKnobAngle(EFreqUnit.KHZ);
			SetKnobAnim(m_iSignalChannelKhz, targetAngle, 0.3, false);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Calculate target angle of a knob for procedural animation
	//! \param[in] freqUnit is frequency unit
	//! \return
	protected float GetKnobAngle(EFreqUnit freqUnit)
	{
		int step, stepsCount;
		int currentStep = 0;

		stepsCount = GetKnobAnimStepCount(freqUnit);
		BaseTransceiver tsv = m_BaseRadioComp.GetTransceiver(0);
		if (!tsv)
			return 0.0;

		step = (tsv.GetMinFrequency() - tsv.GetFrequency()) * -1;

		// Special case of preset number of channels
		if (m_iRadioType == ERadioType.ANPRC68)
		{
			if (step > 0)
				currentStep = step/tsv.GetFrequencyResolution();
		}
		// MHz, looking for the first two digits of a 5 digits frequency, steps are based on models dial
		else if (freqUnit == EFreqUnit.MHZ)
		{
			if (step > 0)
				currentStep = Math.Floor(step/1000);
		}
		// kHz
		else
		{
			if (step >= 1000)
			{
				while (step >= 1000)
				{
					step -= 1000;
				}
			}

			// half steps 050, 150 etc
			if ((step/50) % 2 == 0)
				currentStep = 0;
			else
				currentStep = 1;

			step = Math.Floor(step/100);
			currentStep = currentStep + (step * 2); // 20 steps(000 - 100) instead of 10 + possible half step (050)
		}

		return currentStep * (360/stepsCount);

	}

	//------------------------------------------------------------------------------------------------
	// Proc anims
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Set knob rotation according to the current state
	protected void UpdateKnobState()
	{
		bool state;

		if (m_bIsPowered)
			state = 1;
		else
			state = 0;

		if (!m_SignalManager)
			return;
		
		if (!m_bSignalInit)
			InitSignals();
		
		m_SignalManager.SetSignalValue( m_iSignalPower, (float)state );
		m_SignalManager.SetSignalValue( m_iSignalChannelKhz, GetKnobAngle(EFreqUnit.KHZ) );

		if ( m_iRadioType != ERadioType.ANPRC68 )
			m_SignalManager.SetSignalValue( m_iSignalChannelMhz, GetKnobAngle(EFreqUnit.MHZ) );
	}

	//------------------------------------------------------------------------------------------------
	//! Knob animation setter
	//! \param[in] signal
	//! \param[in] targetAngle is the final angle
	//! \param[in] speed is animation length in seconds
	//! \param[in] clumpAnim false - start anim instantly / false - clump anim with the next one
	protected void SetKnobAnim(int signal, float targetAngle, float speed, bool clumpAnim)
	{
		if (m_bAnimInProgress || !m_SignalManager)
			return;

		SCR_RadioProcAnimCtx animCtx = new SCR_RadioProcAnimCtx();
		animCtx.sAnimSignal = signal;
		animCtx.fStartVal = m_SignalManager.GetSignalValue(signal);
		animCtx.fEndVal = targetAngle;
		animCtx.fAnimSpeed = speed;
		animCtx.fTimeSlice = 0;
		animCtx.bInProgress = true;

		m_aProcAnims.Insert(animCtx);

		if (!clumpAnim)
		{
			m_bAnimInProgress = true;
			ActivateGadgetUpdate();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Knob animation tick
	//! \param[in] timeSlice is frame timeSlice
	protected void AnimKnob(float timeSlice)
	{
		int inProgress;

		for ( int i = 0; i < m_aProcAnims.Count(); i++ )
		{
			SCR_RadioProcAnimCtx animCtx = m_aProcAnims[i];
			if (!animCtx)
				continue;

			if (animCtx.bInProgress)
				inProgress++;

			float newAngle;
			animCtx.fTimeSlice += timeSlice / animCtx.fAnimSpeed;

			// if ended
			if ( animCtx.fTimeSlice >= 1 )
			{
				animCtx.bInProgress = false;
				newAngle = animCtx.fEndVal;
			}
			else
			{
				// Turn counter clockwise if closer
				if ( Math.AbsFloat(animCtx.fStartVal - animCtx.fEndVal) > 180 )
				{
					if ( animCtx.fStartVal < 0)
						animCtx.fStartVal += 360;
					else
						animCtx.fStartVal -= 360;
				}

				float lerp = Math.Lerp(animCtx.fStartVal, animCtx.fEndVal, animCtx.fTimeSlice);
				newAngle = lerp;
			}

			m_SignalManager.SetSignalValue(animCtx.sAnimSignal, newAngle);
		}

		if (inProgress <= 0)
		{
			m_bAnimInProgress = false;
			m_aProcAnims.Clear();
			DeactivateGadgetUpdate();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Amount of possible steps (numbers) on frequency dials for each radio
	//! \param[in] freqUnit is frequency unit
	//! \return
	protected int GetKnobAnimStepCount(EFreqUnit freqUnit)
	{
		if ( m_iRadioType == ERadioType.ANPRC68 )
		{
			return 10;
		}
		else if ( m_iRadioType == ERadioType.R148 )
		{
			if (freqUnit == EFreqUnit.MHZ)
				return 15;
			else
				return 20;
		}
		else if ( m_iRadioType == ERadioType.ANPRC77 )
		{
			if (freqUnit == EFreqUnit.MHZ)
				return 24; // 22 + 1 empty & +1 duplicated
			else
				return 24; // 20 +4 empty ones
		}

		return 1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Cache procedural animation signals
	protected void InitSignals()
	{
		// cache signals
		m_iSignalPower = m_SignalManager.FindSignal("Power");
		m_iSignalChannelKhz = m_SignalManager.FindSignal("Channel_k");
		m_iSignalChannelMhz = m_SignalManager.FindSignal("CHannel_M");
		
		if (m_iSignalPower != -1 && m_iSignalChannelKhz != -1 && m_iSignalChannelMhz != -1)
			m_bSignalInit = true;
	}

	//------------------------------------------------------------------------------------------------
	// Overrides
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	override void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{		
		super.ModeSwitch(mode, charOwner);

		// Update knobs
		UpdateKnobState();
	}
	
	//------------------------------------------------------------------------------------------------
	override void ActivateGadgetUpdate()
	{
		super.ActivateGadgetUpdate();
		
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (itemComponent)
			itemComponent.ActivateOwner(true);	// required for the procedural animations to function
	}
	
	//------------------------------------------------------------------------------------------------
	override void DeactivateGadgetUpdate()
	{
		super.DeactivateGadgetUpdate();
		
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (itemComponent)
			itemComponent.ActivateOwner(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateVisibility(EGadgetMode mode)
	{
		if (m_iRadioCategory != ERadioCategory.MANPACK)	// let radio backpack visibility be handled by inventory
			super.UpdateVisibility(mode);
	}
	
	//------------------------------------------------------------------------------------------------
	override EGadgetType GetType()
	{
		if (m_iRadioCategory == ERadioCategory.MANPACK)
			return EGadgetType.RADIO_BACKPACK;
		else
			return EGadgetType.RADIO;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return radio type
	ERadioType GetRadioType()
	{
		return m_iRadioType;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsVisibleEquipped()
	{
		if (m_iRadioCategory == ERadioCategory.MANPACK)
			return true;
		else
			return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	BaseRadioComponent GetRadioComponent()
	{
		return m_BaseRadioComp;
	}	
	
	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		// knob anim
		if (!m_bAnimInProgress)
		{
			DeactivateGadgetUpdate();
			return;
		}
		
		AnimKnob(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		// BaseRadioComponent
		m_BaseRadioComp =  BaseRadioComponent.Cast(owner.FindComponent(BaseRadioComponent));
		if (!m_BaseRadioComp)
			Print("SCR_RadioEntity: Failed acquiring BaseRadioComponent", LogLevel.WARNING);

		// signal manager
		m_SignalManager = SignalsManagerComponent.Cast(owner.FindComponent( SignalsManagerComponent ) );
	}
}
