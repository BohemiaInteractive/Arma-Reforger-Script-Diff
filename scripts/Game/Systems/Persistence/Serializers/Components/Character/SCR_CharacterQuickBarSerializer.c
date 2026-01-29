class SCR_PersistentQuickSlotItem
{
	int m_iIndex;
	UUID m_sEntityId;
}

class SCR_CharacterQuickBarSerializer : BaseInventoryStorageComponentSerializer
{
	[Attribute(desc: "Filter to only handle specific quick slots. e.g. 0-3 are fixed through weapon slots and must not be saved.")]
	ref array<int> m_aSlotIndexFilter;

	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_CharacterInventoryStorageComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_CharacterInventoryStorageComponent charStorage = SCR_CharacterInventoryStorageComponent.Cast(component);

		array<ref SCR_PersistentQuickSlotItem> slots();
		foreach (int idx, auto quickslot : charStorage.GetQuickSlotItems())
		{
			if (!m_aSlotIndexFilter.IsEmpty() && !m_aSlotIndexFilter.Contains(idx))
				continue;

			auto container = SCR_QuickslotEntityContainer.Cast(quickslot);
			if (!container)
				continue;

			const UUID id = GetSystem().GetId(container.GetEntity());
			if (id.IsNull())
				continue;

			SCR_PersistentQuickSlotItem slot();
			slot.m_iIndex = idx;
			slot.m_sEntityId = id;
			slots.Insert(slot);
		}

		context.StartObject("base");
		const ESerializeResult baseResult = super.Serialize(owner, component, context);
		context.EndObject();
		if (baseResult == ESerializeResult.ERROR)
			return baseResult;
		
		if (baseResult == ESerializeResult.DEFAULT && slots.IsEmpty())
			return baseResult;

		context.WriteValue("version", 1);
		const bool prev = context.EnableTypeDiscriminator(false);
		context.WriteValue("quickSlots", slots);
		context.EnableTypeDiscriminator(prev);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		auto charStorage = SCR_CharacterInventoryStorageComponent.Cast(component);

		if (context.DoesObjectExist("base"))
		{
			if (!context.StartObject("base") || 
				!super.Deserialize(owner, component, context) || 
				!context.EndObject())
			{
				return false;
			}
		}

		int version;
		context.Read(version);

		array<ref SCR_PersistentQuickSlotItem> slots();
		const bool prev = context.EnableTypeDiscriminator(false);
		context.ReadValue("quickSlots", slots);
		context.EnableTypeDiscriminator(prev);

		array<int> allSlots();
		allSlots.Copy(m_aSlotIndexFilter);
		
		foreach (auto quickSlot : slots)
		{
			// We are on the storage that must have held the items, and do direct lookup, so any deferred deserialize is not relevant here
			const IEntity entity = IEntity.Cast(GetSystem().FindById(quickSlot.m_sEntityId));
			if (!entity)
				continue;

			charStorage.StoreItemToQuickSlot(entity, quickSlot.m_iIndex, true);
			charStorage.SetQuickSlotInitialized(quickSlot.m_iIndex);
			allSlots.RemoveItem(quickSlot.m_iIndex);
		}

		foreach (int noDataSlotIdx : allSlots)
		{
			charStorage.SetQuickSlotInitialized(noDataSlotIdx);
		}

		return true;
	}
}
