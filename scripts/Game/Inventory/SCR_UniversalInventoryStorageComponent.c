class SCR_UniversalInventoryStorageComponentClass : UniversalInventoryStorageComponentClass
{
}

//! Current storage variant allows dynamic scaling of slots and handles Move/Insert/Remove operations
//! it will accept any entity for insertion and will remove/add it's visibility flag when inserted/removed from storage
//! \see CharacterInventoryStorageComponent for example of custom storage inheritance from current class
class SCR_UniversalInventoryStorageComponent : UniversalInventoryStorageComponent
{
	[Attribute( "0", UIWidgets.EditBox, "How much weight it can carry")]
	protected float m_fMaxWeight;
	
	[Attribute( "0", UIWidgets.EditBox, "The ID of slots the inserted items will be visible in")]
	protected ref array<int> m_aSlotsToShow;
	
	[Attribute(desc: "Dictates how the entity gets other storage and sets them as linked storage or how it itself sets it as a linked storage of another storage. Do not use base class!")]
	protected ref SCR_BaseLinkedStorageLogic m_LinkedStorageLogic;
	
	//! Storages that will be automatically closed and opened when the parent (this) storage is closed or opened
	protected ref array<BaseInventoryStorageComponent> m_aLinkedStorages;
	
	#ifndef DISABLE_INVENTORY
	private SCR_ItemAttributeCollection 							m_Attributes;
	protected float 												m_fWeight;
	protected SCR_InventoryStorageManagerComponent					pInventoryManager;
	protected static const int										MIN_VOLUME_TO_SHOW_ITEM_IN_SLOT = 200000;	//!< in cm^3
	
	protected int													m_iNrOfNonRefundableItems = 0;		//used for keeping track of all non refundable items inside of a storage
	
	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Returns how much weight the component can carry
	float GetMaxLoad()
	{
		return m_fMaxWeight;
	}

