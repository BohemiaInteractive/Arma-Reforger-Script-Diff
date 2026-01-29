enum EResourceType
{
	INVALID = -1,
	SUPPLIES = 0,
	ELECTRICITY
};

enum EResourceRights
{
	NONE,
	SELF,
	SQUAD,
	FACTION,
	ALL
};

enum EResourceContainerStorageType
{
	ORPHAN,
	STORED,
	CARGO_PROP,
	CARGO_VEHICLE,
	CARGO_CHARACTER
};

enum EResourceContainerOnEmptyBehavior
{
	NONE,
	HIDE,
	DELETE,
};

[BaseContainerProps()]
class SCR_ResourceContainer : SCR_ResourceActor
{	
	// This is for the debugging visualization of containers
	protected ref set<SCR_ResourceInteractor> m_aInteractors = new set<SCR_ResourceInteractor>();
	float debugControlOffset = 0.25;
	
	[Attribute(defvalue: EResourceContainerStorageType.ORPHAN.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Resource consumption source order.", enums: ParamEnumArray.FromEnum(EResourceContainerStorageType),category: "Debugging")]
	protected EResourceContainerStorageType m_eStorageType;
	
	[Attribute(uiwidget: UIWidgets.SpinBox, params: string.Format("0.0 %1 1.0", float.MAX))]
	protected float m_fResourceValueCurrent;
	
	[Attribute(uiwidget: UIWidgets.SpinBox, params: string.Format("0.0 %1 1.0", float.MAX))]
	protected float m_fResourceValueMax;
	
	[Attribute(uiwidget: UIWidgets.CheckBox)]
	protected bool m_bEnableResourceGain;
	
	[Attribute(uiwidget: UIWidgets.SpinBox, params: string.Format("0.0 %1 1.0", float.MAX))]
	protected float m_fResourceGain;
	
	[Attribute(uiwidget: UIWidgets.SpinBox, params: string.Format("0.0 %1 1.0", float.MAX))]
	protected float m_fResourceGainTickrate;
	
	[Attribute(uiwidget: UIWidgets.SpinBox, params: string.Format("0.0 %1 1.0", float.MAX))]
	protected float m_fResourceGainTimeout;
	
	[Attribute(uiwidget: UIWidgets.CheckBox)]
	protected bool m_bEnableResourceDecay;
	
	[Attribute(uiwidget: UIWidgets.SpinBox, params: "0.0 inf 1.0")]
	protected float m_fResourceDecay;
	
	[Attribute(uiwidget: UIWidgets.SpinBox, params: string.Format("0.0 %1 1.0", float.MAX))]
	protected float m_fResourceDecayTickrate;
	
	[Attribute(uiwidget: UIWidgets.SpinBox, params: string.Format("0.0 %1 1.0", float.MAX))]
	protected float m_fResourceDecayTimeout;
	
	[Attribute(defvalue: EResourceContainerOnEmptyBehavior.NONE.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Sets the behavior of when the container resource value reaches 0.", enums: ParamEnumArray.FromEnum(EResourceContainerOnEmptyBehavior))]
	protected EResourceContainerOnEmptyBehavior m_eOnEmptyBehavior;
	
	protected bool m_bIsEncapsulated;
	
	protected WorldTimestamp m_LastUpdateTimestamp;
	protected float m_fResourceGainElapsedTime;
	protected float m_fResourceDecayElapsedTime;
	protected float m_fWeightMultiplier;
	
	protected ref ScriptInvoker m_OnResourcesChangedInvoker;
	protected ref ScriptInvoker m_OnMaxResourcesChangedInvoker;
	protected ref ScriptInvoker m_OnResourcesDepletedInvoker;
	protected ref ScriptInvoker m_OnResourcesMaxedOutInvoker;
	protected ref ScriptInvoker m_OnGainChangedInvoker;
	protected ref ScriptInvoker m_OnDecayChangedInvoker;
	protected ref ScriptInvoker m_OnGainEnabledChangedInvoker;
	protected ref ScriptInvoker m_OnDecayEnabledChangedInvoker;
	
	protected SCR_ResourceEncapsulator m_ResourceEncapsulator;
	
	//------------------------------------------------------------------------------------------------
	float GetResourceValue()
	{
		return m_fResourceValueCurrent;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetMaxResourceValue()
	{
		return m_fResourceValueMax;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetWeightMultiplier()
	{
		return m_fWeightMultiplier;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetResourceGain()
	{
		return m_fResourceGain;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetResourceDecay()
	{
		return m_fResourceDecay;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetResourceGainTickrate()
	{
		return m_fResourceGainTickrate;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetResourceDecayTickrate()
	{
		return m_fResourceDecayTickrate;
	}
	
	//------------------------------------------------------------------------------------------------	
	void GetBoundingVolume(inout vector mins, inout vector maxs)
	{
		if (m_Owner)
			m_Owner.GetBounds(mins, maxs);
	}
	
	//------------------------------------------------------------------------------------------------	
	void GetAxisAlignedBoundingVolume(inout vector mins, inout vector maxs)
	{
		if (!m_Owner)
			return;
		
		m_Owner.GetWorldBounds(mins, maxs);
		mins -= m_Owner.GetOrigin();
		maxs -= m_Owner.GetOrigin();
	}
	
	//------------------------------------------------------------------------------------------------
	EResourceContainerStorageType GetStorageType()
	{
		return m_eStorageType;
	}
	
	//------------------------------------------------------------------------------------------------
	EResourceType GetResourceType()
	{
		return m_eResourceType;
	}
	
	//------------------------------------------------------------------------------------------------
	EResourceContainerOnEmptyBehavior GetOnEmptyBehavior()
	{
		return m_eOnEmptyBehavior;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ResourceEncapsulator GetResourceEncapsulator()
	{
		return m_ResourceEncapsulator;
	}
	
	//------------------------------------------------------------------------------------------------
	set<SCR_ResourceInteractor> GetLinkedInteractors()
	{
		return m_aInteractors;
	}
	
	//------------------------------------------------------------------------------------------------
	array<SCR_ResourceInteractor> GetLinkedInteractorsCopy()
	{
		array<SCR_ResourceInteractor> result = new array<SCR_ResourceInteractor>();
		
		foreach (SCR_ResourceInteractor interactor: m_aInteractors)
		{
			result.Insert(interactor);
		}
		
		return result;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetLinkedInteractorsCount()
	{
		return m_aInteractors.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ResourceInteractor GetLinkedInteractorAt(int index)
	{
		return m_aInteractors[index];
	}
	
	//------------------------------------------------------------------------------------------------
	int GetLinkedInteractorIndex(notnull SCR_ResourceInteractor interactor)
	{
		return m_aInteractors.Find(interactor);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the timestamp that is aimed to be used for the computation of gain and decay.
	WorldTimestamp GetLastUpdateTimestamp()
	{
		return m_LastUpdateTimestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsInteractorLinked(notnull SCR_ResourceInteractor interactor)
	{
		return m_aInteractors.Contains(interactor);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsAllowed(notnull SCR_ResourceInteractor interactor)
	{
		if (interactor.GetResourceType() != m_eResourceType)
			return false;
		
		switch (m_eResourceRights)
		{
			case EResourceRights.NONE:
				return false;
			case EResourceRights.SELF:
				return m_Owner == interactor.GetOwner();
			case EResourceRights.SQUAD:
				// TODO: Logic for detecting the squad.
				return false;
			case EResourceRights.FACTION:
				FactionAffiliationComponent interactorFactionComponent = interactor.GetComponent().GetFactionAffiliationComponent();
				
				if (!interactorFactionComponent)
					return true;
				
				FactionAffiliationComponent containerFactionComponent = m_ResourceComponent.GetFactionAffiliationComponent();
				
				if (!containerFactionComponent)
					return true;
			
				Faction interactorFaction = interactorFactionComponent.GetAffiliatedFaction();
				Faction containerFaction = containerFactionComponent.GetAffiliatedFaction();
			
				if (!interactorFaction || !containerFaction)
					return true;
				
				return interactorFaction == containerFaction || interactorFaction.IsFactionFriendly(containerFaction);
			case EResourceRights.ALL:
				return true;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsIsolated()
	{
		return m_bIsEncapsulated || super.IsIsolated();
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsInRange(vector origin, float range)
	{
		vector mins, maxs;
		
		m_Owner.GetBounds(mins, maxs);
		
		return Math3D.IntersectionSphereAABB(origin - m_Owner.GetOrigin(), range, mins, maxs);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsResourceDecayEnabled()
	{
		return m_bEnableResourceDecay;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsResourceGainEnabled()
	{
		return m_bEnableResourceGain;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsEncapsulated()
	{
		return m_bIsEncapsulated;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool ShouldUpdate()
	{
		return super.ShouldUpdate() || m_bEnableResourceGain || m_bEnableResourceDecay;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsAllowedToStoreResource()
	{
		return m_eStorageType == EResourceContainerStorageType.STORED; 
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanStoreResource()
	{
		return	m_eStorageType == EResourceContainerStorageType.STORED
			&&	(m_fResourceValueMax - m_fResourceValueCurrent);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnResourcesChanged()
	{
		if (!m_OnResourcesChangedInvoker)
			m_OnResourcesChangedInvoker = new ScriptInvoker();
		
		return m_OnResourcesChangedInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnMaxResourcesChanged()
	{
		if (!m_OnMaxResourcesChangedInvoker)
			m_OnMaxResourcesChangedInvoker = new ScriptInvoker();
		
		return m_OnMaxResourcesChangedInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnResourcesDepleted()
	{
		if (!m_OnResourcesDepletedInvoker)
			m_OnResourcesDepletedInvoker = new ScriptInvoker();
		
		return m_OnResourcesDepletedInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnResourcesMaxedOut()
	{
		if (!m_OnResourcesMaxedOutInvoker)
			m_OnResourcesMaxedOutInvoker = new ScriptInvoker();
		
		return m_OnResourcesMaxedOutInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnGainChanged()
	{
		if (!m_OnGainChangedInvoker)
			m_OnGainChangedInvoker = new ScriptInvoker();
		
		return m_OnGainChangedInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDecayChanged()
	{
		if (!m_OnDecayChangedInvoker)
			m_OnDecayChangedInvoker = new ScriptInvoker();
		
		return m_OnDecayChangedInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnGainEnabledChanged()
	{
		if (!m_OnGainEnabledChangedInvoker)
			m_OnGainEnabledChangedInvoker = new ScriptInvoker();
		
		return m_OnGainEnabledChangedInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDecayEnabledChanged()
	{
		if (!m_OnDecayEnabledChangedInvoker)
			m_OnDecayEnabledChangedInvoker = new ScriptInvoker();
		
		return m_OnDecayEnabledChangedInvoker;
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Sets the timestamp that is aimed to be used for the computation of gain and decay.
	//!
	//! \param[in] timestamp the timestamp to set as last update timestamp.
	void SetLastUpdateTimestamp(WorldTimestamp timestamp)
	{
		m_LastUpdateTimestamp = timestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	//!	Sets the resource value of the container to the specified value, clamped to the maximum 
	//!		resource value.
	//!	
	//!	\param[in] value The resource value to set the container to.
	//!	\param[in] notifyChange Whenever or not changes in resource value should be notified for this 
	//!		instance.
	//!	\return true if any change in resource was actually performed, false otherwise.
	bool SetResourceValue(float value, bool notifyChange = true)
	{	
		if (!m_ResourceEncapsulator)
			return SetResourceValueUnsafe(value, notifyChange);
		
		float previousValue		= m_fResourceValueCurrent;
		float newValue			= Math.Clamp(value, 0.0, m_fResourceValueMax);
		
		if (newValue > previousValue)
			m_ResourceEncapsulator.RequestGeneration(newValue - previousValue, notifyChange);
		else if (newValue < previousValue)
			m_ResourceEncapsulator.RequestConsumtion(previousValue - newValue, notifyChange);
		else
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!	Unsafely sets the resource value of the container to the specified value, clamped to the 
	//!		maximum resource value.
	//!	
	//!	\warning Use with care, changing the resource value with this method expects you to take care 
	//!		of all the other factors externally, such as updating resource encapsulators and alike.
	//!	\param[in] value The resource value to set the container to.
	//!	\param[in] notifyChange Whenever or not changes in resource value should be notified for this 
	//!		instance.
	//!	\return true if any change in resource value was actually performed, false otherwise.
	bool SetResourceValueUnsafe(float value, bool notifyChange = true)
	{
		float previousValue		= m_fResourceValueCurrent;
		m_fResourceValueCurrent	= Math.Clamp(value, 0.0, m_fResourceValueMax);
		
		if (previousValue == m_fResourceValueCurrent)
			return false;
		
		if (notifyChange)
			OnResourcesChanged(previousValue);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool SetMaxResourceValue(float value, bool notifyChange = true)
	{
		float previousValue		= m_fResourceValueMax;
		m_fResourceValueMax	= Math.Max(value, 0.0);
		
		if (previousValue == m_fResourceValueMax)
			return false;
		
		if (notifyChange)
			OnMaxResourcesChanged(previousValue);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool SetResourceGain(float value, bool notifyChange = true)
	{
		float previousValue	= m_fResourceGain;
		m_fResourceGain		= Math.Clamp(value, 0.0, m_fResourceValueMax);
		
		if (previousValue == m_fResourceGain)
			return false;
		
		if (notifyChange)
			OnGainChanged(previousValue);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool SetResourceDecay(float value, bool notifyChange = true)
	{
		float previousValue	= m_fResourceDecay;
		m_fResourceDecay	= Math.Clamp(value, 0.0, m_fResourceValueMax);
		
		if (previousValue == m_fResourceDecay)
			return false;
		
		if (notifyChange)
			OnDecayChanged(previousValue);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetResourceGainTickrate(float tickrate)
	{
		m_fResourceGainTickrate = tickrate;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetResourceDecayTickrate(float tickrate)
	{
		m_fResourceDecayTickrate = tickrate;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetResourceGainTimeout(float timeout)
	{
		m_fResourceGainTimeout = timeout;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetResourceDecayTimeout(float timeout)
	{
		m_fResourceDecayTimeout = timeout;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetResourceEncapsulator(SCR_ResourceEncapsulator encapsulator)
	{
		m_ResourceEncapsulator = encapsulator;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IncreaseResourceValue(float value, bool notifyChange = true)
	{
		return SetResourceValue(m_fResourceValueCurrent + value, notifyChange);
	}

	//------------------------------------------------------------------------------------------------
	bool DecreaseResourceValue(float value, bool notifyChange = true)
	{
		return SetResourceValue(m_fResourceValueCurrent - value, notifyChange);
	}

	//------------------------------------------------------------------------------------------------
	bool MaxOutResourceValue(bool notifyChange = true)
	{
		return SetResourceValue(m_fResourceValueMax, notifyChange);
	}

	//------------------------------------------------------------------------------------------------
	bool DepleteResourceValue(bool notifyChange = true)
	{
		return SetResourceValue(0.0, notifyChange);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetOnEmptyBehavior(EResourceContainerOnEmptyBehavior behavior)
	{
		m_eOnEmptyBehavior = behavior;
	}
	
	//------------------------------------------------------------------------------------------------
	bool LinkInteractor(notnull SCR_ResourceInteractor interactor)
	{
		return m_aInteractors.Insert(interactor);
	}
	
	//------------------------------------------------------------------------------------------------
	bool UnlinkInteractor(notnull SCR_ResourceInteractor interactor)
	{
		return m_aInteractors.RemoveItem(interactor);
	}
	
	//------------------------------------------------------------------------------------------------
	void UnlinkEveryInteractor()
	{
		m_aInteractors.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	bool EnableGain(bool shouldEnable, bool notifyChange = true)
	{
		bool previousValue		= m_bEnableResourceGain;
		m_bEnableResourceGain	= shouldEnable;
		
		if (previousValue == m_bEnableResourceGain)
			return false;
		
		m_fResourceGainElapsedTime = 0.0;
		
		if (notifyChange)
			OnGainEnabledChanged(previousValue);
		
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		
		if (!world)
			return true;
		
		SCR_ResourceSystem updateSystem = SCR_ResourceSystem.Cast(world.FindSystem(SCR_ResourceSystem));
        
		if (!updateSystem)
			return true;
		
		if (m_bEnableResourceGain)
			updateSystem.RegisterContainer(this);
		else
			updateSystem.UnregisterContainer(this);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool EnableDecay(bool shouldEnable, bool notifyChange = true)
	{
		bool previousValue		= m_bEnableResourceDecay;
		m_bEnableResourceDecay	= shouldEnable;
		
		if (previousValue == m_bEnableResourceDecay)
			return false;
		
		m_fResourceDecayElapsedTime = 0.0;
		
		if (notifyChange)
			OnDecayEnabledChanged(previousValue);

		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		
		if (!world)
			return true;
		
		SCR_ResourceSystem updateSystem = SCR_ResourceSystem.Cast(world.FindSystem(SCR_ResourceSystem));
        
		if (!updateSystem)
			return true;
		
		if (m_bEnableResourceDecay)
			updateSystem.RegisterContainer(this);
		else
			updateSystem.UnregisterContainer(this);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsEncapsulated(bool shouldEnable)
	{
		m_bIsEncapsulated = shouldEnable;
		
		if (!m_bIsEncapsulated)
			return;
		
		SCR_ResourceInteractor interactor;
		
		for (int index = m_aInteractors.Count() - 1; index >= 0; --index)
		{
			interactor = m_aInteractors[index];
			
			if (SCR_ResourceEncapsulator.Cast(interactor))
				continue;
			
			interactor.UnregisterContainer(this);
		}
		
		if (!m_ResourceComponent)
			return;
		
		m_ResourceComponent.DeleteQueryInteractors();
		m_ResourceComponent.UnflagForProcessing();
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateWeightMultiplier()
	{
		if (m_fResourceValueMax == 0.0)
			return;
		
		m_fWeightMultiplier = m_fResourceValueCurrent / m_fResourceValueMax;
	}
	
	//------------------------------------------------------------------------------------------------
	void DebugDraw(bool shouldShowRange = true)
	{
		if (!m_Owner)
			return;
		
		string decaying;
		string gaining;
		
		if (m_bEnableResourceDecay)
			decaying = string.Format("\n   Decaying %1 every %2s   ", m_fResourceDecay, m_fResourceDecayTickrate);
		
		if (m_bEnableResourceGain)
			gaining = string.Format("\n   Gaining %1 every %2s   ", m_fResourceGain, m_fResourceGainTickrate);
		
		string infoText	= string.Format("   Cur: %1  Max: %2   \n   Storage: %3%4%5", GetResourceValue(), GetMaxResourceValue(), SCR_Enum.GetEnumName(EResourceContainerStorageType, m_eStorageType), decaying, gaining);
		vector origin	= m_Owner.GetOrigin();
		
		int textColor	= 0xFFFFFFFF;
		
		if (m_bIsEncapsulated)
			textColor	= 0xFF2222FF;
		
		// If empty, then the text color is red.
		if (m_fResourceValueCurrent == 0.0)
			textColor = 0xFFFF2222;
		
		DebugTextWorldSpace.Create(GetGame().GetWorld(), infoText, DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA | DebugTextFlags.ONCE, origin[0], origin[1], origin[2], 10, textColor, 0xFF000000);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Update(WorldTimestamp timestamp)
	{
		float resourceValue;
		const float timeslice = timestamp.DiffSeconds(m_LastUpdateTimestamp);
		m_LastUpdateTimestamp = timestamp;
		
		if (m_bEnableResourceGain)
			ComputeResourceGain(timeslice, resourceValue);
		
		if (m_bEnableResourceDecay)
			ComputeResourceDecay(timeslice, resourceValue);
		
		if (m_ResourceComponent && resourceValue != 0.0 && SetResourceValue(m_fResourceValueCurrent + resourceValue))
			m_ResourceComponent.Replicate();
	}

	//------------------------------------------------------------------------------------------------	
	float ComputeResourceDifference()
	{
		return m_fResourceValueMax - m_fResourceValueCurrent;
	}
	
	//------------------------------------------------------------------------------------------------	
	protected void ComputeResourceGain(float timeslice, out float resourceValue)
	{
		m_fResourceGainElapsedTime += timeslice;
		
		const float resourceGainElapsedTimeRelative = m_fResourceGainElapsedTime - m_fResourceGainTimeout;
		
		if (m_fResourceGainElapsedTime < m_fResourceGainTimeout 
		||	resourceGainElapsedTimeRelative < m_fResourceGainTickrate 
		||	m_fResourceGainTickrate <= 0.0)
			return;
		
		resourceValue				+= m_fResourceGain * (int)(resourceGainElapsedTimeRelative / m_fResourceGainTickrate);
		m_fResourceGainElapsedTime	= m_fResourceGainTimeout;
	}
	
	//------------------------------------------------------------------------------------------------	
	protected void ComputeResourceDecay(float timeslice, out float resourceValue)
	{
		m_fResourceDecayElapsedTime += timeslice;
		
		const float resourceDecayElapsedTimeRelative = m_fResourceDecayElapsedTime - m_fResourceDecayTimeout;
		
		if (m_fResourceDecayElapsedTime < m_fResourceDecayTimeout 
		||	resourceDecayElapsedTimeRelative < m_fResourceDecayTickrate 
		||	m_fResourceDecayTickrate <= 0.0)
			return;
		
		resourceValue				-= m_fResourceDecay * (int)(resourceDecayElapsedTimeRelative / m_fResourceDecayTickrate);
		m_fResourceDecayElapsedTime	= m_fResourceDecayTimeout;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnResourcesChanged(float previousValue)
	{
		UpdateWeightMultiplier();
		
		if (m_OnResourcesChangedInvoker)
			m_OnResourcesChangedInvoker.Invoke(this, previousValue);
		
		foreach (SCR_ResourceInteractor interactor: m_aInteractors)
		{
			if (interactor && interactor != m_ResourceEncapsulator)
				interactor.UpdateContainerResourceValue(this, previousValue);
		}
		
		if (previousValue < m_fResourceValueCurrent)
			OnResourcesIncreased(previousValue);
		else
			OnResourcesDecreased(previousValue);

		// Gameplay actions not marking for saving hotfix.
		// Decouple this from Resources after 1.0 and move it to Editor
		if (!m_Owner)
			return;

		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(m_Owner.FindComponent(SCR_EditableEntityComponent));
		if (!editableEntity)
			return;

		editableEntity.SetHierarchyAsDirtyInParents();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnResourcesIncreased(float previousValue)
	{
		// TODO: Perhaps add an invoker here as well?.
		if (m_fResourceValueCurrent == m_fResourceValueMax)
			OnResourcesMaxedOut(previousValue);
		
		if (previousValue != 0.0)
			return;
		
		switch (m_eOnEmptyBehavior)
		{
			case EResourceContainerOnEmptyBehavior.NONE:
				break;
			case EResourceContainerOnEmptyBehavior.HIDE:
				if (m_ResourceComponent)
					m_ResourceComponent.SetIsVisible(true);
				
				break;
			case EResourceContainerOnEmptyBehavior.DELETE:
				break;
			default:
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnResourcesDecreased(float previousValue)
	{
		// TODO: Perhaps add an invoker here as well?.
		if (m_fResourceValueCurrent == 0.0)
			OnResourcesDepleted(previousValue);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnResourcesDepleted(float previousValue)
	{
		if (m_OnResourcesDepletedInvoker)
			m_OnResourcesDepletedInvoker.Invoke(this, previousValue);
		
		switch (m_eOnEmptyBehavior)
		{
			case EResourceContainerOnEmptyBehavior.NONE:
				break;
			case EResourceContainerOnEmptyBehavior.HIDE:
				if (m_ResourceComponent)
					m_ResourceComponent.SetIsVisible(false);
			
				break;
			case EResourceContainerOnEmptyBehavior.DELETE:
				RplComponent.DeleteRplEntity(m_Owner, false);
				
				break;
			default:
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnResourcesMaxedOut(float previousValue)
	{
		if (m_OnResourcesMaxedOutInvoker)
			m_OnResourcesMaxedOutInvoker.Invoke(this, previousValue);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMaxResourcesChanged(float previousValue)
	{
		UpdateWeightMultiplier();
		
		if (m_OnMaxResourcesChangedInvoker)
			m_OnMaxResourcesChangedInvoker.Invoke(this, previousValue);
		
		foreach (SCR_ResourceInteractor interactor: m_aInteractors)
		{
			if (interactor)
				interactor.UpdateContainerMaxResourceValue(this, previousValue);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnGainChanged(float previousValue)
	{
		if (m_OnGainChangedInvoker)
			m_OnGainChangedInvoker.Invoke(this, previousValue);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDecayChanged(float previousValue)
	{
		if (m_OnDecayChangedInvoker)
			m_OnDecayChangedInvoker.Invoke(this, previousValue);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnGainEnabledChanged(float previousValue)
	{
		if (m_OnGainEnabledChangedInvoker)
			m_OnGainEnabledChangedInvoker.Invoke(this, previousValue);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDecayEnabledChanged(float previousValue)
	{
		if (m_OnDecayEnabledChangedInvoker)
			m_OnDecayEnabledChangedInvoker.Invoke(this, previousValue);
	}
	
	//------------------------------------------------------------------------------------------------
	void CopyFromContainer(notnull SCR_ResourceContainer container)
	{
		m_sDebugName				= container.m_sDebugName;
		m_eResourceRights			= container.m_eResourceRights;
		m_eResourceType				= container.m_eResourceType;
		m_eStorageType				= container.m_eStorageType;
		m_fResourceValueCurrent		= container.m_fResourceValueCurrent;
		m_fResourceValueMax			= container.m_fResourceValueMax;
		m_bEnableResourceGain		= container.m_bEnableResourceGain;
		m_fResourceGain				= container.m_fResourceGain;
		m_fResourceGainTickrate		= container.m_fResourceGainTickrate;
		m_fResourceGainTimeout		= container.m_fResourceGainTimeout;
		m_bEnableResourceDecay		= container.m_bEnableResourceDecay;
		m_fResourceDecay			= container.m_fResourceDecay;
		m_fResourceDecayTickrate	= container.m_fResourceDecayTickrate;
		m_fResourceDecayTimeout		= container.m_fResourceDecayTimeout;
		m_eOnEmptyBehavior			= container.m_eOnEmptyBehavior;
	}
	
	//------------------------------------------------------------------------------------------------
	void Initialize(notnull IEntity owner, notnull SCR_ResourceContainer container)
	{
		m_Owner					= owner;
		m_ResourceComponent		= SCR_ResourceComponent.Cast(owner.FindComponent(SCR_ResourceComponent));
		IEntity parentEntity	= m_Owner.GetParent();
		
		CopyFromContainer(container);
		
		SCR_ResourceEncapsulator encapsulator;
		SCR_ResourceComponent parentResourceComponent;
		
		/*!
		Finds the top most faction affiliation component of the parents, if any and also finds the
			first resource encapsulator from parent to parent that can register the container, then
			registers the container into that encapsulator.
		*/
		while (parentEntity)
		{
			if (!encapsulator)
			{	
				parentResourceComponent = SCR_ResourceComponent.Cast(parentEntity.FindComponent(SCR_ResourceComponent));
				
				if (parentResourceComponent)
					encapsulator = parentResourceComponent.GetEncapsulator(m_eResourceType);
				
				if (encapsulator)
					encapsulator.RegisterContainer(this);
			}
			
			parentEntity = parentEntity.GetParent();
		}
		
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		
		if (!world)
			return;
		
		if (m_fResourceValueCurrent == 0.0 && !world.IsEditMode() && m_eOnEmptyBehavior == EResourceContainerOnEmptyBehavior.HIDE)
			m_ResourceComponent.SetIsVisible(false);
		
		if (!m_ResourceComponent.GetReplicationComponent() || m_ResourceComponent.GetReplicationComponent().IsProxy())
			return;
		
		SCR_ResourceSystem updateSystem = SCR_ResourceSystem.Cast(world.FindSystem(SCR_ResourceSystem));
        
		if (!updateSystem)
			return;
		
		if (m_bEnableResourceGain || m_bEnableResourceDecay)
			updateSystem.RegisterContainer(this);
		else
			updateSystem.UnregisterContainer(this);
		
		if (!IsIsolated())
			m_ResourceComponent.FlagForProcessing();
	}
	
	//------------------------------------------------------------------------------------------------
	override void Clear()
	{
		super.Clear();
		
		// Reverse iter needed  due to self delete operations during UnregisterContainer
		for (int index = m_aInteractors.Count() - 1; index >= 0; --index)
		{
			SCR_ResourceInteractor interactor = m_aInteractors[index];
			if (interactor)
				interactor.UnregisterContainer(this);
		}

		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return;

		SCR_ResourceSystem updateSystem = SCR_ResourceSystem.Cast(world.FindSystem(SCR_ResourceSystem));
		if (updateSystem)
			updateSystem.UnregisterContainer(this);
	}
}
