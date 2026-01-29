enum EResourceGeneratorID
{
	INVALID = -1,
	DEFAULT = 0,
	DEFAULT_STORAGE,
	VEHICLE_UNLOAD,
	VEHICLE_LOAD,
	SELF,
	DUMMY_1,
	DUMMY_2
};

class SCR_ResourceGenerationResponse
{
	protected float				m_fAvailableResourceSpace;
	protected float				m_fResourceMultiplier;
	protected float				m_fRange;
	protected EResourceReason	m_eReason;
	
	//------------------------------------------------------------------------------------------------
	void SCR_ResourceGenerationResponse(float availableResourceSpace = 0, float resourceMultiplier = 0, float range = 0, EResourceReason reasonCode = EResourceReason.UNAVAILABLE)
	{
		m_fAvailableResourceSpace	= availableResourceSpace;
		m_fResourceMultiplier		= resourceMultiplier;
		m_fRange					= range;
		m_eReason					= reasonCode;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetAvailableResourceSpace()
	{
		return m_fAvailableResourceSpace;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetResourceMultiplier()
	{
		return m_fResourceMultiplier;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetRange()
	{
		return m_fRange;
	}
	
	//------------------------------------------------------------------------------------------------
	EResourceReason GetReason()
	{
		return m_eReason;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAvailableResourceSpace(float availableResourceSpace)
	{
		m_fAvailableResourceSpace = availableResourceSpace;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetResourceMultiplier(float resourceMultiplier)
	{
		m_fResourceMultiplier = resourceMultiplier;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRange(float range)
	{
		m_fRange = range;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetReason(EResourceReason reasonCode)
	{
		m_eReason = reasonCode;
	}
};

[BaseContainerProps(configRoot: true)]
class SCR_ResourceGenerator : SCR_ResourceInteractor
{
	[Attribute(defvalue: EResourceGeneratorID.DEFAULT.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Identifier for the generator", enums: ParamEnumArray.FromEnum(EResourceGeneratorID))]
	EResourceGeneratorID m_eIdentifier;
	
	[Attribute(defvalue: "1.0", uiwidget: UIWidgets.SpinBox, params: string.Format("0.0 %1 1.0", float.MAX))]
	protected float m_fResourceMultiplier;
	
	[Attribute(uiwidget: UIWidgets.SpinBox, desc: "Sets the range in meters on which the stored resource looks for a Storage point to merge with.",params: "0.0 10000.0 1.0")]
	protected float m_fStorageRange;
	
	[Attribute(uiwidget: UIWidgets.CheckBox)]
	protected bool m_bIsIgnoringItself;
	
	[Attribute(uiwidget: UIWidgets.Object)]
	protected ref SCR_ResourceGeneratorContainerQueue m_ContainerQueue;
	
	[Attribute(uiwidget: UIWidgets.Object)]
	protected ref array<ref SCR_ResourceGeneratorActionBase> m_aActions;
	
	//------------------------------------------------------------------------------------------------
	override float GetResourceGridRange()
	{
		return m_fStorageRange;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetStorageRange()
	{
		return m_fStorageRange;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetResourceMultiplier()
	{
		return m_fResourceMultiplier;
	}
	
	//------------------------------------------------------------------------------------------------
	override EResourceGeneratorID GetIdentifier()
	{
		return m_eIdentifier;
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
	bool IsIgnoringItself()
	{
		return m_bIsIgnoringItself;
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
	SCR_ResourceGenerationResponse RequestAvailability(float resourceAmount)
	{
		GetGame().GetResourceGrid().UpdateInteractor(this);
		
		SCR_ResourceGenerationResponse response = new SCR_ResourceGenerationResponse(GetAggregatedMaxResourceValue() - GetAggregatedResourceValue(), m_fResourceMultiplier, m_fStorageRange, EResourceReason.SUFFICIENT);
		
		if (response.GetAvailableResourceSpace() == 0)
		{
			response.SetReason(EResourceReason.UNAVAILABLE);
			
			return response;
		}
		
		foreach (SCR_ResourceGeneratorActionBase action: m_aActions)
		{
			resourceAmount -= action.ComputeGeneratedResources(this, resourceAmount);
			
			if (resourceAmount <= 0)
				return response;
		}
		
		response.SetReason(EResourceReason.INSUFICIENT);
		
		return response;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ResourceGenerationResponse RequestGeneration(float resourceAmount, SCR_ResourceGenerator generator = null)
	{
		SCR_ResourceGenerationResponse response = RequestAvailability(resourceAmount);
		
		if (response.GetReason() != EResourceReason.SUFFICIENT)
			return response;
		
		m_ContainerQueue.PerformSorting();
		
		int containerCount = m_ContainerQueue.GetContainerCount();
		
		foreach (SCR_ResourceGeneratorActionBase action: m_aActions)
		{
			if (resourceAmount <= 0)
				break;
			
			action.PerformAction(this, resourceAmount);	
		}
		
		m_ResourceComponent.Replicate();
		
		return response;
	}
	
	//------------------------------------------------------------------------------------------------
	void DebugDraw()
	{
		// TODO: Make it so that the nullity of these never happen in the first place.
		if (!m_Owner || !m_ResourceComponent)
			return;
		
		vector origin	= GetOwnerOrigin();
		Color color		= m_ResourceComponent.GetDebugColor();
		Color color2	= m_ResourceComponent.GetDebugColor();
		
		color.Scale(0.2);
		color.SetA(1.0);
		color2.Lerp(Color.FromInt(Color.WHITE), 0.5);
		color2.SetA(1.0);
		Shape.CreateSphere(m_ResourceComponent.GetDebugColor().PackToInt(), ShapeFlags.ONCE | ShapeFlags.NOZWRITE | ShapeFlags.WIREFRAME, origin, m_fStorageRange);
		DebugTextWorldSpace.Create(GetGame().GetWorld(), string.Format("  %1  \n  %2 containers  \n  %3 / %4 resources  \n  %5 m  ", m_sDebugName, GetContainerCount(), GetAggregatedResourceValue(), GetAggregatedMaxResourceValue(), m_fStorageRange), DebugTextFlags.ONCE | DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA, origin[0], origin[1] + m_fStorageRange, origin[2], 10.0, color.PackToInt(), color2.PackToInt());
		
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
	override void Initialize(notnull IEntity owner)
	{
		super.Initialize(owner);
		
		SCR_ResourceContainer container = m_ResourceComponent.GetContainer(m_eResourceType);
		
		if (container && container.IsEncapsulated())
			return;
		
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
			&&	snapshot.Compare(m_fStorageRange,				4)
			&&	snapshot.Compare(m_fAggregatedResourceValue,	4)
			&&	snapshot.Compare(m_fAggregatedMaxResourceValue,	4)
			&&	snapshot.Compare(m_fResourceMultiplier,			4)
			&&	snapshot.Compare(m_eResourceRights,				4)
			&&	snapshot.Compare(m_eResourceType,				4)
			&&	snapshot.Compare(m_eIdentifier,					4);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool ExtractNetworkedVariables(SSnapSerializerBase snapshot, ScriptCtx hint) 
	{
		float aggregatedResourceValue		= GetAggregatedResourceValue();
		float aggregatedMaxResourceValue	= GetAggregatedMaxResourceValue();
		RplId componentRplId				= Replication.FindId(m_ResourceComponent);
		
		snapshot.SerializeBytes(componentRplId,				4);
		snapshot.SerializeBytes(m_fStorageRange,			4);
		snapshot.SerializeBytes(aggregatedResourceValue,	4);
		snapshot.SerializeBytes(aggregatedMaxResourceValue,	4);
		snapshot.SerializeBytes(m_fResourceMultiplier,		4);
		snapshot.SerializeBytes(m_eResourceRights,			4);
		snapshot.SerializeBytes(m_eResourceType,			4);
		snapshot.SerializeBytes(m_eIdentifier,				4);
		
		return true;
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected bool InjectNetworkedVariables(SSnapSerializerBase snapshot, ScriptCtx hint) 
	{		
		RplId componentRplId;
		
		snapshot.SerializeBytes(componentRplId,					4);
		snapshot.SerializeBytes(m_fStorageRange,				4);
		snapshot.SerializeBytes(m_fAggregatedResourceValue,		4);
		snapshot.SerializeBytes(m_fAggregatedMaxResourceValue,	4);
		snapshot.SerializeBytes(m_fResourceMultiplier,			4);
		snapshot.SerializeBytes(m_eResourceRights,				4);
		snapshot.SerializeBytes(m_eResourceType,				4);
		snapshot.SerializeBytes(m_eIdentifier,					4);
		
		m_ResourceComponent = SCR_ResourceComponent.Cast(Replication.FindItem(componentRplId));
		
		if (!m_ResourceComponent)
			return false;
		
		m_Owner = m_ResourceComponent.GetOwner();
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet) 
	{
		snapshot.Serialize(packet, SCR_ResourceGenerator.CODEC_GENERATOR_PACKET_BYTESIZE);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		return snapshot.Serialize(packet, SCR_ResourceGenerator.CODEC_GENERATOR_PACKET_BYTESIZE);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs , ScriptCtx ctx)
	{
		return lhs.CompareSnapshots(rhs, SCR_ResourceGenerator.CODEC_GENERATOR_PACKET_BYTESIZE);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool PropCompare(SCR_ResourceGenerator instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		return instance.PropCompareNetworkedVariables(snapshot, ctx);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_ResourceGenerator instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		return instance.ExtractNetworkedVariables(snapshot, ctx);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_ResourceGenerator instance)
	{
		return instance.InjectNetworkedVariables(snapshot, ctx);
	}
}