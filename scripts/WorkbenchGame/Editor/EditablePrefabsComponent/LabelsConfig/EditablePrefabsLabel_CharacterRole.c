[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityLabel, "m_Label")]
class EditablePrefabsLabel_CharacterRole : EditablePrefabsLabel_Base
{
	[Attribute()]
	protected int m_MedicItemCountRequired;

	protected ref array<EEditableEntityLabel> m_aRoleLabels = {};

	//------------------------------------------------------------------------------------------------
	override bool GetLabelValid(WorldEditorAPI api, IEntitySource entitySource, IEntityComponentSource componentSource, string targetPath, EEditableEntityType entityType, notnull array<EEditableEntityLabel> authoredLabels, out EEditableEntityLabel label)
	{
		if (authoredLabels.Contains(EEditableEntityLabel.ROLE_LEADER))
			return false;

		m_aRoleLabels.Clear();

		CheckWeapons(entitySource);
		CheckInventory(entitySource);
		CheckLoadout(entitySource);

		return GetRoleLabel(label);
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckWeapons(IEntitySource entitySource)
	{
		array<ref array<IEntityComponentSource>> weaponSlotComponents = {};
		array<string> componentTypeArray = { "CharacterWeaponSlotComponent" };
		int weaponSlotCount = SCR_BaseContainerTools.FindComponentSources(entitySource, componentTypeArray, weaponSlotComponents);

		array<IEntityComponentSource> weaponSlotComponentSources = weaponSlotComponents.Get(0);

		if (!weaponSlotComponentSources)
			return;

		Resource resource;
		IEntitySource weaponSource;
		IEntityComponentSource weaponComponentSource;
		foreach	(IEntityComponentSource weaponSlotComponent : weaponSlotComponentSources)
		{
			ResourceName weaponPrefab;
			if (weaponSlotComponent.Get("WeaponTemplate", weaponPrefab))
			{
				if (!weaponPrefab)
					continue;

				resource = Resource.Load(weaponPrefab);
				if (!resource.IsValid())
					continue;

				weaponSource = SCR_BaseContainerTools.FindEntitySource(resource);
				if (!weaponSource)
					continue;

				weaponComponentSource = SCR_BaseContainerTools.FindComponentSource(weaponSource, "WeaponComponent");
				if (!weaponComponentSource)
					continue;

				EWeaponType weaponType;
				if (weaponComponentSource.Get("WeaponType", weaponType))
					AddLabelForWeaponType(weaponType);

				CheckWeaponAttachments(weaponComponentSource);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] weaponEntitySource
	void CheckWeaponAttachments(IEntityComponentSource weaponEntitySource)
	{
		BaseContainer weaponComponent = weaponEntitySource.ToBaseContainer();
		BaseContainerList weaponComponents = weaponComponent.GetObjectArray("components");

		// Check WeaponComponent child components
		BaseContainer container;
		for (int i, count = weaponComponents.Count(); i < count; i++)
		{
			container = weaponComponents.Get(i);
			if (container && container.GetClassName() == "AttachmentSlotComponent") // TODO: inherit?
			{
				string attachmentType;
				container.Get("AttachmentType", attachmentType);
				if (attachmentType == "underbarrel")
				{
					container = container.GetObject("AttachmentSlot"); // variable reuse
					ResourceName attachmentPrefab;

					// TODO check if underbarrel attachment is actually a grenade launcher.
					if (container && container.Get("Prefab", attachmentPrefab))
						m_aRoleLabels.Insert(EEditableEntityLabel.ROLE_GRENADIER);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AddLabelForWeaponType(EWeaponType weaponType)
	{
		switch (weaponType)
		{
			case EWeaponType.WT_RIFLE:
			{
				m_aRoleLabels.Insert(EEditableEntityLabel.ROLE_RIFLEMAN);
				break;
			}
			case EWeaponType.WT_MACHINEGUN:
			{
				m_aRoleLabels.Insert(EEditableEntityLabel.ROLE_MACHINEGUNNER);
				break;
			}
			case EWeaponType.WT_ROCKETLAUNCHER:
			{
				m_aRoleLabels.Insert(EEditableEntityLabel.ROLE_ANTITANK);
				break;
			}
			case EWeaponType.WT_GRENADELAUNCHER:
			{
				m_aRoleLabels.Insert(EEditableEntityLabel.ROLE_GRENADIER);
				break;
			}
			case EWeaponType.WT_SNIPERRIFLE:
			{
				m_aRoleLabels.Insert(EEditableEntityLabel.ROLE_SHARPSHOOTER);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entitySource
	void CheckLoadout(IEntitySource entitySource)
	{
		IEntityComponentSource inventoryManagerComponent = SCR_BaseContainerTools.FindComponentSource(entitySource, BaseLoadoutManagerComponent);
		if (!inventoryManagerComponent)
			return;

		BaseContainerList slotList = inventoryManagerComponent.GetObjectArray("Slots");
		if (!slotList)
			return;

		for (int i, count = slotList.Count(); i < count; i++)
		{
			BaseContainer slot = slotList.Get(i);
			LoadoutAreaType slotArea;
			if (!slot.Get("AreaType", slotArea))
				continue;

			ResourceName slotPrefab;
			if (slotArea.IsInherited(LoadoutBackpackArea) && slot.Get("Prefab", slotPrefab))
			{
				Resource slotResource = Resource.Load(slotPrefab);
				IEntityComponentSource radioComponentSource = SCR_BaseContainerTools.FindComponentSource(slotResource, SCR_RadioComponent);
				ERadioCategory radioCategory;
				if (radioComponentSource && radioComponentSource.Get("m_iRadioCategory", radioCategory) && radioCategory == ERadioCategory.MANPACK)
					m_aRoleLabels.Insert(EEditableEntityLabel.ROLE_RADIOOPERATOR);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entitySource
	void CheckInventory(IEntitySource entitySource)
	{
		IEntityComponentSource inventoryManagerComponent = SCR_BaseContainerTools.FindComponentSource(entitySource, SCR_InventoryStorageManagerComponent);
		if (!inventoryManagerComponent)
			return;

		BaseContainerList inventoryItems = inventoryManagerComponent.GetObjectArray("InitialInventoryItems");
		if (!inventoryItems)
			return;

		int medicItemCount = 0;
		for (int i, count = inventoryItems.Count(); i < count; i++)
		{
			BaseContainer inventoryItem = inventoryItems.Get(i);

			array<ResourceName> prefabsToSpawn = {};
			inventoryItem.Get("PrefabsToSpawn", prefabsToSpawn);

			if (!prefabsToSpawn)
				continue;

			IEntityComponentSource consumableComponentSource;
			BaseContainer consumableEffect;
			foreach (ResourceName prefabToSpawn : prefabsToSpawn)
			{
				Resource inventoryPrefab = Resource.Load(prefabToSpawn);
				if (!inventoryPrefab.IsValid())
					continue;

				consumableComponentSource = SCR_BaseContainerTools.FindComponentSource(inventoryPrefab, SCR_ConsumableItemComponent);
				if (consumableComponentSource)
				{
					consumableEffect = consumableComponentSource.GetObject("m_ConsumableEffect");
					if (consumableEffect)
					{
						SCR_EConsumableType consumableType;
						if (consumableEffect.Get("m_eConsumableType", consumableType) && consumableType == SCR_EConsumableType.BANDAGE)
							medicItemCount++;
					}
				}
			}
		}

		if (medicItemCount >= m_MedicItemCountRequired)
			m_aRoleLabels.Insert(EEditableEntityLabel.ROLE_MEDIC);
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetRoleLabel(out EEditableEntityLabel label)
	{
		label = EEditableEntityLabel.NONE;

		if (m_aRoleLabels.Contains(EEditableEntityLabel.ROLE_RADIOOPERATOR))
			label = EEditableEntityLabel.ROLE_RADIOOPERATOR;
		else if (m_aRoleLabels.Contains(EEditableEntityLabel.ROLE_ANTITANK))
			label = EEditableEntityLabel.ROLE_ANTITANK;
		else if (m_aRoleLabels.Contains(EEditableEntityLabel.ROLE_MACHINEGUNNER))
			label = EEditableEntityLabel.ROLE_MACHINEGUNNER;
		else if (m_aRoleLabels.Contains(EEditableEntityLabel.ROLE_GRENADIER))
			label = EEditableEntityLabel.ROLE_GRENADIER;
		else if (m_aRoleLabels.Contains(EEditableEntityLabel.ROLE_SHARPSHOOTER))
			label = EEditableEntityLabel.ROLE_SHARPSHOOTER;
		else if (m_aRoleLabels.Contains(EEditableEntityLabel.ROLE_MEDIC))
			label = EEditableEntityLabel.ROLE_MEDIC;
		else if (m_aRoleLabels.Contains(EEditableEntityLabel.ROLE_RIFLEMAN))
			label = EEditableEntityLabel.ROLE_RIFLEMAN;

		return label != EEditableEntityLabel.NONE;
	}
}
