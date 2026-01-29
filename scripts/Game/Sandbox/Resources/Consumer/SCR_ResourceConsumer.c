[BaseContainerProps()]
class SCR_ResourceActor : ScriptAndConfig
{
	static const float RESOURCES_LOWER_LIMIT	= 0.0;
	protected static const float UPDATE_PERIOD = 10.0 / 60.0;
	protected static const int CODEC_GENERATOR_PACKET_BYTESIZE = 32;
	
	[Attribute(defvalue: string.Empty, uiwidget: UIWidgets.EditBox, desc: "Identifier for debug prints", category: "Debugging")]
	string m_sDebugName;
	
	[Attribute(defvalue: EResourceRights.NONE.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Limits the taking of resources to a specific group", enums: ParamEnumArray.FromEnum(EResourceRights))]
	protected EResourceRights m_eResourceRights;
	
	[Attribute(defvalue: EResourceType.SUPPLIES.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Sets the type of Resource to be used.\nOnly a transaction matching Resource types can be successfully concluded.", enums: ParamEnumArray.FromEnum(EResourceType))]
	protected EResourceType m_eResourceType;
	
	protected SCR_ResourceComponent	m_ResourceComponent;
	protected IEntity m_Owner;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	Color GetDebugColor()
	{
		return Color.FromInt(m_ResourceComponent.GetDebugColor().PackToInt());
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetGridUpdateId()
	{
		return m_ResourceComponent.GetGridUpdateId();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetOwnerOrigin()
	{
		// TODO: Make it so that the nullity of this never happen in the first place.
		if (m_Owner)
			return m_Owner.GetOrigin();
		
		return vector.Zero;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	string GetDebugName()
	{
		return m_sDebugName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetDebugNameHash()
	{
		return m_sDebugName.Hash();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ResourceComponent GetComponent()
	{
		return m_ResourceComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	IEntity GetOwner()
	{
		return m_Owner;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	EResourceRights GetResourceRight()
	{
		return m_eResourceRights;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool ShouldUpdate()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsIsolated()
	{
		return m_eResourceRights == EResourceRights.SELF ||  m_eResourceRights == EResourceRights.NONE;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] gridUpdateId
	//! \return
	bool IsGridUpdateIdGreaterThan(int gridUpdateId)
	{
		return m_ResourceComponent.IsGridUpdateIdGreaterThan(gridUpdateId);
	}
		
	//------------------------------------------------------------------------------------------------
	//! \param[in] rights
	void SetResourceRights(EResourceRights rights)
	{
		m_eResourceRights = rights;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] gridUpdateId
	void SetGridUpdateId(int gridUpdateId)
	{
		m_ResourceComponent.SetGridUpdateId(gridUpdateId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Method used to perform a periodic/iterative update on the state of the resource actor.
	//! \param[in] timeslice Delta time of the moment when the method was called.
	[Obsolete("Use SCR_ResourceActor::Update(WorldTimestamp timestamp) instead.")]
	void Update(float timeslice);
	
	//------------------------------------------------------------------------------------------------
	//! Method used to perform a periodic/iterative update on the state of the resource actor.
	//! \param[in] timestamp WorldTimestamp object of the moment when the method was called.
	void Update(WorldTimestamp timestamp);
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] timeslice
	void UpdateInner(float timeslice)
	{
		
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void Clear()
	{
		
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_ResourceActor()
	{
		Clear();
	}
}

[BaseContainerProps()]
class SCR_ResourceInteractor : SCR_ResourceActor
{	
	protected ref ScriptInvoker<SCR_ResourceInteractor, float> m_OnResourcesChangedInvoker;
	protected ref ScriptInvoker<SCR_ResourceInteractor, float> m_OnMaxResourcesChangedInvoker;
	protected int m_iGridUpdateId = int.MIN;
	//! HOTFIX: Until replication issues are resolved.
	protected float	m_fAggregatedResourceValue = -1.0;
	protected float	m_fAggregatedMaxResourceValue = -1.0;
	protected vector m_LastPosition = vector.Zero;
	
	//------------------------------------------------------------------------------------------------
	override int GetGridUpdateId()
	{
		return m_iGridUpdateId;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetResourceGridRange()
	{
		return 0.0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetAggregatedResourceValue()
	{
		return 0.0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetAggregatedMaxResourceValue()
	{
		return 0.0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetContainerCount()
	{
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return The last processed world position of the interactor.
	vector GetLastPosition()
	{
		return m_LastPosition;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	EResourceType GetResourceType()
	{
		return m_eResourceType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ResourceContainerQueueBase GetContainerQueue()
	{
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker<SCR_ResourceInteractor, float> GetOnResourcesChanged()
	{
		if (!m_OnResourcesChangedInvoker)	
			m_OnResourcesChangedInvoker = new ScriptInvoker<SCR_ResourceInteractor, float>();
		
		return m_OnResourcesChangedInvoker;
		
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker<SCR_ResourceInteractor, float> GetOnMaxResourcesChanged()
	{
		if (!m_OnMaxResourcesChangedInvoker)	
			m_OnMaxResourcesChangedInvoker = new ScriptInvoker<SCR_ResourceInteractor, float>();
		
		return m_OnMaxResourcesChangedInvoker;
		
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	EResourceGeneratorID GetIdentifier()
	{
		return EResourceGeneratorID.INVALID;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsGridUpdateIdGreaterThan(int gridUpdateId)
	{
		return m_iGridUpdateId > gridUpdateId;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] container
	//! \return
	bool IsAllowed(notnull SCR_ResourceContainer container)
	{
		if (container.GetResourceType() != m_eResourceType)
			return false;
		
		switch (m_eResourceRights)
		{
			case EResourceRights.NONE:
				return false;

			case EResourceRights.SELF:
				return m_Owner == container.GetOwner();

			case EResourceRights.SQUAD:
				if (container.GetResourceRight() == EResourceRights.ALL)
					return true;
				// TODO: Logic for detecting the squad.
				return false;

			case EResourceRights.FACTION:
				if (container.GetResourceRight() == EResourceRights.ALL)
					return true;	
			
				FactionAffiliationComponent interactorFactionComponent = m_ResourceComponent.GetFactionAffiliationComponent();
				
				if (!interactorFactionComponent)
					return true;
				
				FactionAffiliationComponent containerFactionComponent = container.GetComponent().GetFactionAffiliationComponent();
				
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
	//! \param[in] entity
	//! \param[in] resourceType
	//! \return
	bool IsAllowed(notnull IEntity entity, EResourceType resourceType)
	{
		if (resourceType != m_eResourceType)
			return false;
		
		switch (m_eResourceRights)
		{
			case EResourceRights.NONE:
				return false;

			case EResourceRights.SELF:
				return m_Owner == entity;

			case EResourceRights.SQUAD:
				// TODO: Logic for detecting the squad.
			case EResourceRights.FACTION:	
				FactionAffiliationComponent interactorFactionComponent = m_ResourceComponent.GetFactionAffiliationComponent();
				
				if (!interactorFactionComponent)
					return false;
				
				SCR_ResourceComponent containerResourceComponent = SCR_ResourceComponent.Cast(entity.FindComponent(SCR_ResourceComponent));
				
				if (!containerResourceComponent)
					return false;
				
				SCR_ResourceContainer container;
				
				if (!containerResourceComponent.GetContainer(resourceType, container))
					return false;
				
				if (container.GetResourceRight() == EResourceRights.ALL)
					return true;
				
				FactionAffiliationComponent containerrFactionComponent = container.GetComponent().GetFactionAffiliationComponent();
				
				if (!containerrFactionComponent)
					return false;
				
				return interactorFactionComponent.GetAffiliatedFaction() == containerrFactionComponent.GetAffiliatedFaction();

			case EResourceRights.ALL:
				return true;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] container
	//! \return
	bool CanInteractWith(notnull SCR_ResourceContainer container)
	{
		return IsAllowed(container) && container.IsAllowed(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] container
	//! \return
	int FindContainer(notnull SCR_ResourceContainer container)
	{
		return SCR_ResourceContainerQueueBase.INVALID_CONTAINER_INDEX;
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetGridUpdateId(int gridUpdateId)
	{
		if (m_iGridUpdateId > gridUpdateId)
			return;
		
		m_iGridUpdateId = gridUpdateId;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates the last known position of the interactor.
	void UpdateLastPosition()
	{
		m_LastPosition = GetOwnerOrigin();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] container
	//! \return
	bool RegisterContainer(notnull SCR_ResourceContainer container)
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] container
	//! \return
	bool RegisterContainerForced(notnull SCR_ResourceContainer container)
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] containerIndex
	//! \return
	bool UnregisterContainer(int containerIndex)
	{	
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] container
	//! \return
	bool UnregisterContainer(notnull SCR_ResourceContainer container)
	{	
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void Replicate()
	{
		m_ResourceComponent.Replicate();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void ReplicateEx()
	{
		m_ResourceComponent.ReplicateEx();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] container
	//! \param[in] previousValue
	void UpdateContainerResourceValue(SCR_ResourceContainer container, float previousValue);
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] container
	//! \param[in] previousValue
	void UpdateContainerMaxResourceValue(SCR_ResourceContainer container, float previousValue);
	
	//------------------------------------------------------------------------------------------------
	//! Gets called when the interactor container queue is updated by the resource grid.
	//! \param[in] grid The resource grid that cause the update.
	void OnResourceGridUpdated(notnull SCR_ResourceGrid grid)
	{
		SetGridUpdateId(grid.GetGridUpdateId());
		UpdateLastPosition();
		Replicate();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] previousValue
	void OnResourcesChanged(float previousValue)
	{
		m_fAggregatedResourceValue = GetAggregatedResourceValue();
		
		if (m_OnResourcesChangedInvoker)
			m_OnResourcesChangedInvoker.Invoke(this, previousValue);
		
		if (!m_ResourceComponent)
			return;
		
		m_ResourceComponent.Replicate();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] previousValue
	void OnMaxResourcesChanged(float previousValue)
	{
		m_fAggregatedMaxResourceValue = GetAggregatedMaxResourceValue();
		
		if (m_OnMaxResourcesChangedInvoker)
			m_OnMaxResourcesChangedInvoker.Invoke(this, previousValue);
		
		if (!m_ResourceComponent)
			return;
		
		m_ResourceComponent.Replicate();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] container
	void OnContainerRegistered(notnull SCR_ResourceContainer container)
	{
		container.LinkInteractor(this);
		OnResourcesChanged(GetAggregatedResourceValue() - container.GetResourceValue());
		OnMaxResourcesChanged(GetAggregatedMaxResourceValue() - container.GetMaxResourceValue());
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] container
	void OnContainerUnregistered(notnull SCR_ResourceContainer container)
	{
		container.UnlinkInteractor(this);
		OnResourcesChanged(GetAggregatedResourceValue() + container.GetResourceValue());
		OnMaxResourcesChanged(GetAggregatedMaxResourceValue() + container.GetMaxResourceValue());
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	void Initialize(notnull IEntity owner)
	{
		m_Owner = owner;
		m_ResourceComponent = SCR_ResourceComponent.Cast(owner.FindComponent(SCR_ResourceComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override void Clear()
	{
		super.Clear();
		
		m_fAggregatedResourceValue = -1.0;
		m_fAggregatedMaxResourceValue = -1.0;
	}
}

[BaseContainerProps(configRoot: true)]
class SCR_ResourceConsumer : SCR_ResourceInteractor
{	
	protected static const int CODEC_CONSUMER_PACKET_BYTESIZE = 38;
	
	[Attribute(defvalue: EResourceGeneratorID.DEFAULT_STORAGE.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Identifier for the generator used for storage", enums: ParamEnumArray.FromEnum(EResourceGeneratorID))]
	protected EResourceGeneratorID m_eGeneratorIdentifier;
	
	[Attribute(defvalue: "0.0", uiwidget: UIWidgets.SpinBox, desc: "Sets the range in which Resource is sought.",params: "0.0 10000.0 1.0")]
	protected float m_fResourceRange;
	
	[Attribute("1", uiwidget: UIWidgets.SpinBox, params: string.Format("0.0 %1 1.0", float.MAX))]
	protected float m_fBuyMultiplier;
	
	[Attribute(uiwidget: UIWidgets.SpinBox, params: string.Format("0.0 %1 1.0", float.MAX))]
	protected float m_fSellMultiplier;
	
	[Attribute(uiwidget: UIWidgets.CheckBox)]
	protected bool m_bIsIgnoringItself;
	
	[Attribute(uiwidget: UIWidgets.Object)]
	protected ref SCR_ResourceConsumerContainerQueue m_ContainerQueue;
	
	protected bool m_bIsConsuming;
	protected bool m_bIsExchanging;
	protected ref ScriptInvoker m_OnResourceRangeChangedInvoker;
	protected ref ScriptInvoker m_OnBuyMultiplierChangedInvoker;
	protected ref ScriptInvoker m_OnSellMultiplierChangedInvoker;
	protected ref ScriptInvoker m_OnConsumtionStateChangedInvoker; // TODO: Consumtion -> Consumption
	protected ref ScriptInvoker m_OnExchangeStateChangedInvoker;
	
	//------------------------------------------------------------------------------------------------
	override float GetResourceGridRange()
	{
		return m_fResourceRange;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetResourceRange()
	{
		return m_fResourceRange;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetBuyMultiplier()
	{
		return m_fBuyMultiplier;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetSellMultiplier()
	{
		return m_fSellMultiplier;
	}
	
	//------------------------------------------------------------------------------------------------
	override float GetAggregatedResourceValue()
	{
		if (!m_ContainerQueue)
			return m_fAggregatedResourceValue;
		
		return m_ContainerQueue.GetAggregatedResourceValue();
	}
	
	//------------------------------------------------------------------------------------------------
	override float GetAggregatedMaxResourceValue()
	{
		if (!m_ContainerQueue)
			return m_fAggregatedMaxResourceValue;
		
		return m_ContainerQueue.GetAggregatedMaxResourceValue();
	}
	
	//------------------------------------------------------------------------------------------------
	override int GetContainerCount()
	{
		if (!m_ContainerQueue)
			return 0.0;
		
		return m_ContainerQueue.GetContainerCount();
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_ResourceContainerQueueBase GetContainerQueue()
	{
		return m_ContainerQueue;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnResourceRangeChanged()
	{
		if (!m_OnResourceRangeChangedInvoker)	
			m_OnResourceRangeChangedInvoker = new ScriptInvoker();
		
		return m_OnResourceRangeChangedInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnBuyMultiplierChanged()
	{
		if (!m_OnBuyMultiplierChangedInvoker)	
			m_OnBuyMultiplierChangedInvoker = new ScriptInvoker();
		
		return m_OnBuyMultiplierChangedInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnSellMultiplierChanged()
	{
		if (!m_OnSellMultiplierChangedInvoker)	
			m_OnSellMultiplierChangedInvoker = new ScriptInvoker();
		
		return m_OnSellMultiplierChangedInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	// TODO: Consumtion -> Consumption
	ScriptInvoker GetOnConsumtionStateChanged()
	{
		if (!m_OnConsumtionStateChangedInvoker)	
			m_OnConsumtionStateChangedInvoker = new ScriptInvoker();
		
		return m_OnConsumtionStateChangedInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnExchangeStateChanged()
	{
		if (!m_OnExchangeStateChangedInvoker)	
			m_OnExchangeStateChangedInvoker = new ScriptInvoker();
		
		return m_OnExchangeStateChangedInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	EResourceGeneratorID GetGeneratorIdentifier()
	{
		return m_eGeneratorIdentifier;
	}
	
	//------------------------------------------------------------------------------------------------
	override EResourceGeneratorID GetIdentifier()
	{
		return m_eGeneratorIdentifier;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsIgnoringItself()
	{
		return m_bIsIgnoringItself;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsConsuming()
	{
		return m_bIsConsuming;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsExchanging()
	{
		return m_bIsExchanging;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool ShouldUpdate()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanInteractWith(notnull SCR_ResourceContainer container)
	{
		return (!m_bIsIgnoringItself || container.GetOwner() != m_Owner) && super.CanInteractWith(container);
	}
	
	//------------------------------------------------------------------------------------------------
	override int FindContainer(notnull SCR_ResourceContainer container)
	{
		if (!m_ContainerQueue)
			return super.FindContainer(container);
		
		return m_ContainerQueue.FindContainer(container);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] value
	//! \param[in] notifyChange
	//! \return
	bool SetResourceRange(float value, bool notifyChange = true)
	{
		float previousValue	= m_fResourceRange;
		m_fResourceRange	= Math.Max(value, 0.0);
		
		if (previousValue == m_fResourceRange)
			return false;
		
		if (notifyChange)
			OnResourceRangeChanged(previousValue);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] value
	//! \param[in] notifyChange
	//! \return
	bool SetBuyMultiplier(float value, bool notifyChange = true)
	{
		float previousValue	= m_fBuyMultiplier;
		m_fBuyMultiplier	= Math.Max(value, 0.0);
		
		if (previousValue == m_fBuyMultiplier)
			return false;
		
		if (notifyChange)
			OnBuyMultiplierChanged(previousValue);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] value
	//! \param[in] notifyChange
	//! \return
	bool SetSellMultiplier(float value, bool notifyChange = true)
	{
		float previousValue	= m_fSellMultiplier;
		m_fSellMultiplier	= Math.Max(value, 0.0);
		
		if (previousValue == m_fSellMultiplier)
			return false;
		
		if (notifyChange)
			OnSellMultiplierChanged(previousValue);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] shouldEnable
	//! \param[in] notifyChange
	//! \return
	// TODO: Consumtion -> Consumption
	bool EnableConsumtion(bool shouldEnable, bool notifyChange = true)
	{
		bool previousValue	= m_bIsConsuming;
		m_bIsConsuming		= shouldEnable;
		
		if (shouldEnable == m_bIsConsuming)
			return false;
		
		if (notifyChange)
			OnConsumtionStateChanged(previousValue);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] shouldEnable
	//! \param[in] notifyChange
	//! \return
	bool EnableExchange(bool shouldEnable, bool notifyChange = true)
	{
		bool previousValue	= m_bIsExchanging;
		m_bIsExchanging		= shouldEnable;
		
		if (previousValue == m_bIsExchanging)
			return false;
		
		if (notifyChange)
			OnExchangeStateChanged(previousValue);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RegisterContainer(notnull SCR_ResourceContainer container)
	{		
		if (CanInteractWith(container) 
		&&	m_ContainerQueue 
		&&	m_ContainerQueue.RegisterContainer(container) != SCR_ResourceContainerQueueBase.INVALID_CONTAINER_INDEX)
		{
			OnContainerRegistered(container);
			
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RegisterContainerForced(notnull SCR_ResourceContainer container)
	{		
		if (m_ContainerQueue &&	m_ContainerQueue.RegisterContainer(container) != SCR_ResourceContainerQueueBase.INVALID_CONTAINER_INDEX)
		{
			OnContainerRegistered(container);
			
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool UnregisterContainer(int containerIndex)
	{
		return m_ContainerQueue && m_ContainerQueue.PopContainerAt(containerIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool UnregisterContainer(notnull SCR_ResourceContainer container)
	{
		return m_ContainerQueue && m_ContainerQueue.PopContainerAt(m_ContainerQueue.FindContainer(container));
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] resourceCost
	//! \param[in] performFullQuery
	//! \return
	SCR_ResourceConsumtionResponse RequestAvailability(float resourceCost, bool performFullQuery = false)
	{
		GetGame().GetResourceGrid().UpdateInteractor(this);
		
		SCR_ResourceConsumtionResponse response = new SCR_ResourceConsumtionResponse(GetAggregatedResourceValue(), m_fBuyMultiplier, m_fResourceRange, EResourceReason.SUFFICIENT);
		
		if (!m_bIsConsuming)
			return response;
		
		if (resourceCost > response.GetAvailableSupply())
		{
			response.SetReason(EResourceReason.INSUFICIENT);
			
			return response;
		}
		
		if (resourceCost != 0.0 && response.GetAvailableSupply() == 0)
		{
			response.SetReason(EResourceReason.UNAVAILABLE);
			
			return response;
		}
		
		return response;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] resourceCost
	//! \return
	// TODO: Consumtion -> Consumption
	SCR_ResourceConsumtionResponse RequestConsumtion(float resourceCost)
	{
		SCR_ResourceConsumtionResponse response = RequestAvailability(resourceCost, true);
		
		if (!m_bIsConsuming || response.GetReason() != EResourceReason.SUFFICIENT)
			return response;
		
		float resourceUsed;
		SCR_ResourceContainer container;
		SCR_ResourceEncapsulator encapsulator;
		
		m_ContainerQueue.PerformSorting();
		
		int containerCount = m_ContainerQueue.GetContainerCount();
		
		for (int i = 0; i < containerCount && resourceCost > 0.0; i++)
		{
			container = m_ContainerQueue.GetContainerAt(i);
			resourceUsed = Math.Min(resourceCost, container.GetResourceValue());
			resourceCost -= resourceUsed;
			encapsulator = container.GetResourceEncapsulator();
			
			if (encapsulator)
				encapsulator.RequestConsumtion(resourceUsed);
			else
				container.DecreaseResourceValue(resourceUsed);
		}
		
		m_ResourceComponent.Replicate();
		
		return response;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void DebugDraw()
	{
		// TODO: Make it so that the nullity of these never happen in the first place.
		if (!m_Owner || !m_ResourceComponent)
			return;
		
		vector origin	= GetOwnerOrigin();
		Color color		= Color.FromInt(m_ResourceComponent.GetDebugColor().PackToInt());
		
		color.Scale(0.2);
		color.SetA(1.0);
		Shape.CreateSphere(m_ResourceComponent.GetDebugColor().PackToInt(), ShapeFlags.TRANSP | ShapeFlags.ONCE | ShapeFlags.DOUBLESIDE | ShapeFlags.NOZWRITE, origin, m_fResourceRange);
		DebugTextWorldSpace.Create(GetGame().GetWorld(), string.Format("  %1  \n  %2 containers  \n  %3 / %4 resources  \n  %5 m  ", m_sDebugName, GetContainerCount(), GetAggregatedResourceValue(), GetAggregatedMaxResourceValue(), m_fResourceRange), DebugTextFlags.ONCE | DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA, origin[0], origin[1] + m_fResourceRange, origin[2], 10.0, 0xFFFFFFFF, color.PackToInt());
		
		if (m_ContainerQueue)
			m_ContainerQueue.DebugDraw();
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateContainerResourceValue(SCR_ResourceContainer container, float previousValue)
	{
		OnResourcesChanged(
			m_ContainerQueue.UpdateContainerResourceValue(container.GetResourceValue(), previousValue)
		);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateContainerMaxResourceValue(SCR_ResourceContainer container, float previousValue)
	{
		OnMaxResourcesChanged(
			m_ContainerQueue.UpdateContainerMaxResourceValue(container.GetMaxResourceValue(), previousValue)
		);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnResourceRangeChanged(float previousValue)
	{
		if (m_OnResourceRangeChangedInvoker)
			m_OnResourceRangeChangedInvoker.Invoke(this, previousValue);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnBuyMultiplierChanged(float previousValue)
	{
		if (m_OnBuyMultiplierChangedInvoker)
			m_OnBuyMultiplierChangedInvoker.Invoke(this, previousValue);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSellMultiplierChanged(float previousValue)
	{
		if (m_OnSellMultiplierChangedInvoker)
			m_OnSellMultiplierChangedInvoker.Invoke(this, previousValue);
	}
	
	//------------------------------------------------------------------------------------------------
	// TODO: Consumtion -> Consumption
	protected void OnConsumtionStateChanged(float previousValue)
	{
		if (m_OnConsumtionStateChangedInvoker)
			m_OnConsumtionStateChangedInvoker.Invoke(this, previousValue);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnExchangeStateChanged(float previousValue)
	{
		if (m_OnExchangeStateChangedInvoker)
			m_OnExchangeStateChangedInvoker.Invoke(this, previousValue);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Initialize(notnull IEntity owner)
	{
		super.Initialize(owner);
		
		SCR_ResourceContainer container = m_ResourceComponent.GetContainer(m_eResourceType);
		
		if (container && container.IsEncapsulated())
			return;
		
		m_bIsConsuming = true;
		
		if (m_ContainerQueue)
			m_ContainerQueue.Initialize(this);
		
		if (container)
			RegisterContainer(container);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Clear()
	{
		super.Clear();
		
		if (m_ContainerQueue)
			m_ContainerQueue.Clear();
		
		GetGame().GetResourceSystemSubscriptionManager().OnResourceInteractorDeleted(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool PropCompareNetworkedVariables(SSnapSerializerBase snapshot, ScriptCtx hint) 
	{               
		RplId componentRplId = Replication.FindId(m_ResourceComponent);
	
		return	snapshot.Compare(componentRplId,				4)
			&&	snapshot.Compare(m_fResourceRange,				4)
			&&	snapshot.Compare(m_fAggregatedResourceValue,	4)
			&&	snapshot.Compare(m_fAggregatedMaxResourceValue,	4)
			&&	snapshot.Compare(m_fBuyMultiplier,				4)
			&&	snapshot.Compare(m_fSellMultiplier,				4)
			&&	snapshot.Compare(m_bIsConsuming,				1)
			&&	snapshot.Compare(m_bIsExchanging,				1)
			&&	snapshot.Compare(m_eResourceRights,				4)
			&&	snapshot.Compare(m_eResourceType,				4)
			&&	snapshot.Compare(m_eGeneratorIdentifier,		4);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool ExtractNetworkedVariables(SSnapSerializerBase snapshot, ScriptCtx hint) 
	{
		float aggregatedResourceValue		= GetAggregatedResourceValue();
		float aggregatedMaxResourceValue	= GetAggregatedMaxResourceValue();
		RplId componentRplId				= Replication.FindId(m_ResourceComponent);
		
		snapshot.SerializeBytes(componentRplId,				4);
		snapshot.SerializeBytes(m_fResourceRange,			4);
		snapshot.SerializeBytes(aggregatedResourceValue,	4);
		snapshot.SerializeBytes(aggregatedMaxResourceValue,	4);
		snapshot.SerializeBytes(m_fBuyMultiplier,			4);
		snapshot.SerializeBytes(m_fSellMultiplier,			4);
		snapshot.SerializeBytes(m_bIsConsuming,				1);
		snapshot.SerializeBytes(m_bIsExchanging,			1);
		snapshot.SerializeBytes(m_eResourceRights,			4);
		snapshot.SerializeBytes(m_eResourceType,			4);
		snapshot.SerializeBytes(m_eGeneratorIdentifier,		4);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool InjectNetworkedVariables(SSnapSerializerBase snapshot, ScriptCtx hint) 
	{
		RplId componentRplId;
		
		snapshot.SerializeBytes(componentRplId,					4);
		snapshot.SerializeBytes(m_fResourceRange,				4);
		snapshot.SerializeBytes(m_fAggregatedResourceValue,		4);
		snapshot.SerializeBytes(m_fAggregatedMaxResourceValue,	4);
		snapshot.SerializeBytes(m_fBuyMultiplier,				4);
		snapshot.SerializeBytes(m_fSellMultiplier,				4);
		snapshot.SerializeBytes(m_bIsConsuming,					1);
		snapshot.SerializeBytes(m_bIsExchanging,				1);
		snapshot.SerializeBytes(m_eResourceRights,				4);
		snapshot.SerializeBytes(m_eResourceType,				4);
		snapshot.SerializeBytes(m_eGeneratorIdentifier,			4);
		
		m_ResourceComponent = SCR_ResourceComponent.Cast(Replication.FindItem(componentRplId));
		
		if (!m_ResourceComponent)
			return false;
		
		m_Owner = m_ResourceComponent.GetOwner();
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] snapshot
	//! \param[in] ctx
	//! \param[in] packet
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet) 
	{
		snapshot.Serialize(packet, SCR_ResourceConsumer.CODEC_CONSUMER_PACKET_BYTESIZE);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] packet
	//! \param[in] ctx
	//! \param[in] snapshot
	//! \return
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		return snapshot.Serialize(packet, SCR_ResourceConsumer.CODEC_CONSUMER_PACKET_BYTESIZE);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] lhs
	//! \param[in] rhs
	//! \param[in] ctx
	//! \return
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs , ScriptCtx ctx)
	{
		return lhs.CompareSnapshots(rhs, SCR_ResourceConsumer.CODEC_CONSUMER_PACKET_BYTESIZE);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] instance
	//! \param[in] snapshot
	//! \param[in] ctx
	//! \return
	static bool PropCompare(SCR_ResourceConsumer instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		return instance.PropCompareNetworkedVariables(snapshot, ctx);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] instance
	//! \param[in] ctx
	//! \param[in] snapshot
	//! \return
	static bool Extract(SCR_ResourceConsumer instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		return instance.ExtractNetworkedVariables(snapshot, ctx);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] snapshot
	//! \param[in] ctx
	//! \param[in] instance
	//! \return
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_ResourceConsumer instance)
	{
		return instance.InjectNetworkedVariables(snapshot, ctx);
	}
}