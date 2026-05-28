[BaseContainerProps(configRoot: true)]
class SCR_ProjectileWindTable
{
	[Attribute(desc: "Data")]
	protected ref array<ref SCR_ProjectileWindData> m_aData;

	//------------------------------------------------------------------------------------------------
	//! \param[in] projectile
	//! \return
	static SCR_ProjectileWindTable GetProjectileWindageData(notnull IEntity projectile)
	{
		ProjectileMoveComponent pmc = ProjectileMoveComponent.Cast(projectile.FindComponent(ProjectileMoveComponent));
		if (!pmc)
			return null;

		ProjectileMoveComponentClass data = ProjectileMoveComponentClass.Cast(pmc.GetComponentData(projectile));
		ResourceName rn = data.GetProjectileWindTable();
		if (rn.IsEmpty())
			return null;

		Resource res = BaseContainerTools.LoadContainer(rn);
		if (!res.IsValid())
			return null;

		BaseContainer container = res.GetResource().ToBaseContainer();
		if (!container)
			return null;

		return SCR_ProjectileWindTable.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] projectileResource
	//! \return
	static SCR_ProjectileWindTable GetProjectileWindageData(ResourceName projectileResource)
	{
		Resource res = Resource.Load(projectileResource);
		if (!res.IsValid())
			return null;

		BaseResourceObject resObject = res.GetResource();
		if (!resObject)
			return null;

		IEntitySource projectileEntitySrc = resObject.ToEntitySource();
		if (!projectileEntitySrc)
			return null;

		return GetProjectileWindageData(projectileEntitySrc);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] projectileEntitySrc
	//! \return
	static SCR_ProjectileWindTable GetProjectileWindageData(notnull IEntitySource projectileEntitySrc)
	{
		ResourceName rn;
		IEntityComponentSource componentSrc;
		for (int i, componentsCount = projectileEntitySrc.GetComponentCount(); i < componentsCount; i++)
		{
			componentSrc = projectileEntitySrc.GetComponent(i);
			if (!componentSrc)
				continue;

			if (!componentSrc.GetClassName().ToType().IsInherited(ProjectileMoveComponent))
				continue;

			componentSrc.Get("ProjectileWindTableConfig", rn);
			break;
		}

		if (rn.IsEmpty())
			return null;

		Resource res = BaseContainerTools.LoadContainer(rn);
		if (!res.IsValid())
			return null;

		BaseContainer container = res.GetResource().ToBaseContainer();
		if (!container)
			return null;

		return SCR_ProjectileWindTable.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
	}

	//------------------------------------------------------------------------------------------------
	//! Fetches data coresponding to the given angle
	//! \param[in] angle
	//! \param[in] windSpeed
	//! \param[in] initSpeedCoef
	//! \return an array with data coresponding to the given angle, or null
	//! [0] angle in radians
	//! [1] distance in meters
	//! [2] peak altitude
	//! [3] azimuth correction in milliradians for the crosswind
	//! [4] distance correction in meters for the parallel wind
	//! [5] impact angle
	array<float> GetDataByAngleDeg(float angle, float windSpeed, float initSpeedCoef = 1)
	{
		angle = SCR_Math.ConvertToRadians(angle, SCR_EOpticsAngleUnits.DEGREES);
		return GetDataByAngle(angle, windSpeed, initSpeedCoef);
	}

	//------------------------------------------------------------------------------------------------
	//! Fetches data coresponding to the given angle
	//! \param[in] angle
	//! \param[in] windSpeed
	//! \param[in] initSpeedCoef
	//! \return an array or null
	//! [0] angle in radians
	//! [1] distance in meters
	//! [2] peak altitude
	//! [3] azimuth correction in milliradians for the crosswind
	//! [4] distance correction in meters for the parallel wind
	//! [5] impact angle
	array<float> GetDataByAngle(float angle, float windSpeed = 10, float initSpeedCoef = 1)
	{
		foreach (SCR_ProjectileWindData data : m_aData)
		{
			if (initSpeedCoef != data.GetInitSpeedCoef())
				continue;

			if (windSpeed != data.GetWindSpeed())
				continue;

			return data.GetDataByAngle(angle);
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Fetches data coresponding to the given distance
	//! \param[in] distance
	//! \param[in] directFire
	//! \param[in] windSpeed
	//! \param[in] initSpeedCoef
	//! \return an array or null
	//! [0] angle in radians
	//! [1] distance in meters
	//! [2] peak altitude
	//! [3] azimuth correction in milliradians for the crosswind
	//! [4] distance correction in meters for the parallel wind
	//! [5] impact angle
	array<float> GetDataByDisance(float distance, bool directFire, float windSpeed, float initSpeedCoef = 1)
	{
		foreach (SCR_ProjectileWindData data : m_aData)
		{
			if (initSpeedCoef != data.GetInitSpeedCoef())
				continue;

			if (windSpeed != data.GetWindSpeed())
				continue;

			return data.GetDataByDisance(distance, directFire);
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] data
	void SCR_ProjectileWindTable(array<ref SCR_ProjectileWindData> data)
	{
		if (data)
			m_aData = data;
	}
}
