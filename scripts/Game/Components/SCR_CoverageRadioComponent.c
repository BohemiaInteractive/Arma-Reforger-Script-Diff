class SCR_CoverageRadioComponentClass : ScriptedRadioComponentClass
{
}

//! Determinates if the owner entity is covered by the radio signal eminating from the component marked as Source with the same encryption key
class SCR_CoverageRadioComponent : ScriptedRadioComponent
{
	[Attribute("0", desc: "Source radio generates the radio signal. Should be limited to one per faction.", category: "Coverage")]
	protected bool m_bIsSource;

	protected bool m_bWasPowered = true;
	protected bool m_bLastSignalSignature;
	protected bool m_bCouldSend;
	protected bool m_bCouldReceive;

	protected float m_fSavedRange;

	protected string m_sSavedEncryptionKey;

	protected ref ScriptInvokerVoid m_OnCoverageChanged;

	protected ref array<SCR_CoverageRadioComponent> m_aRadiosInRange = {};
	protected ref array<SCR_CoverageRadioComponent> m_aInRangeOfRadios = {};

	protected ref map<string, SCR_ERadioCoverageStatus> m_mCoverage = new map<string, SCR_ERadioCoverageStatus>();

	[RplProp(onRplName: "OnCoverageChanged")]
	protected ref array<string> m_aEncryptionKeyCoverSend = {};

	[RplProp(onRplName: "OnCoverageChanged")]
	protected ref array<string> m_aEncryptionKeyCoverReceive = {};

	//------------------------------------------------------------------------------------------------
	void SetIsSource(bool isSource)
	{
		m_bIsSource = isSource;
	}

	//------------------------------------------------------------------------------------------------
	bool IsSource()
	{
		return m_bIsSource;
	}

	//------------------------------------------------------------------------------------------------
	bool WasPowered()
	{
		return m_bWasPowered;
	}

	//------------------------------------------------------------------------------------------------
	void PrepareCoverageUpdate(string encryptionKey, bool signature)
	{
		m_bCouldSend = m_aEncryptionKeyCoverSend.Contains(encryptionKey);
		m_bCouldReceive = m_aEncryptionKeyCoverReceive.Contains(encryptionKey);

		m_aEncryptionKeyCoverSend.RemoveItem(encryptionKey);
		m_aEncryptionKeyCoverReceive.RemoveItem(encryptionKey);

		m_bLastSignalSignature = signature;
	}

	//------------------------------------------------------------------------------------------------
	void FinishCoverageUpdate(string encryptionKey)
	{
		bool canSend = m_aEncryptionKeyCoverSend.Contains(encryptionKey);
		bool canReceive = m_aEncryptionKeyCoverReceive.Contains(encryptionKey);

		if (canSend == m_bCouldSend && canReceive == m_bCouldReceive)
			return;

		Replication.BumpMe();
		OnCoverageChanged();
	}

	//------------------------------------------------------------------------------------------------
	void GetRadiosInRange(notnull array<SCR_CoverageRadioComponent> radios)
	{
		radios.Copy(m_aRadiosInRange);
	}

