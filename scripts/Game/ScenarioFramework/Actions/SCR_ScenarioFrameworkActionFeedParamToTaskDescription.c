[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionFeedParamToTaskDescription : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Name of the slot task to influence the description parameter")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "Which Prefabs and how many of them will be converted to a description string")]
	ref array<ref SCR_ScenarioFrameworkPrefabFilterCount> m_aPrefabFilter;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!m_Getter && object)
		{
			entity = object;
		}
		else if (m_Getter)
		{
			SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
			if (!entityWrapper)
				return;

			entity = entityWrapper.GetValue();
			if (!entity)
				return;
		}

		SCR_ScenarioFrameworkSlotTask slotTask = SCR_ScenarioFrameworkSlotTask.Cast(entity.FindComponent(SCR_ScenarioFrameworkSlotTask));
		if (!slotTask || !slotTask.m_TaskLayer)
			return;

		string descriptionExtension;

		Resource resource;
		IEntitySource entitySource;
		string displayName;
		IEntityComponentSource editableEntitySource;
		IEntityComponentSource weaponEntitySource;
		IEntityComponentSource inventoryEntitySource;

		foreach (SCR_ScenarioFrameworkPrefabFilterCount filter : m_aPrefabFilter)
		{
			resource = Resource.Load(filter.m_sSpecificPrefabName);
			if (!resource || !resource.IsValid())
				continue;

			entitySource = SCR_BaseContainerTools.FindEntitySource(resource);
			if (!entitySource)
				return;

			editableEntitySource = SCR_EditableEntityComponentClass.GetEditableEntitySource(entitySource);
			weaponEntitySource = SCR_ComponentHelper.GetWeaponComponentSource(entitySource);
			inventoryEntitySource = SCR_ComponentHelper.GetInventoryItemComponentSource(entitySource);

			if (editableEntitySource)
			{
				SCR_EditableEntityUIInfo editableEntityUiInfo = SCR_EditableEntityComponentClass.GetInfo(editableEntitySource);
				if (editableEntityUiInfo)
					displayName = editableEntityUiInfo.GetName();
			}
			else if (weaponEntitySource)
			{
				WeaponUIInfo weaponEntityUiInfo = SCR_ComponentHelper.GetWeaponComponentInfo(weaponEntitySource);
				if (weaponEntityUiInfo)
					displayName = weaponEntityUiInfo.GetName();
			}
			else if (inventoryEntitySource)
			{
				SCR_ItemAttributeCollection inventoryEntityUiInfo = SCR_ComponentHelper.GetInventoryItemInfo(inventoryEntitySource);
				if (inventoryEntityUiInfo)
				{
					UIInfo uiInfo = inventoryEntityUiInfo.GetUIInfo();
					if (uiInfo)
						displayName = uiInfo.GetName();
					else
						continue;
				}
				else
				{
					continue;
				}
			}

			int count = filter.m_iPrefabCount;
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(descriptionExtension))
				descriptionExtension += count.ToString() + "x " + displayName;
			else
				descriptionExtension += ", " + count.ToString() + "x " + displayName;
		}

		slotTask.SetOverriddenObjectDisplayName(descriptionExtension);
		array<LocalizedString> descriptionParams = {};
		descriptionParams.Insert(descriptionExtension);
		slotTask.m_TaskLayer.m_Task.SetTaskDescription(slotTask.m_TaskLayer.m_Task.GetTaskDescription(), descriptionParams);
	}
}