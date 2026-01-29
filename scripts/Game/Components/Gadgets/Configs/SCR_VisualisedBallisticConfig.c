class SCR_VisualisedBallisticConfig : SCR_BaseBallisticConfig
{
	[Attribute(desc: "Lowest range for which data will be shown", params: "0 inf")]
	protected int m_iMinRange;

	[Attribute(desc: "Highest range for which data will be shown", params: "0 inf")]
	protected int m_iMaxRange;

	[Attribute(desc: "Change in elevation down range", params: "1 inf")]
	protected int m_iElevationChangeDownRange;

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetMinRange()
	{
		return m_iMinRange;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetMaxRange()
	{
		return m_iMaxRange;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if such data doesnt already exist and only then it will generate values for this config
	//! \return if data for this config is available in SCR_BallisticData.s_aBallistics
	override bool GenerateBallisticData()
	{
		if (!super.GenerateBallisticData())
			return false;

		SCR_BallisticData ballisticData = SCR_BallisticData.s_aBallistics[m_iBallisticDataId];
		if (!ballisticData)
			return false;

		int numberOfEntries = ballisticData.GetNumberOfEntries();
		if (numberOfEntries < 1)
			return false;

		Resource resource = Resource.Load(m_sProjectilePrefab);
		if (!resource.IsValid())
			return false;

		BaseResourceObject resourceObj = resource.GetResource();
		if (!resourceObj)
			return false;

		IEntitySource entitySrc = resourceObj.ToEntitySource();
		if (!entitySrc)
			return false;

		array<float> values;
		float distance, travelTime, aimHeight, angle;
		for (int i; i < numberOfEntries; i++)
		{
			ballisticData.GetValues(i, values);
			if (!values || values.IsEmpty())
				continue;

			distance = values[0] + m_iElevationChangeDownRange * 0.5;
			aimHeight = BallisticTable.GetHeightFromProjectileSource(distance, travelTime, entitySrc, m_fProjectileInitSpeedCoef, m_bDirectFireMode);

			if (travelTime <= 0 || aimHeight <= 0)
				continue;

			angle = Math.Atan2(aimHeight, distance); //radians
			angle = SCR_Math.ConvertFromRadians(angle, m_eUnitType);
			angle = Math.Round(Math.AbsFloat(angle - values[1]));
			travelTime = Math.AbsFloat(travelTime - values[2]);
			travelTime = travelTime.ToString(0,1).ToFloat();
			values.Insert(angle);
			values.Insert(travelTime);
			ballisticData.SetValues(i, values);
		}

		return true;
	}
}