	//------------------------------------------------------------------------------------------------
	void GetRadiosInRangeOf(notnull array<SCR_CoverageRadioComponent> radios)
	{
		radios.Copy(m_aInRangeOfRadios);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the amount of radios which are covered by this radio's signal
	// \param encryptionFilter When an encryption key is used, returns only radios with this encryption 
	int GetRadiosInRangeCount(string encryptionFilter = "")
	{
		if (encryptionFilter.IsEmpty())
		{
			return m_aRadiosInRange.Count();
		}
		else
		{
			int count;

			foreach (SCR_CoverageRadioComponent radio : m_aRadiosInRange)
			{
				if (radio.GetEncryptionKey() == encryptionFilter)
					count++;
			}

			return count;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the amount of radios which are covering this radio with their signal
	// \param encryptionFilter When an encryption key is used, returns only radios with this encryption
	int GetRadiosInRangeOfCount(string encryptionFilter = "")
	{
		if (encryptionFilter.IsEmpty())
		{
			return m_aInRangeOfRadios.Count();
		}
		else
		{
			int count;

			foreach (SCR_CoverageRadioComponent radio : m_aInRangeOfRadios)
			{
				if (radio.GetEncryptionKey() == encryptionFilter)
					count++;
			}

			return count;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if the provided radio is covered by this radio or vice versa, depending on the direction parameter
	// \param radio Radio we want to check for coverage
	// \param inMyRange True = check if the given radio is covered by this radio, False = check if the given radio covers this radio
	bool ConnectedRadiosContain(notnull SCR_CoverageRadioComponent radio, bool inMyRange)
	{
		if (inMyRange)
			return m_aRadiosInRange.Contains(radio);
		else
			return m_aInRangeOfRadios.Contains(radio);
	}

	//------------------------------------------------------------------------------------------------
	float GetSavedRange()
	{
		return m_fSavedRange;
	}

	//------------------------------------------------------------------------------------------------
	string GetSavedEncryption()
	{
		return m_sSavedEncryptionKey;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ERadioCoverageStatus GetCoverageByEncryption(string encryptionKey)
	{
		return m_mCoverage.Get(encryptionKey);
	}

	//------------------------------------------------------------------------------------------------
	void AddRadioInRange(notnull SCR_CoverageRadioComponent component)
	{
		if (m_aRadiosInRange.Contains(component))
			return;

		m_aRadiosInRange.Insert(component);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveRadioInRange(notnull SCR_CoverageRadioComponent component)
	{
		int index = m_aRadiosInRange.Find(component);

		if (index < 0)
			return;

		m_aRadiosInRange.Remove(index);
	}

	//------------------------------------------------------------------------------------------------
	void AddRadioInRangeOf(notnull SCR_CoverageRadioComponent component)
	{
		if (m_aInRangeOfRadios.Contains(component))
			return;

		m_aInRangeOfRadios.Insert(component);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveRadioInRangeOf(notnull SCR_CoverageRadioComponent component)
	{
		int index = m_aInRangeOfRadios.Find(component);

		if (index < 0)
			return;

		m_aInRangeOfRadios.Remove(index);
	}

	//------------------------------------------------------------------------------------------------
	void OnPowerToggle(bool powered)
	{
		m_bWasPowered = powered;

		if (!powered)
		{
			m_aRadiosInRange.Clear();

			foreach (SCR_CoverageRadioComponent radio : m_aInRangeOfRadios)
			{
				if (!radio)
					continue;

				radio.RemoveRadioInRange(this);
				radio.RemoveRadioInRangeOf(this);
			}

			m_aInRangeOfRadios.Clear();
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnRangeChanged(float newRange)
	{
		m_fSavedRange = newRange;
	}

	//------------------------------------------------------------------------------------------------
	void OnEncryptionChanged(string newEncryptionKey)
	{
		m_sSavedEncryptionKey = newEncryptionKey;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnCoverageChanged()
	{
		if (!m_OnCoverageChanged)
			m_OnCoverageChanged = new ScriptInvokerVoid();

		return m_OnCoverageChanged;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCoverageChanged()
	{
		m_mCoverage.Clear();

		foreach (string sending : m_aEncryptionKeyCoverSend)
		{
			m_mCoverage.Set(sending, SCR_ERadioCoverageStatus.SEND);
		}

		foreach (string receiving : m_aEncryptionKeyCoverReceive)
		{
			if (m_aEncryptionKeyCoverSend.Contains(receiving))
				m_mCoverage.Set(receiving, SCR_ERadioCoverageStatus.BOTH_WAYS);
			else
				m_mCoverage.Set(receiving, SCR_ERadioCoverageStatus.RECEIVE);
		}

		if (m_OnCoverageChanged)
			m_OnCoverageChanged.Invoke();

		SCR_RadioCoverageSystem coverageSystem = SCR_RadioCoverageSystem.GetInstance();

		if (coverageSystem)
			coverageSystem.OnCoverageChanged(this);
	}

	//------------------------------------------------------------------------------------------------
	void Ping(string encryptionKey, bool signalSignature, bool reverse = false)
	{
		// Signature changes each update so we know which radios have already been pinged to prevent infinite loops
		if (m_bLastSignalSignature == signalSignature)
			return;

		m_bLastSignalSignature = signalSignature;

		if (reverse)
		{
			m_aEncryptionKeyCoverSend.Insert(encryptionKey);

			if (m_sSavedEncryptionKey != encryptionKey)
				return;

			foreach (SCR_CoverageRadioComponent inRangeOfRadio : m_aInRangeOfRadios)
			{
				inRangeOfRadio.Ping(encryptionKey, signalSignature, reverse);
			}
		}
		else
		{
			m_aEncryptionKeyCoverReceive.Insert(encryptionKey);

			if (m_sSavedEncryptionKey != encryptionKey)
				return;

			foreach (SCR_CoverageRadioComponent radioInRange : m_aRadiosInRange)
			{
				radioInRange.Ping(encryptionKey, signalSignature, reverse);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{
		super.OnInit(owner);

		// Check for play mode again in case init event was set from outside of this class
		if (!GetGame().InPlayMode())
			return;

		// Map descriptor is only used for visual stuff. No need for it on headless.
		if (RplSession.Mode() != RplMode.Dedicated)
		{
			SCR_RadioCoverageMapDescriptorComponent mapDescriptor = SCR_RadioCoverageMapDescriptorComponent.Cast(owner.FindComponent(SCR_RadioCoverageMapDescriptorComponent));

			if (mapDescriptor)
				mapDescriptor.SetParentRadio(this);
		}

		SCR_RadioCoverageSystem manager = SCR_RadioCoverageSystem.GetInstance();

		if (!manager)
			return;

		BaseTransceiver transceiver = GetTransceiver(0);

		if (!transceiver)
			return;

		m_fSavedRange = transceiver.GetRange();
		m_sSavedEncryptionKey = GetEncryptionKey();
		m_bWasPowered = IsPowered();

		manager.RegisterRadioComponent(this, m_fSavedRange);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (!GetGame().InPlayMode())
			return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CoverageRadioComponent()
	{
		// Map descriptor is only used for visual stuff. No need for it on headless.
		if (RplSession.Mode() != RplMode.Dedicated)
		{
			IEntity owner = GetOwner();

			if (owner)
			{
				SCR_RadioCoverageMapDescriptorComponent mapDescriptor = SCR_RadioCoverageMapDescriptorComponent.Cast(GetOwner().FindComponent(SCR_RadioCoverageMapDescriptorComponent));

				if (mapDescriptor)
					mapDescriptor.SetParentRadio(null);
			}
		}

		SCR_RadioCoverageSystem manager = SCR_RadioCoverageSystem.GetInstance();

		foreach (SCR_CoverageRadioComponent radio : m_aInRangeOfRadios)
		{
			if (!radio)
				continue;

			radio.RemoveRadioInRange(this);
		}

		foreach (SCR_CoverageRadioComponent radio : m_aRadiosInRange)
		{
			if (!radio)
				continue;

			radio.RemoveRadioInRangeOf(this);
		}

		if (manager)
			manager.UnregisterRadioComponent(this);

		OnPowerToggle(false);
	}
}
