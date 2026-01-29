void ScriptInvokerServiceUnregisteredMethod(SCR_MilitaryBaseComponent militaryBaseComponent, SCR_ServicePointComponent serviceComponent);
typedef func ScriptInvokerServiceUnregisteredMethod;
typedef ScriptInvokerBase<ScriptInvokerServiceUnregisteredMethod> ScriptInvokerServiceUnregistered;

class SCR_MilitaryBaseComponentClass : ScriptComponentClass
{
	static override bool DependsOn(string className)
	{
		return className.ToType().IsInherited(FactionAffiliationComponent);
	}

	static override array<typename> Requires(IEntityComponentSource src)
	{
		return {FactionAffiliationComponent};
	}
}

class SCR_MilitaryBaseComponent : ScriptComponent
{
	[Attribute("100"), RplProp(onRplName: "OnRadiusChanged")]
	protected int m_iRadius;

	[Attribute("0", desc: "Not all capture points controlled are required for seizing this base, only the majority.")]
	protected bool m_bSeizedByMajority;

	[Attribute("1")]
	protected bool m_bShowNotifications;

	[Attribute("1")]
	protected bool m_bShowMapIcon;

	[Attribute(ENotification.BASE_SEIZING_DONE_FRIENDLIES.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ENotification))]
	protected ENotification m_eCapturedByFriendliesNotification;

	[Attribute(ENotification.BASE_SEIZING_DONE_ENEMIES.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ENotification))]
	protected ENotification m_eCapturedByEnemiesNotification;

	static const int INVALID_BASE_CALLSIGN = -1;

	protected int m_iCallsignSignal = INVALID_BASE_CALLSIGN;

	protected string m_sCallsign;
	protected string m_sCallsignUpper;
	protected string m_sCallsignNameOnly;
	protected string m_sCallsignNameOnlyUC;

	protected ref ScriptInvoker m_OnRadiusChanged;
	protected ref ScriptInvoker m_OnServiceRegistered;
	protected ref ScriptInvokerServiceUnregistered m_OnServiceUnregistered;

	protected ref array<SCR_MilitaryBaseLogicComponent> m_aSystems = {};
	protected ref array<SCR_ServicePointDelegateComponent> m_aServiceDelegates = {};

	protected RplComponent m_RplComponent;
	protected SCR_FactionAffiliationComponent m_FactionComponent;

	[RplProp(onRplName: "OnCapturingFactionChanged")]
	protected FactionKey m_sCapturingFaction;

	[RplProp(onRplName: "OnCallsignAssigned")]
	protected int m_iCallsign = INVALID_BASE_CALLSIGN;

	protected ref ScriptInvoker<SCR_MilitaryBaseLogicComponent> m_OnMilitaryBaseRegistered;
	protected ref ScriptInvoker<SCR_MilitaryBaseLogicComponent> m_OnMilitaryBaseUnregistered;

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnMilitaryBaseRegistered()
	{
		if (!m_OnMilitaryBaseRegistered)
			m_OnMilitaryBaseRegistered = new ScriptInvoker();

		return m_OnMilitaryBaseRegistered;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnMilitaryBaseUnregistered()
	{
		if (!m_OnMilitaryBaseUnregistered)
			m_OnMilitaryBaseUnregistered = new ScriptInvoker();

		return m_OnMilitaryBaseUnregistered;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}

	//------------------------------------------------------------------------------------------------
	//! Show default notifications upon base capture (setter)
	//! \param[in] allow
	void AllowNotifications(bool allow)
	{
		m_bShowNotifications = allow;
	}

	//------------------------------------------------------------------------------------------------
	//! Show default notifications upon base capture (getter)
	//! \return
	bool NotificationsAllowed()
	{
		return m_bShowNotifications;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] index
	void SetCallsignIndexAutomatic(int index)
	{
		m_iCallsign = index;
		Replication.BumpMe();
		OnCallsignAssigned();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] index
	void SetCallsignIndex(int index)
	{
		m_iCallsign = index;
		Replication.BumpMe();
		OnCallsignAssigned();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnRadiusChanged()
	{
		if (!m_OnRadiusChanged)
			m_OnRadiusChanged = new ScriptInvoker();

		return m_OnRadiusChanged;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnServiceRegistered()
	{
		if (!m_OnServiceRegistered)
			m_OnServiceRegistered = new ScriptInvoker();

		return m_OnServiceRegistered;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerServiceUnregistered GetOnServiceUnregistered()
	{
		if (!m_OnServiceUnregistered)
			m_OnServiceUnregistered = new ScriptInvokerServiceUnregistered();

		return m_OnServiceUnregistered;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	void SetCallsign(notnull SCR_Faction faction)
	{
		if (m_iCallsign == INVALID_BASE_CALLSIGN)
			return;

		SCR_MilitaryBaseCallsign callsignInfo = faction.GetBaseCallsignByIndex(m_iCallsign);

		if (!callsignInfo)
			return;

		m_sCallsign = callsignInfo.GetCallsign();
		m_sCallsignNameOnly = callsignInfo.GetCallsignShort();
		m_sCallsignNameOnlyUC = callsignInfo.GetCallsignUpperCase();
		m_iCallsignSignal = callsignInfo.GetSignalIndex();
	}


	//------------------------------------------------------------------------------------------------
	//! \return
	int GetCallsign()
	{
		return m_iCallsign;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetCallsignSignal()
	{
		return m_iCallsignSignal;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void OnCallsignAssigned()
	{
		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
		if (faction)
		{
			SetCallsign(faction);
			return;
		}

		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return;

		SCR_PlayerFactionAffiliationComponent playerFactionAff = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));
		if (!playerFactionAff)
			return;

		playerFactionAff.GetOnPlayerFactionChangedInvoker().Insert(OnPlayerFactionChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerFactionChanged(SCR_PlayerFactionAffiliationComponent component, Faction previous, Faction current)
	{
		SCR_Faction faction = SCR_Faction.Cast(current);
		if (!faction)
			return;

		SetCallsign(faction);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return callsign name only (eg. "Matros" instead of "Point Matros")
	LocalizedString GetCallsignDisplayNameOnly()
	{
		return m_sCallsignNameOnly;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	LocalizedString GetCallsignDisplayName()
	{
		return m_sCallsign;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	LocalizedString GetCallsignDisplayNameOnlyUC()
	{
		return m_sCallsignNameOnlyUC;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	LocalizedString GetCallsignDisplayNameUpperCase()
	{
		return m_sCallsignUpper;
	}

	//------------------------------------------------------------------------------------------------
	//! Get all registered systems inherited from SCR_ServicePointComponent
	//! \param[out] services
	//! \return
	int GetServices(out array<SCR_ServicePointComponent> services = null)
	{
		int count;

		foreach (SCR_MilitaryBaseLogicComponent component : m_aSystems)
		{
			SCR_ServicePointComponent service = SCR_ServicePointComponent.Cast(component);

			if (service)
			{
				count++;

				if (services)
					services.Insert(service);
			}
		}

		return count;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] services
	//! \param[in] type
	//! \return
	int GetServicesByType(out array<SCR_ServicePointComponent> services, SCR_EServicePointType type)
	{
		int count;

		foreach (SCR_MilitaryBaseLogicComponent component : m_aSystems)
		{
			SCR_ServicePointComponent service = SCR_ServicePointComponent.Cast(component);

			if (service && service.GetType() == type)
			{
				count++;

				if (services)
					services.Insert(service);
			}
		}

		return count;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] label
	//! \return
	SCR_ServicePointComponent GetServiceByLabel(EEditableEntityLabel label)
	{
		foreach (SCR_MilitaryBaseLogicComponent component : m_aSystems)
		{
			SCR_ServicePointComponent service = SCR_ServicePointComponent.Cast(component);

			if (service && service.GetLabel() == label)
				return service;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] type
	//! \return
	SCR_ServicePointComponent GetServiceByType(SCR_EServicePointType type)
	{
		foreach (SCR_MilitaryBaseLogicComponent component : m_aSystems)
		{
			SCR_ServicePointComponent service = SCR_ServicePointComponent.Cast(component);

			if (service && service.GetType() == type)
				return service;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] delegates
	//! \return
	int GetServiceDelegates(out array<SCR_ServicePointDelegateComponent> delegates = null)
	{
		int count;

		foreach (SCR_ServicePointDelegateComponent delegate : m_aServiceDelegates)
		{
			if (delegate)
			{
				count++;

				if (delegates)
					delegates.Insert(delegate);
			}
		}

		return count;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] delegates
	//! \param[in] type
	//! \return
	int GetServiceDelegatesByType(out array<SCR_ServicePointDelegateComponent> delegates, SCR_EServicePointType type)
	{
		int count;

		foreach (SCR_ServicePointDelegateComponent delegate : m_aServiceDelegates)
		{
			if (delegate && delegate.GetType() == type)
			{
				count++;

				if (delegates)
					delegates.Insert(delegate);
			}
		}

		return count;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] label
	//! \return
	SCR_ServicePointDelegateComponent GetServiceDelegateByLabel(EEditableEntityLabel label)
	{
		foreach (SCR_ServicePointDelegateComponent delegate : m_aServiceDelegates)
		{
			if (delegate && delegate.GetLabel() == label)
				return delegate;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] type
	//! \return
	SCR_ServicePointDelegateComponent GetServiceDelegateByType(SCR_EServicePointType type)
	{
		foreach (SCR_ServicePointDelegateComponent delegate : m_aServiceDelegates)
		{
			if (delegate && delegate.GetType() == type)
				return delegate;
		}

		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get all registered systems inherited from SCR_CampaignBuildingProviderComponent
	int GetBuildingProviders(out array<SCR_CampaignBuildingProviderComponent> providers)
	{
		int count;
		SCR_CampaignBuildingProviderComponent provider;
		
		providers.Clear();
		
		foreach (SCR_MilitaryBaseLogicComponent component : m_aSystems)
		{
			provider = SCR_CampaignBuildingProviderComponent.Cast(component);

			if (provider)
			{
				count++;

				if (providers)
					providers.Insert(provider);
			}
		}

		return count;
	}

	//------------------------------------------------------------------------------------------------
	//! Get all registered systems inherited from SCR_SeizingComponent
	//! \param[out] capturePoints
	//! \return
	int GetCapturePoints(out array<SCR_SeizingComponent> capturePoints)
	{
		int count;

		foreach (SCR_MilitaryBaseLogicComponent component : m_aSystems)
		{
			SCR_SeizingComponent capturePoint = SCR_SeizingComponent.Cast(component);

			if (capturePoint)
			{
				count++;

				if (capturePoints)
					capturePoints.Insert(capturePoint);
			}
		}

		return count;
	}

	//------------------------------------------------------------------------------------------------
	//! Get all registered systems inherited from SCR_FlagComponent
	//! \param[out] flags
	//! \return
	int GetFlags(out array<SCR_FlagComponent> flags)
	{
		int count;

		foreach (SCR_MilitaryBaseLogicComponent component : m_aSystems)
		{
			SCR_FlagComponent flag = SCR_FlagComponent.Cast(component);

			if (flag)
			{
				count++;

				if (flags)
					flags.Insert(flag);
			}
		}

		return count;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] radius
	void SetRadius(int radius)
	{
		if (IsProxy())
			return;

		if (radius == m_iRadius)
			return;

		if (radius <= 0)
		{
			Print("SCR_MilitaryBaseComponent: SetRadius called with suspicious value (" + radius + ").", LogLevel.WARNING);
			radius = 0;
		}

		m_iRadius = radius;

		Replication.BumpMe();
		OnRadiusChanged();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRadiusChanged()
	{
		if (m_OnRadiusChanged)
			m_OnRadiusChanged.Invoke(m_iRadius, this);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetRadius()
	{
		return m_iRadius;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	void SetFaction(Faction faction)
	{
		if (IsProxy())
			return;

		if (!m_FactionComponent)
			return;

		m_FactionComponent.SetAffiliatedFaction(faction);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	void SetFaction(FactionKey faction)
	{
		if (IsProxy())
			return;

		if (!m_FactionComponent)
			return;

		m_FactionComponent.SetAffiliatedFactionByKey(faction);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFactionChanged(FactionAffiliationComponent owner, Faction previousFaction, Faction faction)
	{
		if (!GetGame().InPlayMode())
			return;

		if (!IsProxy())
			m_sCapturingFaction = FactionKey.Empty;

		foreach (SCR_MilitaryBaseLogicComponent comp : m_aSystems)
		{
			if (!comp)
				continue;

			comp.OnBaseFactionChanged(faction);
		}

		SCR_MilitaryBaseSystem.GetInstance().OnBaseFactionChanged(faction, this);
		if (!faction)
			return;

		ChangeFlags(faction);

		if (m_bShowNotifications)
			NotifyPlayerInRadius(faction);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] checkDefaultFaction
	//! \return
	Faction GetFaction(bool checkDefaultFaction = false)
	{
		if (!m_FactionComponent)
			return null;

		Faction faction = m_FactionComponent.GetAffiliatedFaction();

		if (!faction && checkDefaultFaction)
			faction = m_FactionComponent.GetDefaultAffiliatedFaction();

		return faction;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	Faction GetCapturingFaction()
	{
		FactionManager fManager = GetGame().GetFactionManager();
		
		if (!fManager)
			return null;
		
		return fManager.GetFactionByKey(m_sCapturingFaction);
	}

	//------------------------------------------------------------------------------------------------
	//! Called every time the status of the registered service has changed.
	//! \param[in] state
	//! \param[in] serviceComponent
	// To be overridden in inherited classes
	void OnServiceStateChanged(SCR_EServicePointStatus state, notnull SCR_ServicePointComponent serviceComponent)
	{

	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] delegate
	void RegisterServiceDelegate(notnull SCR_ServicePointDelegateComponent delegate)
	{
		m_aServiceDelegates.Insert(delegate);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] delegate
	void UnregisterServiceDelegate(notnull SCR_ServicePointDelegateComponent delegate)
	{
		m_aServiceDelegates.RemoveItem(delegate);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] component
	void RegisterLogicComponent(notnull SCR_MilitaryBaseLogicComponent component)
	{
		if (m_aSystems.Contains(component))
			return;

		m_aSystems.Insert(component);

		SCR_ServicePointComponent service = SCR_ServicePointComponent.Cast(component);

		if (service)
		{
			if (m_OnServiceRegistered)
				m_OnServiceRegistered.Invoke(this, service);

			OnServiceStateChanged(service.GetServiceState(), service);
			service.GetOnServiceStateChanged().Insert(OnServiceStateChanged);
		}

		SCR_MilitaryBaseSystem.GetInstance().OnLogicRegisteredInBase(component, this);

		SCR_FlagComponent flag = SCR_FlagComponent.Cast(component);

		if (flag && GetFaction())
			GetGame().GetCallqueue().CallLater(UpdateFlags, 500, false);	// Give the system time to properly register slots

		if (!IsProxy())
		{
			SCR_SeizingComponent seizingComponent = SCR_SeizingComponent.Cast(component);

			if (seizingComponent)
			{
				seizingComponent.GetOnCaptureFinish().Insert(OnPointCaptured);
				seizingComponent.GetOnCaptureStart().Insert(OnPointContested);
				seizingComponent.GetOnCaptureInterrupt().Insert(OnPointSecured);
			}
		}

		if (m_OnMilitaryBaseRegistered)
			m_OnMilitaryBaseRegistered.Invoke(component);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] component
	void UnregisterLogicComponent(notnull SCR_MilitaryBaseLogicComponent component)
	{
		if (!m_aSystems.Contains(component))
			return;

		m_aSystems.RemoveItem(component);

		SCR_ServicePointComponent service = SCR_ServicePointComponent.Cast(component);
		SCR_MilitaryBaseSystem baseManager = SCR_MilitaryBaseSystem.GetInstance();

		if (baseManager)
			baseManager.OnLogicUnregisteredInBase(component, this);

		if (service && baseManager)
		{
			service.GetOnServiceStateChanged().Remove(OnServiceStateChanged);

			if (m_OnServiceUnregistered)
				m_OnServiceUnregistered.Invoke(this, service);
		}

		if (!IsProxy())
		{
			SCR_SeizingComponent seizingComponent = SCR_SeizingComponent.Cast(component);

			if (seizingComponent)
			{
				seizingComponent.GetOnCaptureFinish().Remove(OnPointCaptured);
				seizingComponent.GetOnCaptureStart().Remove(OnPointContested);
				seizingComponent.GetOnCaptureInterrupt().Remove(OnPointSecured);
			}
		}

		if (m_OnMilitaryBaseUnregistered)
			m_OnMilitaryBaseUnregistered.Invoke(component);
	}

	//------------------------------------------------------------------------------------------------
	//! Triggered when a registered SCR_SeizingComponent gets captured
	//! \param[in] faction
	//! \param[in] point
	void OnPointCaptured(notnull SCR_Faction faction, notnull SCR_SeizingComponent point)
	{
		if (faction == GetFaction())
			return;

		array<SCR_SeizingComponent> capturePoints = {};
		int pointsCount = GetCapturePoints(capturePoints);
		int controlled;

		foreach (SCR_SeizingComponent capturePoint : capturePoints)
		{
			if (capturePoint == point || capturePoint.GetFaction() == faction)
				controlled++;
		}

		// Capture this base if the required amount of registered capture points has been seized
		if (pointsCount == controlled || (m_bSeizedByMajority && controlled > (pointsCount - controlled)))
			OnRequiredPointsCaptured(faction);
	}

	//------------------------------------------------------------------------------------------------
	//! Triggered when a registered SCR_SeizingComponent starts getting captured
	//! \param[in] faction
	//! \param[in] point
	void OnPointContested(notnull SCR_Faction faction, notnull SCR_SeizingComponent point)
	{
		if (faction == GetFaction())
			return;

		array<SCR_SeizingComponent> capturePoints = {};
		int pointsCount = GetCapturePoints(capturePoints);
		int controlled;

		foreach (SCR_SeizingComponent capturePoint : capturePoints)
		{
			if (capturePoint.GetFaction() == faction)
				controlled++;
		}

		// This base is currently being seized
		if (controlled + 1 == pointsCount || (m_bSeizedByMajority && controlled + 1 > (pointsCount - controlled)))
		{
			m_sCapturingFaction = faction.GetFactionKey();
			OnCapturingFactionChanged();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Triggered when a registered SCR_SeizingComponent stops getting captured
	//! \param[in] faction
	//! \param[in] point
	void OnPointSecured(notnull SCR_Faction faction, notnull SCR_SeizingComponent point)
	{
		if (faction != GetCapturingFaction())
			return;

		array<SCR_SeizingComponent> capturePoints = {};
		int pointsCount = GetCapturePoints(capturePoints);
		int controlled;

		foreach (SCR_SeizingComponent capturePoint : capturePoints)
		{
			if (capturePoint.GetFaction() == faction)
				controlled++;
		}

		// This base is no longer being seized
		if (controlled + 1 == pointsCount || (m_bSeizedByMajority && controlled + 1 > (pointsCount - controlled)))
		{
			m_sCapturingFaction = FactionKey.Empty;
			OnCapturingFactionChanged();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Triggered when a the required amount of SCR_SeizingComponents to seize the base is captured
	//! \param[in] faction
	void OnRequiredPointsCaptured(notnull SCR_Faction faction)
	{
		SetFaction(faction);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void OnCapturingFactionChanged()
	{
		foreach (SCR_MilitaryBaseLogicComponent comp : m_aSystems)
		{
			if (!comp)
				continue;

			comp.OnCapturingFactionChanged(m_sCapturingFaction);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateFlags()
	{
		Faction faction = GetFaction();
		if (!faction)
			return;

		ChangeFlags(faction);
	}

	//------------------------------------------------------------------------------------------------
	protected void ChangeFlags(notnull Faction faction)
	{
		SCR_Faction factionCast = SCR_Faction.Cast(faction);

		if (!factionCast)
			return;

		array<SCR_FlagComponent> flags = {};
		GetFlags(flags);

		foreach (SCR_FlagComponent flagComponent : flags)
		{
			flagComponent.ChangeMaterial(factionCast.GetFactionFlagMaterial());
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetIsLocalPlayerPresent()
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(SCR_PlayerController.GetLocalPlayerId());

		if (!player)
			return false;

		return GetIsEntityPresent(player);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entity
	//! \return
	bool GetIsEntityPresent(notnull IEntity entity)
	{
		return (vector.DistanceSqXZ(entity.GetOrigin(), GetOwner().GetOrigin()) <= (m_iRadius * m_iRadius));
	}

	//------------------------------------------------------------------------------------------------
	protected void NotifyPlayerInRadius(notnull Faction faction)
	{
		Faction playerFaction = SCR_FactionManager.SGetLocalPlayerFaction();

		if (!playerFaction)
			return;

		if (!GetIsLocalPlayerPresent())
			return;

		if (playerFaction == faction)
			SCR_NotificationsComponent.SendLocal(m_eCapturedByFriendliesNotification);
		else
			SCR_NotificationsComponent.SendLocal(m_eCapturedByEnemiesNotification);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		// Check for play mode again in case init event was set from outside of this class
		if (!GetGame().InPlayMode())
			return;

		SCR_MilitaryBaseSystem baseManager = SCR_MilitaryBaseSystem.GetInstance();

		if (baseManager)
			baseManager.RegisterBase(this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		m_FactionComponent = SCR_FactionAffiliationComponent.Cast(owner.FindComponent(SCR_FactionAffiliationComponent));

		// Attributes check
		if (!m_RplComponent)
			Print("SCR_MilitaryBaseComponent: RplComponent not found on owner entity. Multiplayer functionality will not be available.", LogLevel.WARNING);

		if (!m_FactionComponent)
			Print("SCR_MilitaryBaseComponent: SCR_FactionAffiliationComponent not found on owner entity. Faction handling will not be available.", LogLevel.WARNING);

		if (m_iRadius <= 0)
		{
			Print("SCR_MilitaryBaseComponent: m_iRadius set to suspicious value (" + m_iRadius + ").", LogLevel.WARNING);
			m_iRadius = 0;
		}

		if (!GetGame().InPlayMode())
			return;

		if (m_FactionComponent)
			m_FactionComponent.GetOnFactionChanged().Insert(OnFactionChanged);

		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_MilitaryBaseComponent()
	{
		SCR_MilitaryBaseSystem baseManager = SCR_MilitaryBaseSystem.GetInstance();

		if (baseManager)
			baseManager.UnregisterBase(this);

		foreach (SCR_MilitaryBaseLogicComponent component : m_aSystems)
		{
			if (component)
				component.UnregisterBase(this);
		}
	}
}
