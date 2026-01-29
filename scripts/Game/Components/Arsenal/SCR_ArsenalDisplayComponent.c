class SCR_ArsenalDisplayComponentClass : SCR_ArsenalComponentClass
{
}

class SCR_ArsenalDisplayComponent : SCR_ArsenalComponent
{
	[Attribute("0", desc: "Type of display data an arsenal item needs in in order to be displayed on this arsenal display", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EArsenalItemDisplayType), category: "Display settings")]
	protected EArsenalItemDisplayType m_eArsenalItemDisplayType;

	[Attribute("4", desc: "Refresh time in seconds. Every x time the arsenal will try to refresh")]
	protected float m_fRefreshDelay;

	protected ref array<SCR_WeaponRackSlotEntity> m_aSlots = {};

	//------------------------------------------------------------------------------------------------
	override void SetSupportedArsenalItemTypes(SCR_EArsenalItemType types)
	{
		ClearArsenal();
		super.SetSupportedArsenalItemTypes(types);
	}

	//------------------------------------------------------------------------------------------------
	override void SetSupportedArsenalItemModes(SCR_EArsenalItemMode modes)
	{
		ClearArsenal();
		super.SetSupportedArsenalItemModes(modes);
	}

	//------------------------------------------------------------------------------------------------
	protected void RegisterSlot(SCR_WeaponRackSlotEntity slot)
	{
		if (!slot || m_aSlots.Find(slot) != -1)
		{
			return;
		}

		m_aSlots.Insert(slot);
	}

	//------------------------------------------------------------------------------------------------
	override void OnFactionChanged(FactionAffiliationComponent owner, Faction previousFaction, Faction newFaction)
	{
		ClearArsenal();
		RefreshArsenal();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnArsenalGameModeTypeChanged(SCR_EArsenalGameModeType newArsenalGameModeType)
	{
		ClearArsenal();
		RefreshArsenal();
	}

	//------------------------------------------------------------------------------------------------
	//! Remove every slot's item
	override void ClearArsenal()
	{
		for (int i = 0; i < m_aSlots.Count(); i++)
		{
			m_aSlots[i].RemoveItem();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Refresh arsenal after m_fRefreshDelay delay
	protected void RefreshUpdate()
	{
		RefreshArsenal(false, GetAssignedFaction());
	}

	//------------------------------------------------------------------------------------------------
	override void RefreshArsenal(bool init = false, SCR_Faction faction = null)
	{
		array<SCR_ArsenalItem> filteredArsenalItems = {};

		//~ Could not get arsenal items
		if (!GetFilteredArsenalItems(filteredArsenalItems, m_eArsenalItemDisplayType))
			return;

		int availableItemCount = filteredArsenalItems.Count();
		int availableSlotCount = m_aSlots.Count();

		SCR_ArsenalItem itemToSpawn;
		SCR_WeaponRackSlotEntity currentSlot = null;
		for (int i = 0; i < availableSlotCount; i++)
		{
			currentSlot = m_aSlots[i];
			if (GetSlotValid(currentSlot))
			{
				continue;
			}
			else if(currentSlot && currentSlot.GetChildren())
			{
				currentSlot.RemoveItem();
			}
			else if (!currentSlot)
			{
				m_aSlots.RemoveOrdered(i--);
				availableSlotCount--;
				continue;
			}

			if (!currentSlot.CanSpawnItem())
			{
				continue;
			}

			int index = i % availableItemCount;
			for (int j = 0; j < availableItemCount; j++)
			{
				itemToSpawn = filteredArsenalItems[index];
				if (GetItemValidForSlot(itemToSpawn.GetItemType(), itemToSpawn.GetItemMode(), currentSlot.GetSlotSupportedItemTypes(), currentSlot.GetSlotSupportedItemModes()))
					break;

				itemToSpawn = null;
				index++;
				if (index >= availableItemCount)
					index = 0;
			}

			if (!itemToSpawn)
				continue;

			currentSlot.SpawnNewItem(itemToSpawn.GetItemResource(), itemToSpawn, itemToSpawn.GetDisplayDataOfType(m_eArsenalItemDisplayType));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetSlotValid(SCR_WeaponRackSlotEntity slotEntity, out bool isEmpty = true)
	{
		if (!slotEntity)
			return false;

		if (slotEntity.GetChildren() && slotEntity.GetChildren().GetPrefabData())
		{
			isEmpty = false;
			return GetItemValidForSlot(slotEntity.GetCurrentOccupiedItemType(), slotEntity.GetCurrentOccupiedItemMode(), slotEntity.GetSlotSupportedItemTypes(), slotEntity.GetSlotSupportedItemModes());
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetItemValidForSlot(SCR_EArsenalItemType itemType, SCR_EArsenalItemMode itemMode, SCR_EArsenalItemType supportedSlotTypes, SCR_EArsenalItemMode supportedSlotModes)
	{
		return GetItemValid(itemType, itemMode) && SCR_Enum.HasPartialFlag(supportedSlotTypes, itemType) && SCR_Enum.HasPartialFlag(supportedSlotModes, itemMode);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		IEntity child = owner.GetChildren();
		while (child)
		{
			SCR_WeaponRackSlotEntity slot = SCR_WeaponRackSlotEntity.Cast(child);
			if (slot)
				RegisterSlot(slot);

			child = child.GetSibling();
		}

		RplComponent rpl = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!rpl || (rpl && rpl.Role() == RplRole.Authority))
		{
			RefreshArsenal(true);

			if (!SCR_Global.IsEditMode())
				GetGame().GetCallqueue().CallLater(RefreshUpdate, m_fRefreshDelay * 1000, true);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;

		super.OnPostInit(owner);

		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);

		if (SCR_Global.IsEditMode())
			return;

		GetGame().GetCallqueue().Remove(RefreshUpdate);
	}
}
