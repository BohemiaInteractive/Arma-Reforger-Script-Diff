class SCR_BIKIGeneralHelper
{
	//------------------------------------------------------------------------------------------------
	//! \param[in] itemResourceName
	//! \return
	static string GetName(ResourceName itemResourceName)
	{
		if (!itemResourceName) // IsEmpty()
			return string.Empty;

		Resource resource = Resource.Load(itemResourceName);
		if (!resource.IsValid())
			return string.Empty;

		BaseContainer baseContainer = resource.GetResource().ToBaseContainer();
		if (!baseContainer)
			return string.Empty;

		return GetName(baseContainer);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	static string GetName(notnull BaseContainer baseContainer)
	{
		BaseContainer uiInfo = GetUIInfo(baseContainer);
		string result;
		if (!uiInfo || !uiInfo.Get("Name", result))
			return string.Empty;

		if (result.Contains("#"))
			return WidgetManager.Translate(result);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] resource
	//! \return
	static BaseContainer GetUIInfo(notnull Resource resource)
	{
		if (!resource.IsValid())
			return null;

		return GetUIInfo(resource.GetResource().ToBaseContainer());
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	static BaseContainer GetUIInfo(notnull BaseContainer baseContainer)
	{
		IEntityComponentSource componentSource;

		// character?
		componentSource = SCR_BaseContainerTools.FindComponentSource(baseContainer, SCR_EditableCharacterComponent);
		if (componentSource)
		{
			if (componentSource.GetObject("m_UIInfo"))
				return componentSource.GetObject("m_UIInfo");
		}

		// group?
		componentSource = SCR_BaseContainerTools.FindComponentSource(baseContainer, SCR_EditableGroupComponent);
		if (componentSource)
		{
			if (componentSource.GetObject("m_UIInfo"))
				return componentSource.GetObject("m_UIInfo");
		}

		// vehicle?
		componentSource = SCR_BaseContainerTools.FindComponentSource(baseContainer, SCR_EditableVehicleComponent);
		if (componentSource)
		{
			if (componentSource.GetObject("m_UIInfo"))
				return componentSource.GetObject("m_UIInfo");
		}

		// turret?
		componentSource = SCR_BaseContainerTools.FindComponentSource(baseContainer, WeaponSlotComponent);
		if (componentSource)
		{
			ResourceName resourceName;
			if (componentSource.Get("WeaponTemplate", resourceName))
				return GetUIInfo(Resource.Load(resourceName));
		}

		// inventory item?
		componentSource = SCR_BaseContainerTools.FindComponentSource(baseContainer, InventoryItemComponent);
		if (componentSource)
		{
			BaseContainer baseContainer2 = componentSource.GetObject("Attributes");
			if (baseContainer2)
			{
				baseContainer2 = baseContainer2.GetObject("ItemDisplayName"); // var reuse
				if (baseContainer2)
					return baseContainer2;
			}
		}

		// weapon?
		componentSource = SCR_BaseContainerTools.FindComponentSource(baseContainer, WeaponComponent);
		if (componentSource)
		{
			if (componentSource.GetObject("UIInfo"))
				return componentSource.GetObject("UIInfo");
		}

		// something else?

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	static float GetMass(notnull BaseContainer baseContainer)
	{
		IEntityComponentSource entityComponentSource = SCR_BaseContainerTools.FindComponentSource(baseContainer, InventoryItemComponent);
		if (entityComponentSource)
		{
			BaseContainer subContainer = entityComponentSource.GetObject("Attributes"); // variable reuse
			if (subContainer)
			{
				subContainer = subContainer.GetObject("ItemPhysAttributes"); // variable reuse
				if (subContainer)
				{
					float result;
					if (subContainer.Get("Weight", result))
						return result;
				}
			}
		}

		entityComponentSource = SCR_BaseContainerTools.FindComponentSource(baseContainer, EquipmentStorageComponent); // variable reuse
		if (entityComponentSource)
		{
			BaseContainer subContainer = entityComponentSource.GetObject("Attributes"); // variable reuse
			if (subContainer)
			{
				subContainer = subContainer.GetObject("ItemPhysAttributes"); // variable reuse
				if (subContainer)
				{
					float result;
					if (subContainer.Get("Weight", result))
						return result;
				}
			}
		}

		entityComponentSource = SCR_BaseContainerTools.FindComponentSource(baseContainer, "RigidBody"); // variable reuse
		if (entityComponentSource)
		{
			float result;
			if (entityComponentSource.Get("Mass", result))
				return result;
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	static bool IsCharacter(notnull BaseContainer baseContainer)
	{
		return baseContainer.GetClassName().ToType()
			&& baseContainer.GetClassName().ToType().IsInherited(SCR_ChimeraCharacter);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	static bool IsWeapon(notnull BaseContainer baseContainer)
	{
		return SCR_BaseContainerTools.FindComponentIndex(baseContainer, WeaponComponent) > -1;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	static bool IsMagazine(notnull BaseContainer baseContainer)
	{
		return SCR_BaseContainerTools.FindComponentIndex(baseContainer, MagazineComponent) > -1;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	static bool IsBullet(notnull BaseContainer baseContainer)
	{
		return baseContainer.GetClassName().ToType()
			&& baseContainer.GetClassName().ToType().IsInherited(Projectile)
			&& SCR_BaseContainerTools.FindComponentIndex(baseContainer, ShellMoveComponent) > -1;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	static bool IsGroup(notnull BaseContainer baseContainer)
	{
		return baseContainer.GetClassName().ToType()
			&& baseContainer.GetClassName().ToType().IsInherited(SCR_AIGroup);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	static bool IsWheeled(notnull BaseContainer baseContainer)
	{
		return baseContainer.GetClassName().ToType()
			&& baseContainer.GetClassName().ToType().IsInherited(Vehicle)
			&& SCR_BaseContainerTools.FindComponentIndex(baseContainer, VehicleWheeledSimulation) > -1;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	static bool IsTracked(notnull BaseContainer baseContainer)
	{
		return baseContainer.GetClassName().ToType()
			&& baseContainer.GetClassName().ToType().IsInherited(Vehicle)
			&& SCR_BaseContainerTools.FindComponentIndex(baseContainer, VehicleTrackedSimulation) > -1;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	static bool IsHelicopter(notnull BaseContainer baseContainer)
	{
		return baseContainer.GetClassName().ToType()
			&& baseContainer.GetClassName().ToType().IsInherited(Vehicle)
			&& SCR_BaseContainerTools.FindComponentIndex(baseContainer, VehicleHelicopterSimulation) > -1;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	static bool IsStaticWeapon(notnull BaseContainer baseContainer)
	{
		return baseContainer.GetClassName().ToType()
			&& baseContainer.GetClassName().ToType().IsInherited(Turret)
			&& SCR_BaseContainerTools.FindComponentIndex(baseContainer, SCR_TurretControllerComponent) > -1;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	static bool IsComposition(notnull BaseContainer baseContainer)
	{
		return SCR_BaseContainerTools.FindComponentIndex(baseContainer, SCR_SlotCompositionComponent) > -1;
	}
}
