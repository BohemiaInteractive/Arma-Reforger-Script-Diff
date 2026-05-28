class SCR_BIKIWeaponHelper
{
	//------------------------------------------------------------------------------------------------
	//! \param[in] weaponContainer
	//! \return
	static array<float> GetAttachmentMasses(notnull BaseContainer weaponContainer)
	{
		array<IEntityComponentSource> componentSources = {};
		if (SCR_BaseContainerTools.FindComponentSourcesOfClass(weaponContainer, AttachmentSlotComponent, true, componentSources) < 1)
			return {};

		array<float> result = {};
		Resource resource;
		foreach (IEntityComponentSource componentSource : componentSources)
		{
			ResourceName attachmentPrefab;
			if (!componentSource.GetObject("AttachmentSlot") || !componentSource.GetObject("AttachmentSlot").Get("Prefab", attachmentPrefab) || !attachmentPrefab)
				continue;

			resource = Resource.Load(attachmentPrefab);
			if (!resource.IsValid())
				continue;

			result.Insert(SCR_BIKIGeneralHelper.GetMass(resource.GetResource().ToBaseContainer()));
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] weaponContainer
	//! \return
	static array<string> GetAttachmentNames(notnull BaseContainer weaponContainer)
	{
		array<IEntityComponentSource> componentSources = {};
		if (SCR_BaseContainerTools.FindComponentSourcesOfClass(weaponContainer, AttachmentSlotComponent, true, componentSources) < 1)
			return {};

		array<string> result = {};
		foreach (IEntityComponentSource componentSource : componentSources)
		{
			ResourceName attachmentPrefab;
			if (!componentSource.GetObject("AttachmentSlot") || !componentSource.GetObject("AttachmentSlot").Get("Prefab", attachmentPrefab) || !attachmentPrefab) // IsEmpty()
				continue;

			string itemName = SCR_BIKIGeneralHelper.GetName(attachmentPrefab);
			if (itemName)
				result.Insert(itemName);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] weaponContainer
	//! \return
	static float GetInitialSpeedCoefficient(notnull BaseContainer weaponContainer)
	{
		IEntityComponentSource muzzleComponentSource = GetMuzzleComponentSource(weaponContainer);
		if (!muzzleComponentSource)
			return 0;

		float result;
		muzzleComponentSource.Get("BulletInitSpeedCoef", result);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] weaponContainer
	//! \return
	static float GetMass(notnull BaseContainer weaponContainer)
	{
		float result = SCR_BIKIGeneralHelper.GetMass(weaponContainer);
		foreach (float mass : GetAttachmentMasses(weaponContainer))
		{
			if (mass > 0)
				result += mass;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] weaponContainer
	//! \return
	static float GetMaxRateOfFire(notnull BaseContainer weaponContainer)
	{
		IEntityComponentSource muzzleComponentSource = GetMuzzleComponentSource(weaponContainer);
		if (!muzzleComponentSource)
			return 0;

		BaseContainerList baseContainerList = muzzleComponentSource.GetObjectArray("FireModes");
		if (!baseContainerList)
			return 0;

		float result;
		for (int i, count = baseContainerList.Count(); i < count; ++i)
		{
			float rpm;
			if (baseContainerList.Get(i).Get("RoundsPerMinute", rpm) && result < rpm)
				result = rpm;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] weaponContainer
	//! \return
	static array<string> GetFireModes(notnull BaseContainer weaponContainer)
	{
		IEntityComponentSource muzzleComponentSource = GetMuzzleComponentSource(weaponContainer);
		if (!muzzleComponentSource)
			return {};

		BaseContainerList baseContainerList = muzzleComponentSource.GetObjectArray("FireModes");
		if (!baseContainerList)
			return {};

		array<string> result = {};

		for (int i, count = baseContainerList.Count(); i < count; ++i)
		{
			weaponContainer = baseContainerList.Get(i); // variable reuse
			string fireMode;
			if (weaponContainer.Get("UIName", fireMode) && fireMode)
			{
				float maxBurst;
				if (weaponContainer.Get("MaxBurst", maxBurst) && maxBurst > 1)
					result.Insert(string.Format("%1 (%2)", fireMode, maxBurst));
				else
					result.Insert(fireMode);
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] weaponContainer
	//! \return
	static ResourceName GetDefaultMagazine(notnull BaseContainer weaponContainer)
	{
		IEntityComponentSource muzzleComponentSource = GetMuzzleComponentSource(weaponContainer);
		if (!muzzleComponentSource)
			return ResourceName.Empty;

		ResourceName result;
		muzzleComponentSource.Get("MagazineTemplate", result);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] weaponContainer
	//! \return
	static typename GetMagazineWell(notnull BaseContainer weaponContainer)
	{
		IEntityComponentSource muzzleComponentSource = GetMuzzleComponentSource(weaponContainer);
		if (!muzzleComponentSource)
			return typename.Empty;

		BaseContainer baseContainer = muzzleComponentSource.GetObject("MagazineWell");
		if (!baseContainer)
			return typename.Empty;

		return baseContainer.GetClassName().ToType();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] weaponContainer
	//! \return
	static ResourceName GetStaticWeaponWeapon(notnull BaseContainer weaponContainer)
	{
		IEntityComponentSource componentSource = SCR_BaseContainerTools.FindComponentSource(weaponContainer, WeaponSlotComponent);
		if (!componentSource)
			return ResourceName.Empty;

		bool enabled;
		if (!componentSource.Get("Enabled", enabled) || !enabled)
			return ResourceName.Empty;

		ResourceName result;
		componentSource.Get("WeaponTemplate", result);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] weaponContainer
	//! \return array of -x, +x, -y, +y
	static array<float> GetStaticWeaponAngles(notnull BaseContainer weaponContainer)
	{
		IEntityComponentSource componentSource = SCR_BaseContainerTools.FindComponentSource(weaponContainer, SCR_TurretControllerComponent);
		if (!componentSource)
			return { 0, 0, 0, 0 };

		array<float> result = { 0, 0, 0, 0 };
		vector values;

		componentSource.Get("LimitsHoriz", values);
		result[0] = values[0];
		result[1] = values[1];

		componentSource.Get("LimitsVert", values);
		result[2] = values[0];
		result[3] = values[1];

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] weaponContainer
	//! \return
	static array<string> GetAllTurretNames(notnull BaseContainer vehicleContainer)
	{
		array<string> result = {};

		array<IEntityComponentSource> componentSources = {};
		if (SCR_BaseContainerTools.FindComponentSourcesOfClass(vehicleContainer, SlotManagerComponent, false, componentSources) > 0)
		{
			BaseContainerList baseContainerList;
			BaseContainer baseContainer;
			Resource resource;
			foreach (IEntityComponentSource componentSource : componentSources)
			{
				baseContainerList = componentSource.GetObjectArray("Slots");
				if (!baseContainerList)
					continue;

				for (int i, count = baseContainerList.Count(); i < count; ++i)
				{
					baseContainer = baseContainerList.Get(i);
					if (!baseContainer)
						continue;

					bool enabled;
					if (!baseContainer.Get("Enabled", enabled) || !enabled)
						continue;

					ResourceName resourceName;
					if (baseContainer.Get("Prefab", resourceName) && resourceName)
					{
						resource = Resource.Load(resourceName);
						if (resource.IsValid())
							result.InsertAll(GetAllTurretNames(resource.GetResource().ToBaseContainer()));
					}
				}
			}
		}

		if (SCR_BaseContainerTools.FindComponentSourcesOfClass(vehicleContainer, WeaponSlotComponent, false, componentSources) > 0)
		{
			Resource resource;
			foreach (IEntityComponentSource componentSource : componentSources)
			{
				ResourceName resourceName;
				if (componentSource.Get("WeaponTemplate", resourceName) && resourceName)
				{
					resource = Resource.Load(resourceName);
					if (resource.IsValid())
					{
						array<string> turrets = GetAllTurretNames(resource.GetResource().ToBaseContainer());
						if (turrets.IsEmpty())
						{
							string name = SCR_BIKIGeneralHelper.GetName(vehicleContainer);
							if (name)
								result.Insert(name);
						}
						else
						{
							result.InsertAll(turrets);
						}
					}
				}
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	// PROTECTED METHODS (helper's helpers)
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	protected static IEntityComponentSource GetMuzzleComponentSource(notnull BaseContainer weaponContainer)
	{
		array<IEntityComponentSource> componentSources = {};
		if (SCR_BaseContainerTools.FindComponentSourcesOfClass(weaponContainer, MuzzleComponent, true, componentSources) < 1)
			return null;

		return componentSources[0];
	}
}
