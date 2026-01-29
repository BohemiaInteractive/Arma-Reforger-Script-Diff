class SCR_FieldManualUI_WeaponStatsHelper
{
	protected string m_sDisplayName;
	protected string m_sDescription;
	protected ResourceName m_sInventoryIcon; // not set atm

	protected int m_iDefaultSightDistanceSetting;
	protected ref array<string> m_aFireModes;
	int m_iMuzzleVelocity;
	protected int m_iRateOfFire;
	protected float m_fMass;
	protected ref array<int> m_aSightDistanceSettings;

	//------------------------------------------------------------------------------------------------
	//! \return
	string GetDisplayName()
	{
		return m_sDisplayName;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetInventoryIcon()
	{
		return m_sInventoryIcon;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	string GetDescription()
	{
		return m_sDescription;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	array<string> GetFireModes()
	{
		return m_aFireModes;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetRateOfFire()
	{
		return m_iRateOfFire;
	}

	//------------------------------------------------------------------------------------------------
	//! in kg
	float GetMass()
	{
		return m_fMass;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	array<int> GetSightDistanceSettings()
	{
		return m_aSightDistanceSettings;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetDefaultSightDistanceSetting()
	{
		return m_iDefaultSightDistanceSetting;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] weaponResource
	void SCR_FieldManualUI_WeaponStatsHelper(ResourceName weaponResource)
	{
		if (!weaponResource)
			return;

		Resource resource = Resource.Load(weaponResource);
		if (!resource.IsValid())
			return;

		IEntity entity = GetGame().SpawnEntityPrefabLocal(resource, null, null);
		if (!entity)
			return;

		WeaponComponent weaponComponent = WeaponComponent.Cast(entity.FindComponent(WeaponComponent));
		if (!weaponComponent)
		{
			delete entity;
			return;
		}

		m_iDefaultSightDistanceSetting = Math.Round(weaponComponent.GetCurrentSightsZeroing()); // NOT zeroing, dammit
		m_iMuzzleVelocity = weaponComponent.GetInitialProjectileSpeed();
		UIInfo weaponComponentUIInfo = weaponComponent.GetUIInfo();
		if (weaponComponentUIInfo)
		{
			m_sDisplayName = weaponComponentUIInfo.GetName();
			m_sDescription = weaponComponentUIInfo.GetDescription();
		}

		m_aFireModes = {};
		m_aSightDistanceSettings = {};

//		WeaponAttachmentsStorageComponent weaponAttachmentsStorageComponent = WeaponAttachmentsStorageComponent.Cast(entity.FindComponent(WeaponAttachmentsStorageComponent));
//		if (weaponAttachmentsStorageComponent)
//		{
//			ItemAttributeCollection collection = weaponAttachmentsStorageComponent.GetAttributes();
//			UIInfo uiInfo = collection.GetUIInfo();
//			WeaponUIInfo weaponUIInfo = WeaponUIInfo.Cast(uiInfo);
//			if (weaponUIInfo)
//			{
				//Warka: Hotfix, as the GetWeaponIconPath() was removed from the WeaponUIInfo class
				//m_sInventoryIcon = weaponUIInfo.GetWeaponIconPath();
//			}
//		}

		MuzzleComponent muzzleComponent = MuzzleComponent.Cast(weaponComponent.FindComponent(MuzzleComponent));
		if (muzzleComponent)
		{
			array<BaseFireMode> fireModes = {};
			muzzleComponent.GetFireModesList(fireModes);

			foreach (BaseFireMode fireMode : fireModes)
			{
				if (fireMode.GetBurstSize() == 0)
					continue;

				m_aFireModes.Insert(fireMode.GetUIName());
				float shotSpan = fireMode.GetShotSpan();
				if (shotSpan > 0)
				{
					int rateOfFire = Math.Round(60 / shotSpan);
					if (rateOfFire > m_iRateOfFire)
						m_iRateOfFire = rateOfFire;
				}
			}
		}

		WeaponAttachmentsStorageComponent storageComponent = WeaponAttachmentsStorageComponent.Cast(entity.FindComponent(WeaponAttachmentsStorageComponent));
		if (storageComponent)
		{
			m_fMass = storageComponent.GetTotalWeight();
		}

		BaseSightsComponent sightsComponent = weaponComponent.GetSights();
		if (sightsComponent)
		{
			// TODO when API is available
		}

		delete entity;
	}
};
