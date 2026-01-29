enum EResourceDebugVisualizationFlags
{
	CONSUMER		= 1 << 0,
	CONTAINER		= 1 << 1,
	GENERATOR		= 1 << 2,
	ENCAPSULATOR	= 1 << 3,
}

//~ ScriptInvokers
void SCR_Resources_OnResourceEnabledChanged(SCR_ResourceComponent resourceComponent, array<EResourceType> disabledResourceTypes);
typedef func SCR_Resources_OnResourceEnabledChanged;

//! Component class with the container configuration on it in order to avoid duplication of data in the
//! container prefabs and the entity catalogs for the container-to-container interaction.
[ComponentEditorProps(category: "GameScripted/Resources", description: "")]
class SCR_ResourceComponentClass : ScriptComponentClass
{	
	//! Container prefab base configuration.
	//! Warning: This gets copied/duplicated in SCR_ResourceComponent through
	//! SCR_ResourceContainer::Initialize(IEntity owner, SCR_ResourceContainer container = null).
	[Attribute(uiwidget: UIWidgets.Object, category: "Containers")]
	protected ref array<ref SCR_ResourceContainer> m_aContainers;

	//------------------------------------------------------------------------------------------------
	//! Warning: Only to be used for initialising the container in SCR_ResourceComponent.
	//! \return The prefab base container configuration, or null if none.
	array<ref SCR_ResourceContainer> GetContainers()
	{
		return m_aContainers;
	}
	
	//------------------------------------------------------------------------------------------------
	override static array<typename> Requires(IEntityComponentSource src)
	{
		return { RplComponent };
	}
}

//! Component used to activate the sandbox resources functionality on an entity.
//! This component can enable the functionality for both sandbox container and sandbox consumer in an individual manner.
//! The sandbox resources functionality does not require a manager so just the presence of containers being registered into a consumer's queue is enough for it to perform its functionality.
//! The replication component is a requirement for the component to work properly.
class SCR_ResourceComponent : ScriptComponent
{
	const float UPDATE_DISTANCE_TRESHOLD = 2.5;
	const float UPDATE_DISTANCE_TRESHOLD_SQUARE = UPDATE_DISTANCE_TRESHOLD * UPDATE_DISTANCE_TRESHOLD;
	protected const float UPDATE_PERIOD = 10.0 / 60.0;
	protected bool m_bIsNetDirty;
	protected int m_iGridUpdateId = int.MIN;
	
	protected int m_iGridContainersBoundsMins = 0xFFFFFFFF;
	protected int m_iGridContainersBoundsMaxs = 0xFFFFFFFF;
	protected vector m_vGridContainersBoundingVolumeMins;
	protected vector m_vGridContainersBoundingVolumeMaxs;
	
	protected bool m_bIsFlaggedForProcessing;

	//! Refer to SCR_ResourceEncapsulator for documentation.
	[Attribute(uiwidget: UIWidgets.Object, category: "Encapsulators")]
	protected ref array<ref SCR_ResourceEncapsulator> m_aEncapsulators;
	
	//! Refer to SCR_ResourceConsumer for documentation.
	[Attribute(uiwidget: UIWidgets.Object, category: "Consumers"), RplProp(onRplName: "TEMP_OnInteractorReplicated")]
	protected ref array<ref SCR_ResourceConsumer> m_aConsumers;
	
	//! Refer to SCR_ResourceGenerator for documentation.
	[Attribute(uiwidget: UIWidgets.Object, category: "Generators"), RplProp(onRplName: "TEMP_OnInteractorReplicated")]
	protected ref array<ref SCR_ResourceGenerator> m_aGenerators;
	
