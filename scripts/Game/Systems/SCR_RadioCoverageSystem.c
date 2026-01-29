void OnCoverageChangedDelegate(SCR_CoverageRadioComponent radio);
typedef func OnCoverageChangedDelegate;
typedef ScriptInvokerBase<OnCoverageChangedDelegate> OnCoverageChangedInvoker;

class SCR_RadioCoverageSystem : GameSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo.SetAbstract(false);
	}

	protected bool m_bSignalSignature;
	protected bool m_bUpdateCoverage;
	protected bool m_bForcedPause;

	protected ref OnCoverageChangedInvoker m_OnCoverageChanged;

	protected ref array<SCR_CoverageRadioComponent> m_aRadioComponents = {};

	protected ref map<string, SCR_CoverageRadioComponent> m_mEncryptionSignalSources = new map<string, SCR_CoverageRadioComponent>();

	protected static const int UPDATE_DELAY = 2000;

	//------------------------------------------------------------------------------------------------
	static SCR_RadioCoverageSystem GetInstance()
	{
		World world = GetGame().GetWorld();

		if (!world)
			return null;

		return SCR_RadioCoverageSystem.Cast(world.FindSystem(SCR_RadioCoverageSystem));
	}

	//------------------------------------------------------------------------------------------------
	static bool UpdateAll(bool forceRecalculation = false)
	{
		SCR_RadioCoverageSystem system = GetInstance();

		if (system)
			return system.UpdateRadios(forceRecalculation);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override event bool ShouldBePaused()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnStarted()
	{
		if (!m_bForcedPause)
		{
			UpdateRadios(true);
			GetGame().GetCallqueue().CallLater(UpdateRadios, UPDATE_DELAY, true, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnStopped()
	{
		GetGame().GetCallqueue().Remove(UpdateRadios);
	}

	//------------------------------------------------------------------------------------------------
	protected bool UpdateRadios(bool forceUpdateCoverage = false)
	{
		m_bUpdateCoverage = false;

		foreach (SCR_CoverageRadioComponent radio : m_aRadioComponents)
		{
			UpdateRadioComponent(radio);
		}

		if (!m_bUpdateCoverage && !forceUpdateCoverage)
			return m_bUpdateCoverage;

		if (Replication.IsServer())
		{
			for (int i = 0, count = m_mEncryptionSignalSources.Count(); i < count; i++)
			{
				UpdateCoverage(m_mEncryptionSignalSources.GetKey(i));
			}
		}

		return m_bUpdateCoverage;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateRadioComponent(notnull SCR_CoverageRadioComponent component)
	{
		bool rangesHandled;
		string encryption = component.GetEncryptionKey();

		// Radio has been turned on or off
		if (!component.WasPowered() && component.IsPowered())
		{
			m_bUpdateCoverage = true;
			HandleRanges(component);
			component.OnPowerToggle(true);
			rangesHandled = true;

			if (Replication.IsServer() && component.IsSource() && !m_mEncryptionSignalSources.Get(encryption))
				m_mEncryptionSignalSources.Set(encryption, component);
		}
		else if (component.WasPowered() && !component.IsPowered())
		{
			m_bUpdateCoverage = true;
			component.OnPowerToggle(false);

			if (Replication.IsServer() && m_mEncryptionSignalSources.Get(encryption) == component)
				FindNewSource(encryption);
		}

		// Signal range has changed
		float currentRange = component.GetTransceiver(0).GetRange();

		if (currentRange != component.GetSavedRange())
		{
			m_bUpdateCoverage = true;
			component.OnRangeChanged(currentRange);

			if (!rangesHandled)
				HandleRanges(component, currentRange);
		}

		// Encryption key has changed
		string savedEncryption = component.GetSavedEncryption();

		if (encryption != savedEncryption)
		{
			m_bUpdateCoverage = true;

			if (Replication.IsServer() && m_mEncryptionSignalSources.Get(savedEncryption) == component)
				FindNewSource(savedEncryption);

			if (Replication.IsServer() && component.IsSource() && !m_mEncryptionSignalSources.Get(encryption))
				m_mEncryptionSignalSources.Set(encryption, component);

			component.OnEncryptionChanged(encryption);
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnCoverageChanged(notnull SCR_CoverageRadioComponent radio)
	{
		if (m_OnCoverageChanged)
			m_OnCoverageChanged.Invoke(radio);
	}

	//------------------------------------------------------------------------------------------------
	OnCoverageChangedInvoker GetOnCoverageChanged()
	{
		if (!m_OnCoverageChanged)
			m_OnCoverageChanged = new OnCoverageChangedInvoker();

		return m_OnCoverageChanged;
	}

	//------------------------------------------------------------------------------------------------
	void TogglePeriodicUpdates(bool toggle)
	{
		m_bForcedPause = !toggle;

		if (toggle)
			OnStarted();
		else
			OnStopped();
	}

	//------------------------------------------------------------------------------------------------
	void RegisterRadioComponent(notnull SCR_CoverageRadioComponent component, float range = -1)
	{
		if (m_aRadioComponents.Contains(component))
			return;

		m_aRadioComponents.Insert(component);

		if (!component.IsPowered())
			return;

		if (Replication.IsServer() && component.IsSource() && !m_mEncryptionSignalSources.Get(component.GetEncryptionKey()))
			m_mEncryptionSignalSources.Set(component.GetEncryptionKey(), component);

		HandleRanges(component, range);
	}

	//------------------------------------------------------------------------------------------------
	void UnregisterRadioComponent(notnull SCR_CoverageRadioComponent component)
	{
		m_aRadioComponents.RemoveItem(component);

		if (!GetGame().InPlayMode())
			return;

		string encryption = component.GetEncryptionKey();

		if (Replication.IsServer() && m_mEncryptionSignalSources.Get(encryption) == component)
			FindNewSource(encryption);

		UpdateRadios(true);
	}

	//------------------------------------------------------------------------------------------------
	protected void FindNewSource(string encryptionKey)
	{
		m_mEncryptionSignalSources.Set(encryptionKey, null);

		foreach (SCR_CoverageRadioComponent newSource : m_aRadioComponents)
		{
			if (!newSource.IsPowered() || !newSource.IsSource() || newSource.GetEncryptionKey() != encryptionKey)
				continue;

			m_mEncryptionSignalSources.Set(encryptionKey, newSource);
			break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleRanges(notnull SCR_CoverageRadioComponent component, float range = -1)
	{
		if (range < 0)
			range = component.GetTransceiver(0).GetRange();

		float rangeSq = range * range;

		vector position = component.GetOwner().GetOrigin();
		int otherRange;
		int distanceSq;

		foreach (SCR_CoverageRadioComponent radio : m_aRadioComponents)
		{
			if (radio == component || !radio.IsPowered())
				continue;

			distanceSq = vector.DistanceSqXZ(radio.GetOwner().GetOrigin(), position);

			if (distanceSq <= rangeSq)
			{
				component.AddRadioInRange(radio);
				radio.AddRadioInRangeOf(component);
			}
			else
			{
				component.RemoveRadioInRange(radio);
				radio.RemoveRadioInRangeOf(component);
			}

			otherRange = radio.GetTransceiver(0).GetRange();

			if (distanceSq <= (otherRange * otherRange))
			{
				radio.AddRadioInRange(component);
				component.AddRadioInRangeOf(radio);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateCoverage(string encryptionKey)
	{
		SCR_CoverageRadioComponent source = m_mEncryptionSignalSources.Get(encryptionKey);

		if (!source)
			return;

		// Signature changes each update so we know which radios have already been pinged to prevent infinite loops
		foreach (SCR_CoverageRadioComponent radio : m_aRadioComponents)
		{
			radio.PrepareCoverageUpdate(encryptionKey, m_bSignalSignature);
		}

		// Forward ping
		m_bSignalSignature = !m_bSignalSignature;
		source.Ping(encryptionKey, m_bSignalSignature);

		// Backward ping
		m_bSignalSignature = !m_bSignalSignature;
		source.Ping(encryptionKey, m_bSignalSignature, true);

		foreach (SCR_CoverageRadioComponent radio : m_aRadioComponents)
		{
			radio.FinishCoverageUpdate(encryptionKey);
		}
	}
}

enum SCR_ERadioCoverageStatus
{
	NONE,
	RECEIVE,
	SEND,
	BOTH_WAYS
}
