[EntityEditorProps(category: "GameScripted/Campaign", description: "A weapon rack slot to be filled with a weapon.", color: "0 0 255 255")]
class SCR_WeaponRackSlotEntityClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_WeaponRackSlotEntity : GenericEntity
{
	[Attribute("0", UIWidgets.Flags, "", enums: ParamEnumArray.FromEnum(SCR_EArsenalItemType))]
	protected SCR_EArsenalItemType m_eSupportedItemTypes;
	
	[Attribute("0", UIWidgets.Flags, "", enums: ParamEnumArray.FromEnum(SCR_EArsenalItemMode))]
	protected SCR_EArsenalItemMode m_eSupportedItemModes;
	
	protected SCR_EArsenalItemType m_eCurrentItemType;
	protected SCR_EArsenalItemMode m_eCurrentItemMode;
	
	//------------------------------------------------------------------------------------------------
	SCR_EArsenalItemType GetSlotSupportedItemTypes()
	{
		return m_eSupportedItemTypes;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_EArsenalItemMode GetSlotSupportedItemModes()
	{
		return m_eSupportedItemModes;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_EArsenalItemMode GetCurrentOccupiedItemType()
	{
		return m_eCurrentItemType;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_EArsenalItemMode GetCurrentOccupiedItemMode()
	{
		return m_eCurrentItemMode;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Delete the slot's item (first found child)
	void RemoveItem()
	{
		IEntity child = GetChildren();
		if (child)
		{
			RplComponent.DeleteRplEntity(child, false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return whether it is possible to create an item
	//! \param deleteExisting if provided true, return true
	//! \return true if the rack has no child entity, false otherwise
	bool CanSpawnItem(bool deleteExisting = false)
	{
		if (deleteExisting)
			return true;
		
		return !GetChildren();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create a new item in this slot.\
	//! If an item already exists and deleteExisting is false, nothing is done.\
	//! If an item already exists and deleteExisting is true, it is deleted and replaced.
	//! \param itemResource the wanted item's resource - must not be null
	//! \param arsenalData must not be null
	//! \param itemDisplayData if provided, 
	//! \param deleteExisting if true, delete the existing item if there is one
	// TODO: notnull itemResource & arsenalData
	void SpawnNewItem(Resource itemResource, SCR_ArsenalItem arsenalData, SCR_ArsenalItemDisplayData itemDisplayData = null, bool deleteExisting = false)
	{
		IEntity child = GetChildren();
		if (child)
		{
			if (deleteExisting)
				RplComponent.DeleteRplEntity(child, false);
			else
				return;
		}
		
		EntitySpawnParams params = new EntitySpawnParams();
		params.Parent = this;
		params.TransformMode = ETransformMode.WORLD;
		
		IEntity item = GetGame().SpawnEntityPrefab(itemResource, GetWorld(), params);
		if (!item)
			return;
		
		// Since item is in hierarchy in which parent (slotEntity) is not traceable (also has no volume), we'll have
		// to clear its proxy flag so we can trace the item safely. Proxy flag is reset on pickup via item slots
		item.ClearFlags(EntityFlags.PROXY);
		
		vector positionOffset, rotationOffset = vector.Zero;
		if (itemDisplayData)
		{
			positionOffset = itemDisplayData.GetItemOffset();
			rotationOffset = itemDisplayData.GetItemRotation();
		}
		
		// set weapon rotation
		item.SetAngles(rotationOffset);
		
		// offset the item locally
		vector mat[4];
		item.GetLocalTransform(mat);
		mat[3] = mat[3] + positionOffset;
		item.SetLocalTransform(mat);
		
		if (item.FindComponent(MagazineComponent))
			item.SetFlags(EntityFlags.PROXY);
		
		item.ClearFlags(EntityFlags.ACTIVE);
		
		InventoryItemComponent itemComp = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (itemComp)
		{
			itemComp.DisablePhysics();
		}
		
		item.Update();
		
		m_eCurrentItemType = arsenalData.GetItemType();
		m_eCurrentItemMode = arsenalData.GetItemMode();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_WeaponRackSlotEntity(IEntitySource src, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_WeaponRackSlotEntity()
	{
	}

};
