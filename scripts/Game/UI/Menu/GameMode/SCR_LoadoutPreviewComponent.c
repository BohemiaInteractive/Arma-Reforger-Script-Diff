class SCR_LoadoutPreviewComponent : ScriptedWidgetComponent
{
	[Attribute("{9F18C476AB860F3B}Prefabs/World/Game/ItemPreviewManager.et")]
	protected ResourceName m_sPreviewManager;

	[Attribute("Preview")]
	protected string m_sPreviewWidgetName;

	protected ItemPreviewManagerEntity m_PreviewManager;
	protected ItemPreviewWidget m_wPreview;

	protected bool m_bReloadLoadout;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wPreview = ItemPreviewWidget.Cast(w.FindAnyWidget(m_sPreviewWidgetName));
		m_bReloadLoadout = true;
	}

	protected void DeleteChildrens(IEntity entity, bool deleteRoot)
	{
		if (!entity || !entity.FindComponent(InventoryItemComponent))
			return;

		IEntity child = entity.GetChildren();
		while (child)
		{
			IEntity sibling = child.GetSibling();
			DeleteChildrens(child, true);
			child = sibling;
		}

		if (!entity.IsDeleted() && deleteRoot)
			delete entity;
	}

	//------------------------------------------------------------------------------------------------
	IEntity SetPreviewedLoadout(notnull SCR_BasePlayerLoadout loadout, PreviewRenderAttributes attributes = null)
	{
		if (!m_bReloadLoadout)
			return null;

		ChimeraWorld world = GetGame().GetWorld();
		m_PreviewManager = world.GetItemPreviewManager();

		if (!m_PreviewManager)
		{
			Resource res = Resource.Load(m_sPreviewManager);
			if (res.IsValid())
				GetGame().SpawnEntityPrefabLocal(res, world);

			m_PreviewManager = world.GetItemPreviewManager();
			if (!m_PreviewManager)
			{
				return null;
			}
		}

		ResourceName resName = loadout.GetLoadoutResource();
		if (SCR_PlayerArsenalLoadout.Cast(loadout))
		{
			IEntity previewedEntity = m_PreviewManager.ResolvePreviewEntityForPrefab(resName);
			if (!previewedEntity)
				return previewedEntity;

			SCR_ArsenalManagerComponent arsenalManager;
			if (!SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
				return previewedEntity;

			SCR_PlayerLoadoutData loadoutData = arsenalManager.m_LocalPlayerLoadoutData;
			if (!loadoutData)
				return previewedEntity;

			DeleteChildrens(previewedEntity, false);

			EquipedLoadoutStorageComponent loadoutStorage = EquipedLoadoutStorageComponent.Cast(previewedEntity.FindComponent(EquipedLoadoutStorageComponent));
			if (loadoutStorage)
			{
				for (int i = 0; i < loadoutData.Clothings.Count(); ++i)
				{
					InventoryStorageSlot slot = loadoutStorage.GetSlot(loadoutData.Clothings[i].SlotIdx);
					if (!slot)
						continue;

					Resource resource = Resource.Load(loadoutData.Clothings[i].ClothingPrefab);
					if (!resource)
						continue;

					IEntity cloth = GetGame().SpawnEntityPrefabLocal(resource, previewedEntity.GetWorld());
					if (!cloth)
						continue;

					slot.AttachEntity(cloth);
				}
			}

			IEntity activeWeaponEntity;
			EquipedWeaponStorageComponent weaponStorage = EquipedWeaponStorageComponent.Cast(previewedEntity.FindComponent(EquipedWeaponStorageComponent));
			if (weaponStorage)
			{
				for (int i = 0, weaponsCount = loadoutData.Weapons.Count(); i < weaponsCount; ++i)
				{
					InventoryStorageSlot slot = weaponStorage.GetSlot(loadoutData.Weapons[i].SlotIdx);
					if (!slot)
						continue;

					Resource resource = Resource.Load(loadoutData.Weapons[i].WeaponPrefab);
					if (!resource)
						continue;

					IEntity weapon = GetGame().SpawnEntityPrefabLocal(resource, previewedEntity.GetWorld());
					if (!weapon)
						continue;

					slot.AttachEntity(weapon);

					if (loadoutData.Weapons[i].Active)
						activeWeaponEntity = weapon;
					
					const int attachmentsCount = loadoutData.Weapons[i].Attachments.Count();
					WeaponAttachmentsStorageComponent attachmentsStorage = WeaponAttachmentsStorageComponent.Cast(weapon.FindComponent(WeaponAttachmentsStorageComponent));
					if (!attachmentsStorage || attachmentsStorage.GetSlotsCount() != attachmentsCount)
						continue;

					IEntity attachment;
					ResourceName prefab, current;
					for (int nAttachment = 0; nAttachment < attachmentsCount; ++nAttachment)
					{
						slot = attachmentsStorage.GetSlot(nAttachment);
						attachment = slot.GetAttachedEntity();
						current = SCR_ResourceNameUtils.GetPrefabName(attachment);
						prefab = loadoutData.Weapons[i].Attachments[nAttachment];
						if (current == prefab)
							continue;

						if (attachment)
							delete attachment;

						resource = Resource.Load(prefab);
						if (!resource)
							continue;

						attachment = GetGame().SpawnEntityPrefabLocal(resource, previewedEntity.GetWorld());
						if (!attachment)
							continue;

						slot.AttachEntity(attachment);
					}
				}
			}

			BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(previewedEntity.FindComponent(BaseWeaponManagerComponent));
			if (weaponManager)
			{
				array<WeaponSlotComponent> outSlots = {};
				weaponManager.GetWeaponsSlots(outSlots);
				foreach (WeaponSlotComponent weaponSlot : outSlots)
				{
					if (weaponSlot.GetWeaponEntity() == activeWeaponEntity)
					{
						weaponManager.SelectWeapon(weaponSlot);
						break;
					}
				}
			}

			m_PreviewManager.SetPreviewItem(m_wPreview, previewedEntity, attributes, true);
			return previewedEntity;
		}
		else
		{
			m_PreviewManager.SetPreviewItemFromPrefab(m_wPreview, resName, attributes);
			return m_PreviewManager.ResolvePreviewEntityForPrefab(resName);
		}
	}

	//------------------------------------------------------------------------------------------------
	ItemPreviewManagerEntity GetPreviewManagerEntity()
	{
		return m_PreviewManager;
	}

	//------------------------------------------------------------------------------------------------
	void SetPreviewManagerEntity(ItemPreviewManagerEntity instance)
	{
		m_PreviewManager = instance;
	}

	//------------------------------------------------------------------------------------------------
	ItemPreviewWidget GetItemPreviewWidget()
	{
		return m_wPreview;
	}

	//------------------------------------------------------------------------------------------------
	void SetItemPreviewWidget(ItemPreviewWidget instance)
	{
		m_wPreview = instance;
	}

	//------------------------------------------------------------------------------------------------
	void SetReloadLoadout(bool flag)
	{
		m_bReloadLoadout = flag;
	}
}