	//~ Any Resource Types that is set here is a disabled resource type
	[Attribute(desc: "List of all disabled resource types", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EResourceType)), RplProp(onRplName: "OnResourceTypeEnabledChanged")]
	protected ref array<EResourceType> m_aDisabledResourceTypes;
	
	//~ Any ResourceType that is set here cannot be enabled or disabled in runtime for this entity by editor
	[Attribute(desc: "Any ResourceType that is set here cannot be enabled or disabled in runtime for this entity by editor", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EResourceType))]
	protected ref array<EResourceType> m_aDisallowChangingEnableResource;
	
	//! HOTFIX: Until replication issues are resolved.
	protected ref ScriptInvoker m_TEMP_OnInteractorReplicated;
	
	protected ref ScriptInvokerBase<SCR_Resources_OnResourceEnabledChanged> m_OnResourceTypeEnabledChanged;
	
	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------------------------
	//! HOTFIX: Until replication issues are resolved.
	ScriptInvoker TEMP_GetOnInteractorReplicated()
	{
		if (!m_TEMP_OnInteractorReplicated)
			m_TEMP_OnInteractorReplicated = new ScriptInvoker();
		
		return m_TEMP_OnInteractorReplicated;
	}
	
	//------------------------------------------------------------------------------------------------
	//! HOTFIX: Until replication issues are resolved.
	void TEMP_OnInteractorReplicated()
	{
		if (m_TEMP_OnInteractorReplicated)
			m_TEMP_OnInteractorReplicated.Invoke();
	}
	//---- REFACTOR NOTE END ----
	
	//! Setting for enabling the debugging visualization of the container and/or consumer.
	[Attribute(uiwidget: UIWidgets.CheckBox, category: "Debugging")]
	protected bool m_bEnableDebugVisualization;
	
	//! Flags for enabling the debugging visualization.
	[Attribute(defvalue: "-1", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EResourceDebugVisualizationFlags), category: "Debugging")]
	protected EResourceDebugVisualizationFlags m_eDebugVisualizationFlags;

	//! Setting for the base color for the debugging visualization of the container and/or consumer.
	[Attribute(defvalue: "0.4 0.0 0.467 0.267 ", uiwidget: UIWidgets.ColorPicker, category: "Debugging")]
	protected ref Color m_DebugColor;
	
	[RplProp(onRplName: "OnVisibilityChanged")]
	protected bool m_bIsVisible = true;
	
	//! Defined/Configured through SCR_ResourceComponentClass::m_Container.
	//! Refer to SCR_ResourceContainer for documentation.
	protected ref array<ref SCR_ResourceContainer> m_aContainerInstances = {};
	
	//! Replication component attached to the owner entity.
	protected RplComponent m_ReplicationComponent;
	protected FactionAffiliationComponent m_FactionAffiliationComponent;
	
	protected vector m_LastPosition = vector.Zero;
	protected bool m_bHasParent;
	protected bool m_bIsOwnerActive;
	
	// TODO: Remove after fixing initialisation and hierarchy issues.
	protected bool m_bIsInitialized;

	// TODO: Remove after fixing initialisation and hierarchy issues.
	protected bool m_bIsAddedToParentBuffered;
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] resourceType Type to check
	//! \return If supplies are enabled or not (Only checked if global supplies are enabled, otherwise it always returns false)
	bool IsResourceTypeEnabled(EResourceType resourceType = EResourceType.SUPPLIES)
	{
		if (!SCR_ResourceSystemHelper.IsGlobalResourceTypeEnabled(resourceType))
			return false;
		
		return !m_aDisabledResourceTypes.Contains(resourceType);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] disabledResourceTypes List of disabled resources
	//! \return Count of all disabled resource types
	int GetDisabledResourceTypes(inout notnull array<EResourceType> disabledResourceTypes)
	{
		disabledResourceTypes.Copy(m_aDisabledResourceTypes);
		return disabledResourceTypes.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] resourceType Type to check
	//! \return If supplies are enabled or not (Only checked if global supplies are enabled, otherwise it always returns false)
	bool CanResourceTypeEnabledBeChanged(EResourceType resourceType = EResourceType.SUPPLIES)
	{		
		return !m_aDisallowChangingEnableResource.Contains(resourceType);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set Resource Type enabled or disabled
	//! \param[in] enable Set true to enable resource type, set false to disable
	//! \param[in] resourceType Resource type to enable/disable
	void SetResourceTypeEnabled(bool enable, EResourceType resourceType = EResourceType.SUPPLIES)
	{
		if (!CanResourceTypeEnabledBeChanged(resourceType))
			return;
		
		int index = m_aDisabledResourceTypes.Find(resourceType);
		
		//~ Already Enabled/Disabled
		if ((index < 0) == enable)
			return;
		
		if (!enable)
			m_aDisabledResourceTypes.Insert(resourceType);
		else 
			m_aDisabledResourceTypes.Remove(index);
		
		Replication.BumpMe();
		
		//~ Enable/Disable resource type for all encapsulated resourceComponents
		/*array<SCR_ResourceEncapsulator> encapsulators = GetEncapsulators();
		SCR_ResourceComponent resourceComponent;
		foreach (SCR_ResourceEncapsulator encapsulator : encapsulators)
		{
			resourceComponent = encapsulator.GetComponent();
			if (!resourceComponent || !resourceComponent.CanResourceTypeEnabledBeChanged(resourceType))
				continue;
			
			resourceComponent.SetResourceTypeEnabled(enable, resourceType);
		}*/

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if ((gameMode && gameMode.IsMaster()) ||  (!gameMode && Replication.IsServer()))
			OnResourceTypeEnabledChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnResourceTypeEnabledChanged()
	{
		if (m_OnResourceTypeEnabledChanged)
			m_OnResourceTypeEnabledChanged.Invoke(this, m_aDisabledResourceTypes);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return ScriptInvoker OnSupplies Enabled
	ScriptInvokerBase<SCR_Resources_OnResourceEnabledChanged> GetOnResourceTypeEnabledChanged()
	{
		if (!m_OnResourceTypeEnabledChanged)
			m_OnResourceTypeEnabledChanged = new ScriptInvokerBase<SCR_Resources_OnResourceEnabledChanged>();
		
		return m_OnResourceTypeEnabledChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Use this function to obtain the Resource component instead of IEntity.FindComponent(SCR_ResourceComponent)
	//! \param[in] entity Entity to get Resource component from. Will loop through children in entity is a vehicle and no resource component was found on the entity
	//! \param[in] ignoreChildIfHasStorage If looping through children and this is true than it will not return the ResourceComponent if that child has an inventory storage
	//! \return Get ResourceComponent from given entity.
	static SCR_ResourceComponent FindResourceComponent(IEntity entity, bool ignoreChildIfHasStorage = false)
	{
		//~ Function is used in many places. Not all can guarantee that entity is not null
		if (!entity)
			return null;
		
		//~ Get resource component
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.Cast(entity.FindComponent(SCR_ResourceComponent));
		if (resourceComponent)
			return resourceComponent;
		
		//~ Loop over slotted entities if entity is a vehicle. Note that slotted entities are not yet in hierarchy the moment they are created for clients
		if (Vehicle.Cast(entity))
		{
			SlotManagerComponent slotManager = SlotManagerComponent.Cast(entity.FindComponent(SlotManagerComponent));
			if (!slotManager)
				return null;
			
			array<EntitySlotInfo> outSlotInfos = {};
			slotManager.GetSlotInfos(outSlotInfos);
			IEntity slot;
			
			foreach (EntitySlotInfo slotInfo : outSlotInfos)
			{
				slot = slotInfo.GetAttachedEntity();
				
				if (!slot)
					continue;
				
				resourceComponent = SCR_ResourceComponent.Cast(slot.FindComponent(SCR_ResourceComponent));
				if (resourceComponent)
				{					
					//~ It does not care if the slotted entity has a storage or not
					if (!ignoreChildIfHasStorage)
						return resourceComponent;
					
					//~ Has no storage so this is a valid resource component
					if (!slot.FindComponent(ScriptedBaseInventoryStorageComponent))
						return resourceComponent;
				}
			}
		}
		//~ Check if parent is vehicle than get resource component from parent or any other slot
		else if (Vehicle.Cast(entity.GetParent()))
		{
			InventoryItemComponent inventoryItem = InventoryItemComponent.Cast(entity.FindComponent(InventoryItemComponent));
			if (inventoryItem)
			{
				//~ Is item in inventory thus ignore getting vehicle parent
				if (inventoryItem.GetParentSlot())
					return null;
			}
			
			return SCR_ResourceComponent.FindResourceComponent(entity.GetParent(), ignoreChildIfHasStorage);
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetGridContainersBoundsMins()
	{
		return m_iGridContainersBoundsMins;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetGridContainersBoundsMaxs()
	{
		return m_iGridContainersBoundsMaxs;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] mins
	//! \param[out] maxs
	void GetGridContainersBounds(out int mins, out int maxs)
	{
		mins = m_iGridContainersBoundsMins;
		maxs = m_iGridContainersBoundsMaxs;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] mins
	//! \param[out] maxs
	void GetGridContainersBoundingBox(out vector mins, out vector maxs)
	{
		mins = m_vGridContainersBoundingVolumeMins;
		maxs = m_vGridContainersBoundingVolumeMaxs;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] mins
	//! \param[out] maxs
	void GetGridContainersWorldBoundingBox(out vector mins, out vector maxs)
	{
		vector ownerOrigin = GetOwner().GetOrigin();
		mins = ownerOrigin + m_vGridContainersBoundingVolumeMins;
		maxs = ownerOrigin + m_vGridContainersBoundingVolumeMaxs;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	RplComponent GetReplicationComponent()
	{
		return m_ReplicationComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	FactionAffiliationComponent GetFactionAffiliationComponent()
	{
		return m_FactionAffiliationComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] resourceType
	//! \return The first encapsulator instance of a specified resource type, or null if none.
	SCR_ResourceEncapsulator GetEncapsulator(EResourceType resourceType)
	{
		if (resourceType == EResourceType.INVALID || !m_aEncapsulators)
			return null;
		
		int higherLimitPosition = m_aEncapsulators.Count();
		
		if (higherLimitPosition == 0)
			return null;
		
		int position;
		SCR_ResourceEncapsulator encapsulator;
		
		while (position < higherLimitPosition)
		{
			if (GetNextEncapsulatorCandidate(position, higherLimitPosition, encapsulator, resourceType))
				break;
		}
		
		if (!encapsulator
		||	position == m_aEncapsulators.Count()
		||	resourceType != encapsulator.GetResourceType())
			return null;
		
		return encapsulator;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] resourceType
	//! \param[out] encapsulator The first encapsulator instance of a specified resource type, or null if none.
	//! \return If a encapsulator instance of a specified resource type has been found.
	bool GetEncapsulator(EResourceType resourceType, out SCR_ResourceEncapsulator encapsulator)
	{
		encapsulator = null;
		
		if (resourceType == EResourceType.INVALID || !m_aEncapsulators)
			return false;
		
		int higherLimitPosition = m_aEncapsulators.Count();
		
		if (higherLimitPosition == 0)
			return false;
		
		int position;
		
		while (position < higherLimitPosition)
		{
			if (GetNextEncapsulatorCandidate(position, higherLimitPosition, encapsulator, resourceType))
				break;
		}
		
		if (!encapsulator 
		||	position == m_aEncapsulators.Count() 
		||	resourceType != encapsulator.GetResourceType())
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetNextEncapsulatorCandidate(inout int position, inout int higherLimitPosition, inout SCR_ResourceEncapsulator encapsulator, EResourceType resourceType)
	{
		int comparePosition	= position + ((higherLimitPosition - position) >> 1);
		encapsulator		= m_aEncapsulators[comparePosition];
		
		if (!encapsulator)
			return null;
		
		EResourceType compareResourceType = encapsulator.GetResourceType();
		
		if (resourceType > compareResourceType)
			position = comparePosition + 1;
		else if (resourceType < compareResourceType)
			higherLimitPosition = comparePosition;
		else 
		{
			encapsulator = m_aEncapsulators[comparePosition];
			
			return true;
		}
		
		encapsulator = null;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return The encapsulator instances, or null if none.
	array<SCR_ResourceEncapsulator> GetEncapsulators()
	{
		if (!m_aEncapsulators)
			return null;
		
		int encapsulatorCount = m_aEncapsulators.Count();
		
		if (encapsulatorCount == 0)
			return null;
		
		array<SCR_ResourceEncapsulator> encapsulators = {};
		
		encapsulators.Reserve(encapsulatorCount);
		
		foreach (SCR_ResourceEncapsulator encapsulator: m_aEncapsulators)
		{
			encapsulators.Insert(encapsulator);
		}
		
		return encapsulators;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] resourceType
	//! \return The first container instance of a specified resource type, or null if none.
	SCR_ResourceContainer GetContainer(EResourceType resourceType)
	{
		if (resourceType == EResourceType.INVALID || !m_aContainerInstances)
			return null;
		
		int higherLimitPosition = m_aContainerInstances.Count();
		
		if (higherLimitPosition == 0)
			return null;
		
		int position;
		SCR_ResourceContainer container;
		
		while (position < higherLimitPosition)
		{
			if (GetNextContainerCandidate(position, higherLimitPosition, container, resourceType))
				break;
		}
		
		if (!container
		||	position == m_aContainerInstances.Count()
		||	resourceType != container.GetResourceType())
			return null;
		
		return container;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] resourceType
	//! \param[out] container
	//! \return If a container instance of a specified resource type has been found.
	bool GetContainer(EResourceType resourceType, out SCR_ResourceContainer container)
	{
		container = null;
		
		if (resourceType == EResourceType.INVALID || !m_aContainerInstances)
			return false;
		
		int higherLimitPosition = m_aContainerInstances.Count();
		if (higherLimitPosition == 0)
			return false;
		
		int position;
		
		while (position < higherLimitPosition)
		{
			if (GetNextContainerCandidate(position, higherLimitPosition, container, resourceType))
				break;
		}
		
		if (!container 
		||	position == m_aContainerInstances.Count() 
		||	resourceType != container.GetResourceType())
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetNextContainerCandidate(inout int position, inout int higherLimitPosition, inout SCR_ResourceContainer container, EResourceType resourceType)
	{
		int comparePosition	= position + ((higherLimitPosition - position) >> 1);
		container			= m_aContainerInstances[comparePosition];
		
		if (!container)
			return null;
		
		EResourceType compareResourceType = container.GetResourceType();
		
		if (resourceType > compareResourceType)
		{
			position = comparePosition + 1;
		}
		else if (resourceType < compareResourceType)
		{
			higherLimitPosition = comparePosition;
		}
		else 
		{
			container = m_aContainerInstances[comparePosition];
			
			return true;
		}
		
		container = null;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return The container instances, or null if none.
	array<SCR_ResourceContainer> GetContainers()
	{
		int containerCount = m_aContainerInstances.Count();
		
		if (containerCount == 0)
			return null;
		
		array<SCR_ResourceContainer> containers = {};
		
		containers.Reserve(containerCount);
		
		foreach (SCR_ResourceContainer container: m_aContainerInstances)
		{
			containers.Insert(container);
		}
		
		return containers;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] identifier
	//! \param[in] resourceType
	//! \return The first consumer instance of a specified resource type, or null if none.
	SCR_ResourceConsumer GetConsumer(EResourceGeneratorID identifier, EResourceType resourceType)
	{
		if (identifier == EResourceGeneratorID.INVALID || resourceType == EResourceType.INVALID || !m_aConsumers)
			return null;
		
		int higherLimitPosition = m_aConsumers.Count();
		if (higherLimitPosition == 0)
			return null;
		
		int position;
		SCR_ResourceConsumer consumer;
		
		while (position < higherLimitPosition)
		{
			if (GetNextConsumerCandidate(position, higherLimitPosition, consumer, identifier, resourceType))
				break;
		}
		
		if (!consumer 
		||	position == m_aConsumers.Count()
		||	identifier != consumer.GetGeneratorIdentifier() 
		||	resourceType != consumer.GetResourceType())
			return null;
		
		return consumer;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] identifier
	//! \param[in] resourceType
	//! \param[in,out] consumer The first consumer instance of a specified resource type, or null if none.
	//! \return If a consumer instance of a specified resource type has been found.
	bool GetConsumer(EResourceGeneratorID identifier, EResourceType resourceType, inout SCR_ResourceConsumer consumer)
	{
		consumer = null;
		
		if (identifier == EResourceGeneratorID.INVALID || resourceType == EResourceType.INVALID || !m_aConsumers)
			return false;
		
		int higherLimitPosition = m_aConsumers.Count();
		if (higherLimitPosition == 0)
			return false;
		
		int position;
		
		while (position < higherLimitPosition)
		{
			if (GetNextConsumerCandidate(position, higherLimitPosition, consumer, identifier, resourceType))
				break;
		}
		
		if (!consumer 
		||	position == m_aConsumers.Count() 
		||	identifier != consumer.GetGeneratorIdentifier()
		||	resourceType != consumer.GetResourceType())
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetNextConsumerCandidate(inout int position, inout int higherLimitPosition, inout SCR_ResourceConsumer consumer, EResourceGeneratorID identifier, EResourceType resourceType)
	{
		int comparePosition	= position + ((higherLimitPosition - position) >> 1);
		consumer			= m_aConsumers[comparePosition];
		
		if (!consumer)
			return false;
		
		EResourceType compareResourceType		= consumer.GetResourceType();
		EResourceGeneratorID comapareIdentifier	= consumer.GetGeneratorIdentifier();
		
		if (identifier > comapareIdentifier)
		{
			position = comparePosition + 1;
		}
		else if (identifier < comapareIdentifier)
		{
			higherLimitPosition = comparePosition;
		}
		else if (resourceType > compareResourceType)
		{
			position = comparePosition + 1;
		}
		else if (resourceType < compareResourceType)
		{
			higherLimitPosition = comparePosition;
		}
		else 
		{
			consumer = m_aConsumers[comparePosition];
			
			return true;
		}
		
		consumer = null;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return The consumer instances, or null if none.
	array<SCR_ResourceConsumer> GetConsumers()
	{
		int consumerCount = m_aConsumers.Count();
		if (consumerCount == 0)
			return null;
		
		array<SCR_ResourceConsumer> consumers = {};
		consumers.Reserve(consumerCount);
		
		foreach (SCR_ResourceConsumer container: m_aConsumers)
		{
			consumers.Insert(container);
		}
		
		return consumers;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] identifier
	//! \param[in] resourceType
	//! \return The first generator instance of a specified resource type, or null if none.
	SCR_ResourceGenerator GetGenerator(EResourceGeneratorID identifier, EResourceType resourceType)
	{
		if (identifier == EResourceGeneratorID.INVALID || resourceType == EResourceType.INVALID || !m_aGenerators)
			return null;
		
		int higherLimitPosition = m_aGenerators.Count();
		if (higherLimitPosition == 0)
			return null;
		
		int position;
		SCR_ResourceGenerator generator;
		
		while (position < higherLimitPosition)
		{
			if (GetNextGeneratorCandidate(position, higherLimitPosition, generator, identifier, resourceType))
				break;
		}
		
		if (!generator 
		||	position == m_aGenerators.Count() 
		||	identifier != generator.GetIdentifier() 
		||	resourceType != generator.GetResourceType())
			return null;
		
		return generator;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] identifier
	//! \param[in] resourceType
	//! \param[out] generator The first generator instance of a specified resource type, or null if none.
	//! \return If a generator instance of a specified resource type has been found.
	bool GetGenerator(EResourceGeneratorID identifier, EResourceType resourceType, out SCR_ResourceGenerator generator)
	{
		generator = null;
		
		if (identifier == EResourceGeneratorID.INVALID || resourceType == EResourceType.INVALID || !m_aGenerators)
			return false;
		
		int higherLimitPosition = m_aGenerators.Count();
		if (higherLimitPosition == 0)
			return false;
		
		int position;
		
		while (position < higherLimitPosition)
		{
			if (GetNextGeneratorCandidate(position, higherLimitPosition, generator, identifier, resourceType))
				break;
		}
		
		if (!generator 
		||	position == m_aGenerators.Count() 
		||	identifier != generator.GetIdentifier() 
		||	resourceType != generator.GetResourceType())
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetNextGeneratorCandidate(inout int position, inout int higherLimitPosition, inout SCR_ResourceGenerator generator, EResourceGeneratorID identifier, EResourceType resourceType)
	{
		int comparePosition	= position + ((higherLimitPosition - position) >> 1);
		generator			= m_aGenerators[comparePosition];
		
		if (!generator)
			return false;
		
		EResourceType compareResourceType		= generator.GetResourceType();
		EResourceGeneratorID comapareIdentifier	= generator.GetIdentifier();
		
		if (identifier > comapareIdentifier)
		{
			position = comparePosition + 1;
		}
		else if (identifier < comapareIdentifier)
		{
			higherLimitPosition = comparePosition;
		}
		else if (resourceType > compareResourceType)
		{
			position = comparePosition + 1;
		}
		else if (resourceType < compareResourceType)
		{
			higherLimitPosition = comparePosition;
		}
		else 
		{
			generator = m_aGenerators[comparePosition];
			
			return true;
		}
		
		generator = null;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return The generator instances, or null if none.
	//!
	array<SCR_ResourceGenerator> GetGenerators()
	{
		int generatorCount = m_aGenerators.Count();
		
		if (generatorCount == 0)
			return null;
		
		array<SCR_ResourceGenerator> generators = {};
		
		generators.Reserve(generatorCount);
		
		foreach (SCR_ResourceGenerator generator: m_aGenerators)
		{
			generators.Insert(generator);
		}
		
		return generators;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetGridUpdateId()
	{
		return m_iGridUpdateId;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return The last processed world position of the component.
	vector GetLastPosition()
	{
		return m_LastPosition;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return true if the owner has a parent entity, false otherwise.
	bool HasParent()
	{
		return m_bHasParent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return The debug base color for the debugging visualization of the container and/or consumer.
	Color GetDebugColor()
	{
		return Color.FromInt(m_DebugColor.PackToInt());
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] gridUpdateId
	//! \return
	bool IsGridUpdateIdGreaterThan(int gridUpdateId)
	{
		return m_iGridUpdateId > gridUpdateId;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return true if the debugging visualization of the container and/or consumer is enabled, false oherwise.
	bool IsDebugVisualizationEnabled()
	{
		return m_bEnableDebugVisualization;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsVisible()
	{
		return m_bIsVisible;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsOwnerActive()
	{
		return m_bIsOwnerActive;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool ShouldConsumersBeReplicated()
	{
		return m_aConsumers != null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool ShouldGeneratorsBeReplicated()
	{
		return m_aGenerators != null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] gridUpdateId
	void SetGridUpdateId(int gridUpdateId)
	{
		if (m_iGridUpdateId > gridUpdateId)
			return;
		
		m_iGridUpdateId = gridUpdateId;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] state
	void SetIsVisible(bool state)
	{
		m_bIsVisible = state;
		m_bIsNetDirty = true;
		
		ReplicateEx();
		OnVisibilityChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] mins
	void SetGridContainersBoundsMins(int mins)
	{
		m_iGridContainersBoundsMins = mins;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] maxs
	void SetGridContainersBoundsMaxs(int maxs)
	{
		m_iGridContainersBoundsMaxs = maxs;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] mins
	//! \param[in] maxs
	void SetGridContainersBounds(int mins, int maxs)
	{
		m_iGridContainersBoundsMins = mins;
		m_iGridContainersBoundsMaxs = maxs;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates the serial number for the current processing call of the resource grid onto this component.
	void UpdateLastPosition()
	{
		m_LastPosition = GetOwner().GetOrigin();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void FlagForProcessing()
	{
		if (m_bIsFlaggedForProcessing)
			return;
		
		GetGame().GetResourceGrid().FlagResourceItem(this);
		
		m_bIsFlaggedForProcessing = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void UnflagForProcessing()
	{
		if (!m_bIsFlaggedForProcessing)
			return;
		
		GetGame().GetResourceGrid().UnflagResourceItem(this);
		
		m_bIsFlaggedForProcessing = false;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void DeleteConsumers()
	{
		if (!m_aConsumers)
			return;
		
		delete m_aConsumers;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void DeleteGenerators()
	{
		if (!m_aGenerators)
			return;
		
		delete m_aGenerators;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void DeleteQueryInteractors()
	{
		delete m_aConsumers;
		delete m_aGenerators;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initialises the component, the consumer and/or the container.
	//! Event called after init when all components are initialised.
	//! \param[in] owner Entity that owns this component.
	override event protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		Initialize(owner);
		SetEventMask(owner, EntityEvent.INIT);
		
		//~ Remove any duplicate entries
		if (m_aDisabledResourceTypes.IsEmpty())
		{
			//~ TODO: Make this cleaner
			
			set<EResourceType> duplicateRemoveSet = new set<EResourceType>();
			
			foreach (EResourceType resourceType : m_aDisabledResourceTypes)
			{
				duplicateRemoveSet.Insert(resourceType);
			}
			
			m_aDisabledResourceTypes.Clear();
			foreach (EResourceType resourceType : duplicateRemoveSet)
			{
				m_aDisabledResourceTypes.Insert(resourceType);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	void Initialize(notnull IEntity owner)
	{
		if (m_bIsInitialized)
			return;
		
		//The replication component is a must, as the authority is the only one allowed to perform an update on the container and/or consumer.
		m_ReplicationComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		FactionAffiliationComponent factionAffiliationComponentTemp;
		IEntity parentEntity = owner.GetParent();
		
		while (parentEntity)
		{
			factionAffiliationComponentTemp = FactionAffiliationComponent.Cast(parentEntity.FindComponent(FactionAffiliationComponent));
			
			if (factionAffiliationComponentTemp)
				m_FactionAffiliationComponent = factionAffiliationComponentTemp;
			
			parentEntity = parentEntity.GetParent();
		}
		
		// In the case that no parent has a faction affiliation component, then get the owner's.
		if (!m_FactionAffiliationComponent)
			m_FactionAffiliationComponent = FactionAffiliationComponent.Cast(owner.FindComponent(FactionAffiliationComponent));
		
		Physics physics = owner.GetPhysics();
		vector tempBoundsMaxs, tempBoundsMins;
		
		// ---------------------------------------------------------------- Container initialisation.
		// The container is configured through SCR_ResourceComponentClass.
		// Note: Order matters, containers should be processed first for the optimization on when the self resource right is enabled on them.
		SCR_ResourceComponentClass prefabData = SCR_ResourceComponentClass.Cast(GetComponentData(owner));
		
		if (prefabData)
		{
			// Container instances holding the initial configuration for this component instance's containers.
			array<ref SCR_ResourceContainer> containers = prefabData.GetContainers();
			
			if (containers)
			{
				SCR_ResourceContainer containerInstance;
				typename containerInstanceTypename;
				
				m_aContainerInstances.Reserve(containers.Count());
				
				foreach (SCR_ResourceContainer container: containers)
				{
					containerInstanceTypename = container.Type();
					
					containerInstance = SCR_ResourceContainer.Cast(containerInstanceTypename.Spawn());
					
					// The copying of the container configuration in the prefab data happens here.
					containerInstance.Initialize(owner, container);
					
					int maxPosition = m_aContainerInstances.Count();
					int position;
					EResourceType resourceType = container.GetResourceType();
					EResourceType compareResourceType;
					int comparePosition;
					SCR_ResourceContainer compareContainer;
					
					while (position < maxPosition)
					{
						comparePosition		= position + ((maxPosition - position) >> 1);
						compareContainer	= m_aContainerInstances[comparePosition];
						compareResourceType	= compareContainer.GetResourceType();
						
						if (resourceType > compareResourceType)
							position = comparePosition + 1;
					
						else if (resourceType < compareResourceType)
							maxPosition = comparePosition;
						
						else 
							break;
					}
					
					// Clean container instance to copy the prefab container configuration to.
					m_aContainerInstances.InsertAt(containerInstance, position);
					
					if (container.IsIsolated())
						continue;
					
					containerInstance.GetAxisAlignedBoundingVolume(tempBoundsMins, tempBoundsMaxs);
				}
				
				m_vGridContainersBoundingVolumeMaxs = tempBoundsMaxs;
				m_vGridContainersBoundingVolumeMins = tempBoundsMins;
			}
		}
		
		// ------------------------------------------------------------- Encapsulator initialization.
		if (m_aEncapsulators)
		{
			array<ref SCR_ResourceEncapsulator> encapsulators = {};
			encapsulators.Reserve(m_aEncapsulators.Count());
			
			foreach (SCR_ResourceEncapsulator encapsulator: m_aEncapsulators)
			{
				encapsulator.Initialize(owner);
				
				int position;
				int maxPosition = encapsulators.Count();
				EResourceType resourceType = encapsulator.GetResourceType();
				EResourceType compareResourceType;
				int comparePosition;
				SCR_ResourceEncapsulator compareEncapsulator;
				
				while (position < maxPosition)
				{
					comparePosition	= position + ((maxPosition - position) >> 1);
					compareEncapsulator = encapsulators[comparePosition];
					compareResourceType	= compareEncapsulator.GetResourceType();
					
					if (resourceType > compareResourceType)
						position = comparePosition + 1;
					
					else if (resourceType < compareResourceType)
						maxPosition = comparePosition;
					
					else 
						break;
				}
				
				encapsulators.InsertAt(encapsulator, position);
			}
			
			m_aEncapsulators = encapsulators;
		}
		
		// ----------------------------------------------------------------- Consumer initialization.
		if (m_aConsumers)
		{
			array<ref SCR_ResourceConsumer> consumers = {};
			consumers.Reserve(m_aConsumers.Count());
			
			foreach (SCR_ResourceConsumer consumer: m_aConsumers)
			{
				SCR_ResourceContainer container = GetContainer(consumer.GetResourceType());
				
				if (container && container.IsEncapsulated())
					continue;
				
				consumer.Initialize(owner);
				
				int position;
				int maxPosition = consumers.Count();
				EResourceGeneratorID generatorIdentifier = consumer.GetGeneratorIdentifier();
				EResourceGeneratorID compareGeneratorIdentifier;
				EResourceType resourceType = consumer.GetResourceType();
				EResourceType compareResourceType;
				int comparePosition;
				SCR_ResourceConsumer compareConsumer;
				
				while (position < maxPosition)
				{
					comparePosition	= position + ((maxPosition - position) >> 1);
					compareConsumer = consumers[comparePosition];
					compareGeneratorIdentifier	= compareConsumer.GetGeneratorIdentifier();
					compareResourceType	= compareConsumer.GetResourceType();
					
					if (generatorIdentifier > compareGeneratorIdentifier)
						position = comparePosition + 1;
					else if (generatorIdentifier < compareGeneratorIdentifier)
						maxPosition = comparePosition;
					else if (resourceType > compareResourceType)
						position = comparePosition + 1;
					else if (resourceType < compareResourceType)
						maxPosition = comparePosition;
					else 
						break;
				}
				
				consumers.InsertAt(consumer, position);
			}
			
			m_aConsumers = consumers;
		}
		
		// ---------------------------------------------------------------- Generator initialization.
		if (m_aGenerators)
		{
			array<ref SCR_ResourceGenerator> generators = {};
			generators.Reserve(m_aGenerators.Count());
			
			foreach (SCR_ResourceGenerator generator: m_aGenerators)
			{
				SCR_ResourceContainer container = GetContainer(generator.GetResourceType());
		
				if (container && container.IsEncapsulated())
					continue;
				
				generator.Initialize(owner);
				
				int position;
				int maxPosition = generators.Count();
				EResourceGeneratorID identifier = generator.GetIdentifier();
				EResourceGeneratorID comapareIdentifier;
				EResourceType resourceType = generator.GetResourceType();
				EResourceType compareResourceType;
				int comparePosition;
				SCR_ResourceGenerator compareGenerator;
				
				while (position < maxPosition)
				{
					comparePosition		= position + ((maxPosition - position) >> 1);
					compareGenerator	= generators[comparePosition];
					comapareIdentifier	= compareGenerator.GetIdentifier();
					compareResourceType	= compareGenerator.GetResourceType();
					
					if (identifier > comapareIdentifier)
						position = comparePosition + 1;
					else if (identifier < comapareIdentifier)
						maxPosition = comparePosition;
					else if (resourceType > compareResourceType)
						position = comparePosition + 1;
					else if (resourceType < compareResourceType)
						maxPosition = comparePosition;
					else 
						break;
				}
				
				generators.InsertAt(generator, position);
			}
			
			m_aGenerators = generators;
		}
		
		if (m_bIsAddedToParentBuffered)
			OnAddedToParentEx(GetOwner().GetChildren(), GetOwner().GetParent());
		
		Vehicle vehicle = Vehicle.Cast(GetOwner());
			
		if (vehicle)
			vehicle.GetOnPhysicsActive().Insert(OnVehiclePhysicsActive);
		
		m_bIsInitialized = true;
		
		Replicate();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called on parent entity when child entity is added into hierarchy
	//! \param[in] parent
	//! \param[in] child
	override event protected void OnChildAdded(IEntity parent, IEntity child)
	{
		SCR_ResourceComponent childResourceComponent = SCR_ResourceComponent.Cast(child.FindComponent(SCR_ResourceComponent));
		if (!childResourceComponent)
			return;
		
		Initialize(GetOwner());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Event after entity is allocated and initialized.
	//! \param owner The owner entity
	override event protected void EOnInit(IEntity owner)
	{
		SCR_ResourceContainer container;
		SCR_ResourceContainerQueueBase queue;
		
		foreach (SCR_ResourceEncapsulator encapsulator: m_aEncapsulators)
		{
			queue = encapsulator.GetContainerQueue();
			
			for (int i = queue.GetContainerCount() - 1; i >= 0; --i)
			{
				container = queue.GetContainerAt(i);
				
				if (container.GetResourceValue() == 0.0 && GetGame().GetWorld() && !GetGame().GetWorld().IsEditMode() && container.GetOnEmptyBehavior() == EResourceContainerOnEmptyBehavior.HIDE)
					container.GetComponent().SetIsVisible(false);
			}
		}
		
		bool isCompletelyIsolated = true;
		
		for (int i = m_aContainerInstances.Count() - 1; i >= 0; --i)
		{
			container = m_aContainerInstances[i];
			isCompletelyIsolated &= container.IsIsolated();
		}
		
		if (isCompletelyIsolated)
		{
			UpdateLastPosition();
			
			return;
		}
		
		Vehicle vehicle = Vehicle.Cast(GetOwner().GetRootParent());
		if (vehicle)
			vehicle.GetOnPhysicsActive().Insert(OnVehiclePhysicsActive);
		
		UpdateLastPosition();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Processes and presents the debugging visualization for the consumer and/or container.
	//! Generic/Shared visualization should be processed explicitely here and consumer specific
	//! visualiation should be processed through SCR_ResourceConsumer::DebugDraw(), similarly for
	//! the container with SCR_ResourceContainer::DebugDraw().
	//! A white arrow is drawn explicitely from here with the intent of providing a visual cue of the
	//! extents of the highest range, that is the storage range of the container or the resource range
	//! of the consumer. Whatever is the highest is the height that will be selected for this arrow.
	protected void DebugDraw()
	{
		// Height for the white arrow.
		float height;

		// The white arrow will point to this position, the origin of the owner entity in this case.
		vector origin = GetOwner().GetOrigin();

		// TODO: Cache the height value and only change it on a event basis.
		if (m_aConsumers && m_eDebugVisualizationFlags & EResourceDebugVisualizationFlags.CONSUMER)
		{
			foreach (SCR_ResourceConsumer consumer: m_aConsumers)
			{
				// Processes and presents the debugging visualization for the consumer.
				consumer.DebugDraw();
				
				// Sets the height of the arrow to be the same as the consumer resource range if the current height is less than it.
				height = Math.Max(height, consumer.GetResourceRange());
			}
		}
		
		//! TODO: Cache the height value and only change it on a event basis.
		if (m_aEncapsulators && m_eDebugVisualizationFlags & EResourceDebugVisualizationFlags.ENCAPSULATOR)
		{
			foreach (SCR_ResourceEncapsulator encapsulator: m_aEncapsulators)
			{
				//! Processes and presents the debugging visualization for the encapsulator.
				encapsulator.DebugDraw();
			}
		}
		
		// TODO: Cache the height value and only change it on a event basis.
		if (m_aGenerators && m_eDebugVisualizationFlags & EResourceDebugVisualizationFlags.GENERATOR)
		{
			foreach (SCR_ResourceGenerator generator: m_aGenerators)
			{
				// Processes and presents the debugging visualisation for the generator.
				generator.DebugDraw();
				
				// Sets the height of the arrow to be the same as the generator resource range if the current height is less than it.
				height = Math.Max(height, generator.GetStorageRange());
			}
		}

		// TODO: Cache the height value and only change it on a event basis.
		if (m_aContainerInstances && m_eDebugVisualizationFlags & EResourceDebugVisualizationFlags.CONTAINER)
		{
			foreach (SCR_ResourceContainer container: m_aContainerInstances)
			{
				// Processes and presents the debugging visualisation for the container.
				container.DebugDraw();

				// Sets the height of the arrow to be the same as the container storage range if the
				// current height (Same as resource range of the consumer if a consumer is present on the
				// component or 0.0 otherwise) is less than it.
				//height = Math.Max(height, container.GetStorageRange());
			}
		}
		
		//! Draws the arrow for the visual cue regarding the maximum range.
		Shape.CreateArrow((origin + vector.Up * height), origin, 1.0, 0xFFFFFFFF, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void Replicate()
	{
		if (!m_ReplicationComponent || m_ReplicationComponent.IsProxy())
			return;
		
		m_bIsNetDirty = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Utility method used to replicate the component state.
	//! Warning: This is important for the SCR_ResourceConsumer and SCR_ResourceContainer classes as it is
	//! used to cause the component to replicate, so do not add extra functionality here unless it is
	//! necessary for the replication functionality of both the SCR_ResourceConsumer and SCR_ResourceContainer classes.
	void ReplicateEx()
	{
		if (!m_bIsNetDirty)
			return;
		
		Replication.BumpMe();
		
		m_bIsNetDirty = false;
		
		TEMP_OnInteractorReplicated();
	}
	
#ifdef WORKBENCH 
	override int _WB_GetAfterWorldUpdateSpecs(IEntity owner, IEntitySource src)
	{
		return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called after updating world in Workbench. The entity must be visible in frustum, selected or named.
	//! Used for performing the debug visualization on Workbench Editor's viewport.
	//! \param[in] owner Entity that owns this component.
	//! \param[in] timeSlice Difference of the previous call of this method and the time of the current call.
	override event void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		if (m_bEnableDebugVisualization)
			DebugDraw();
		
		super._WB_AfterWorldUpdate(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	//! Any property value has been changed. You can use editor API here and do some additional edit actions which will be part of the same "key changed" action.
	override event bool _WB_OnKeyChanged(IEntity owner, BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		SCR_ResourceGrid grid = GetGame().GetResourceGrid();
		
		if (m_aConsumers)
		{
			foreach (SCR_ResourceConsumer consumer: m_aConsumers)
			{
				consumer.GetContainerQueue().Clear();
			}
		}
			
		if (m_aGenerators)
		{
			foreach (SCR_ResourceGenerator generator: m_aGenerators)
			{
				generator.GetContainerQueue().Clear();
			}
		}
		
		grid.IncreaseGridUpdateId();
		grid.UnregisterResourceItem(this);
		
		return super._WB_OnKeyChanged(owner, src, key, ownerContainers, parent);
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	void OnVisibilityChanged()
	{
		IEntity owner = GetOwner();
		
		if (m_bIsVisible)
		{
			owner.SetFlags(EntityFlags.VISIBLE | EntityFlags.TRACEABLE, true);
			SCR_PhysicsHelper.ChangeSimulationState(owner, SimulationState.COLLISION, true);
			
			return;
		}
		
		owner.ClearFlags(EntityFlags.VISIBLE | EntityFlags.TRACEABLE, true);
		SCR_PhysicsHelper.ChangeSimulationState(owner, SimulationState.NONE, true);
	}
		
	//------------------------------------------------------------------------------------------------
	override event protected void OnAddedToParent(IEntity child, IEntity parent)
	{
		if (!m_ReplicationComponent || m_ReplicationComponent.IsProxy())
			return;
		
		if (m_bIsInitialized)
			OnAddedToParentEx(child, parent);
		else
			m_bIsAddedToParentBuffered = true;
	}
		
	//------------------------------------------------------------------------------------------------
	protected void OnAddedToParentEx(IEntity child, IEntity parent)
	{
		foreach (SCR_ResourceContainer container: m_aContainerInstances)
		{
			if (!container || container.GetStorageType() != EResourceContainerStorageType.ORPHAN)
				continue;
			
			container.EnableDecay(false);
		}
		
		m_bIsAddedToParentBuffered = false;
		m_bHasParent = true;
		bool isCompletelyIsolated = true;
		
		foreach (SCR_ResourceContainer container: m_aContainerInstances)
		{
			isCompletelyIsolated &= container.IsEncapsulated();
		}
		
		if (isCompletelyIsolated)
			return;
		
		Vehicle vehicle = Vehicle.Cast(GetOwner().GetRootParent());
		
		if (!vehicle)
			return;
		
		vehicle.GetOnPhysicsActive().Insert(OnVehiclePhysicsActive);
	}
	
	//------------------------------------------------------------------------------------------------
	override event protected void OnRemovedFromParent(IEntity child, IEntity parent)
	{
		if (!m_ReplicationComponent || m_ReplicationComponent.IsProxy())
			return;
		
		m_bHasParent = false;
		
		foreach (SCR_ResourceContainer container: m_aContainerInstances)
		{
			if (!container || container.GetStorageType() != EResourceContainerStorageType.ORPHAN)
				continue;
			
			container.EnableDecay(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] owner
	//! \param[in] activeState
	void OnVehiclePhysicsActive(IEntity owner, bool activeState)
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return;
		
		SCR_ResourceSystem updateSystem = SCR_ResourceSystem.Cast(world.FindSystem(SCR_ResourceSystem));
		
		m_bIsOwnerActive = activeState;
	}
	
	//------------------------------------------------------------------------------------------------
	override event protected void OnDelete(IEntity owner)
	{
		UnflagForProcessing();
		
		ArmaReforgerScripted game = GetGame();
		if (!game)
			return;
		
		ChimeraWorld world = ChimeraWorld.CastFrom(game.GetWorld());
		if (!world)
			return;
		
		SCR_ResourceGrid grid = GetGame().GetResourceGrid();
		
		grid.IncreaseGridUpdateId();
		grid.UnregisterResourceItem(this);
		
		SCR_ResourceSystem updateSystem = SCR_ResourceSystem.Cast(world.FindSystem(SCR_ResourceSystem));
		if (!updateSystem)
			return;
		
		delete m_aConsumers;
		delete m_aGenerators;
		delete m_aEncapsulators;
		delete m_aContainerInstances;
	}
}