	//------------------------------------------------------------------------------------------------
	//! Get an estimate of how many times the resource can fit in in this storage
	int GetEstimatedCountFitForResourceWeight(ResourceName prefab)
	{
		const int volumeBased = GetEstimatedCountFitForResource(prefab);
		const float itemWeight = GetWeightFromResource(prefab);
		const int weightBased = Math.Floor((m_fMaxWeight - GetTotalWeight()) / itemWeight);
		
		if (volumeBased < weightBased)
			return volumeBased;

		return weightBased;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks if parent container will allow for storing provided amount of additional weight
	//! \param[in] weight
	protected bool CheckParentWeightLimit(float weight)
	{
		IEntity parent = GetOwner().GetParent();
		if (!parent || ChimeraCharacter.Cast(parent))
			return true;

		SCR_UniversalInventoryStorageComponent parentStorage = SCR_UniversalInventoryStorageComponent.Cast(parent.FindComponent(SCR_UniversalInventoryStorageComponent));
		if (!parentStorage)
			return true;

		return parentStorage.IsAdditionalWeightOk(weight);
	}
	
	//------------------------------------------------------------------------------------------------
	private SCR_ItemAttributeCollection GetAttributeCollection( IEntity item )
	{
		InventoryItemComponent pItemComp = GetItemComponent( item );
		if( !pItemComp )
			return null;
		
		return SCR_ItemAttributeCollection.Cast( pItemComp.GetAttributes() );
	}
	
	//------------------------------------------------------------------------------------------------
	protected InventoryItemComponent GetItemComponent( IEntity pItem )
	{
		return InventoryItemComponent.Cast( pItem.FindComponent( InventoryItemComponent ) );
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsAdditionalWeightOk( float fWeight ) 
	{ 
		if (!m_Attributes)
			return false;
		
		fWeight += GetTotalWeight() - m_Attributes.GetWeight();
		
		return m_fMaxWeight >= fWeight;		
	}	
	
	//------------------------------------------------------------------------------------------------
	override bool CanStoreItem(IEntity item, int slotID)
	{
		if (!super.CanStoreItem(item, slotID))
			return false;

		InventoryItemComponent pItemComp = GetItemComponent(item);
		if (!pItemComp)
			return false;

		bool bVolumeOK = PerformVolumeValidation(item);
		if (!bVolumeOK)
		{
			if(pInventoryManager)	
				pInventoryManager.SetReturnCode(EInventoryRetCode.RETCODE_ITEM_TOO_BIG);
			
			return false;
		}

		bool bWeightOK = IsAdditionalWeightOk(pItemComp.GetTotalWeight());
		if (!bWeightOK)
		{
			if (pInventoryManager)	
				pInventoryManager.SetReturnCode(EInventoryRetCode.RETCODE_ITEM_TOO_HEAVY);
			
			return false;
		}

		IEntity parent = GetOwner().GetParent();
		if (parent)
		{
			if (parent == item.GetParent())
				return true; // If transfered from my parent inventory to me then it has to be within parents allowed weight limit

			return CheckParentWeightLimit(pItemComp.GetTotalWeight());
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanStoreResource(ResourceName resourceName, int slotID)
	{
		if (!super.CanStoreResource(resourceName, slotID))
			return false;
		
		bool bVolumeOK = PerformVolumeValidationForResource(resourceName);
		if (!bVolumeOK)
		{
			if (pInventoryManager)	
				pInventoryManager.SetReturnCode(EInventoryRetCode.RETCODE_ITEM_TOO_BIG);
			
			return false;
		}
		
		float fWeight = GetWeightFromResource(resourceName);
		bool bWeightOK = IsAdditionalWeightOk(fWeight);
		if (!bWeightOK)
		{
			if (pInventoryManager)	
				pInventoryManager.SetReturnCode(EInventoryRetCode.RETCODE_ITEM_TOO_HEAVY);
			
			return false;
		}

		if (GetOwner().GetParent())
			return CheckParentWeightLimit(fWeight);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
 	override bool CanReplaceItem(IEntity nextItem, int slotID)
	{
		if (!super.CanReplaceItem(nextItem, slotID))
			return false;
		
		if (!nextItem)
			return false;
		
		IEntity item = Get(slotID); 
		
		if (!item)
			return false;
		
		// item is the item that is getting replaced by nextItem
		// nextItem is the item that is replacing the item at slotID
		// slotID is is the slot ID for the item that is getting replaced by nextItem
		
		InventoryItemComponent itemComp = GetItemComponent(item);
		if(!itemComp)
			return false;
		
		InventoryItemComponent nextItemComp = GetItemComponent(nextItem);
		if(!nextItemComp)
			return false;
		
		float itemVolume = itemComp.GetTotalVolume();
		float nextItemVolume = nextItemComp.GetTotalVolume();
		float occupiedVolumeWithoutItem = GetOccupiedSpace() - itemVolume;
		
		bool bVolumeOK = occupiedVolumeWithoutItem + nextItemVolume <= GetMaxVolumeCapacity();
		if (!bVolumeOK || !PerformDimensionValidation(nextItem))
		{	
			if (pInventoryManager)
				pInventoryManager.SetReturnCode(EInventoryRetCode.RETCODE_ITEM_TOO_BIG);
			
			return false;
		}
		
		bool bWeightOK = IsAdditionalWeightOk(nextItemComp.GetTotalWeight() - itemComp.GetTotalWeight());
		if (!bWeightOK)
		{
			if (pInventoryManager)
				pInventoryManager.SetReturnCode(EInventoryRetCode.RETCODE_ITEM_TOO_HEAVY);
			
			return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnRemovedFromSlot(IEntity item, int slotID)
	{
		super.OnRemovedFromSlot(item, slotID);
		
		GenericEntity pGenComp = GenericEntity.Cast( item );
		InventoryItemComponent pItemComponent = InventoryItemComponent.Cast(pGenComp.FindComponent(InventoryItemComponent));
		pItemComponent.ShowOwner();
		pItemComponent.EnablePhysics();
		
		m_fWeight -= pItemComponent.GetTotalWeight();
		
		SCR_ItemAttributeCollection refundItemAttributes = SCR_ItemAttributeCollection.Cast(pItemComponent.GetAttributes());
		if (refundItemAttributes && !refundItemAttributes.IsRefundable())
			m_iNrOfNonRefundableItems--;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnAddedToSlot(IEntity item, int slotID)
	{
		super.OnAddedToSlot(item, slotID);
		
		GenericEntity pGenComp = GenericEntity.Cast( item );
		InventoryItemComponent pItemComponent = InventoryItemComponent.Cast(pGenComp.FindComponent(InventoryItemComponent));
		if( !pItemComponent )
			return;	
	
		float fVol = pItemComponent.GetTotalVolume();
		if ( m_aSlotsToShow.Find( slotID ) != -1 )
		{
				pItemComponent.ShowOwner();
		}
		else
		{
			if ( fVol >= MIN_VOLUME_TO_SHOW_ITEM_IN_SLOT )
				pItemComponent.ShowOwner();
		}

		pItemComponent.DisablePhysics();
		pItemComponent.ActivateOwner(false);
		
		m_fWeight += pItemComponent.GetTotalWeight();
		
		SCR_ItemAttributeCollection refundItemAttributes = SCR_ItemAttributeCollection.Cast(pItemComponent.GetAttributes());
		if (refundItemAttributes && !refundItemAttributes.IsRefundable())
			m_iNrOfNonRefundableItems++;
	}
	
	//------------------------------------------------------------------------ LINKED STORAGES ----------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Get a list of all linked storages of this storage
	//! \param[out] linkedStorages Linked storages
	//! \return Amount of linked storages
	int GetLinkedStorages(notnull out array<BaseInventoryStorageComponent> linkedStorages)
	{
		if (!m_aLinkedStorages)
			return 0;
		
		linkedStorages.Copy(m_aLinkedStorages);
		return linkedStorages.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add a linked storage to this storage
	//! \param[in] newLinkedStorage The storage to link
	void AddLinkedStorage(BaseInventoryStorageComponent newLinkedStorage)
	{
		//~ Do not add self as linked storage
		if (newLinkedStorage == this)
			return;
		
		if (!m_aLinkedStorages)
			m_aLinkedStorages = {};
		//~ Already contains the storage
		else if (m_aLinkedStorages.Contains(newLinkedStorage))
			return;

		m_aLinkedStorages.Insert(newLinkedStorage);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if the given storage is a linked child storage
	//! \param[in] storage Storage to check
	//! return True if the storage is a linked child of this storage
	bool IsStorageALinkedChild(notnull BaseInventoryStorageComponent storage)
	{
		return m_aLinkedStorages.Contains(storage);
	}
		
	//------------------------------------------------------------------------------------------------
	int GetNonRefundableItemCount()
	{
		return m_iNrOfNonRefundableItems;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetNonRefundableItemCount(int refundableItemCount)
	{
		m_iNrOfNonRefundableItems = refundableItemCount;
	}
	
	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void OnManagerChanged(InventoryStorageManagerComponent manager)
	{
		super.OnManagerChanged(manager);
		
		pInventoryManager = SCR_InventoryStorageManagerComponent.Cast( manager );
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_UniversalInventoryStorageComponent( IEntityComponentSource src, IEntity ent, IEntity parent )
	{
		m_Attributes = SCR_ItemAttributeCollection.Cast(GetAttributes());
		if (!m_Attributes)
			return;

		m_fWeight = m_Attributes.GetWeight();
		
		if (m_LinkedStorageLogic)
			m_LinkedStorageLogic.Init(this);
	}
	#else
	private SCR_ItemAttributeCollection GetAttributeCollection( IEntity item );
	protected InventoryItemComponent GetItemComponent( IEntity pItem );
	protected bool IsVolumeOk( float fVolume );	
	protected bool IsWeightOk( float fWeight );
	override bool CanStoreItem(IEntity item, int slotID);
	override bool CanStoreResource(ResourceName resourceName, int slotID, int count);
	override bool CanRemoveItem(IEntity item);
	override void OnRemovedFromSlot(IEntity item, int slotID);
	protected override void OnAddedToSlot(IEntity item, int slotID);
	override void OnManagerChanged(InventoryStorageManagerComponent manager);
//	void SCR_UniversalInventoryStorageComponent( IEntityComponentSource src, IEntity ent, IEntity parent );
	#endif	
}
