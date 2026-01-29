class SCR_BaseBallisticConfig : ScriptAndConfig
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Prfab from which system will extrapolate the ballistic data", params: "et class=Projectile")]
	protected ResourceName m_sProjectilePrefab;

	[Attribute(desc: "Static text that will be shown for this config")]
	protected string m_sDisplayedText;

	[Attribute(desc: "Resolution of generated ballistic data - smaller value == highier resolution == more memory consumed", params: "1 inf")]
	protected int m_iRangeStep;

	[Attribute("1", desc: "Coeficient of the initial projectile speed", params: "0.1 inf")]
	protected float m_fProjectileInitSpeedCoef;

	[Attribute("0", desc: "Average dispersion", params: "0 inf")]
	protected float m_fStandardDispersion;

	[Attribute(SCR_Enum.GetDefault(SCR_EOpticsAngleUnits.MILS_NATO), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EOpticsAngleUnits))]
	protected string m_sUnitType;

	[Attribute(desc: "Is this config is for direct or indirect fire mode")]
	protected bool m_bDirectFireMode;

	[Attribute("0", desc: "Minimal allowed angle at which this projectile can be fired [deg]", params: "0 inf")]
	protected float m_fMinFireAngle;

	[Attribute("85", desc: "Max allowed angle at which this projectile can be fired [deg]", params: "0 inf")]
	protected float m_fMaxFireAngle;

	[Attribute("{A37CF52DBA874559}UI/Imagesets/WeaponInfo/WeaponInfo_Ammo.imageset", UIWidgets.ResourceNamePicker, "Set of images that contain ammo type icon", ".imageset", category: "Layouts")]
	protected ResourceName m_sAmmoTypeImageSet;

	[Attribute(desc: "Quad name from the {A37CF52DBA874559}UI/Imagesets/WeaponInfo/WeaponInfo_Ammo.imageset", category: "Layouts")]
	protected string m_sAmmoTypeQuadName;

	[Attribute("32", desc: "Size of the ammo type image", params: "1 inf", category: "Layouts")]
	protected int m_iAmmoTypeImageSize;

	protected SCR_EOpticsAngleUnits m_eUnitType = -1;
	protected int m_iBallisticDataId = -1;
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetProjectilePrefab()
	{
		return m_sProjectilePrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetDisplayedText()
	{
		return m_sDisplayedText;
	}

	//------------------------------------------------------------------------------------------------
	int GetRangeStep()
	{
		return m_iRangeStep;
	}

	//------------------------------------------------------------------------------------------------
	float GetProjectileInitSpeedCoef()
	{
		return m_fProjectileInitSpeedCoef;
	}

	//------------------------------------------------------------------------------------------------
	float GetStandardDispersion()
	{
		return m_fStandardDispersion;
	}

	//------------------------------------------------------------------------------------------------
	bool IsGenerated()
	{
		return m_iBallisticDataId > -1;
	}

	//------------------------------------------------------------------------------------------------
	bool IsDirectFire()
	{
		return m_bDirectFireMode;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetAmmoTypeImageSet()
	{
		return m_sAmmoTypeImageSet;
	}

	//------------------------------------------------------------------------------------------------
	string GetAmmoTypeQuadName()
	{
		return m_sAmmoTypeQuadName;
	}

	//------------------------------------------------------------------------------------------------
	int GetAmmoTypeImageSize()
	{
		return m_iAmmoTypeImageSize;
	}

	//------------------------------------------------------------------------------------------------
	int GetBallisticDataId()
	{
		return m_iBallisticDataId;
	}

	//------------------------------------------------------------------------------------------------
	SCR_EOpticsAngleUnits GetUnitType()
	{
		if (m_eUnitType < 0)
			m_eUnitType = typename.StringToEnum(SCR_EOpticsAngleUnits, m_sUnitType);

		return m_eUnitType;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if such data doesnt already exist and only then it will generate values for this config
	//! \return if data for this config is available in SCR_BallisticData.s_aBallistics
	bool GenerateBallisticData()
	{
		GetUnitType();

		if (VerifyDataExistence(m_iBallisticDataId))
			return true;

		if (m_iRangeStep < 1)
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

		float distance, travelTime, aimHeight, angle;
		array<ref array<float>> ballisticValues = {};
		int numberOfFailedAttempts = 10;
		while (numberOfFailedAttempts > 0)
		{
			distance += m_iRangeStep;

			aimHeight = BallisticTable.GetHeightFromProjectileSource(distance, travelTime, entitySrc, m_fProjectileInitSpeedCoef, m_bDirectFireMode);
			angle = Math.Atan2(aimHeight, distance); //radians

			if (travelTime <= 0 || angle <= 0)
			{
				numberOfFailedAttempts--;
				continue;
			}

			angle = Math.Round(SCR_Math.ConvertFromRadians(angle, m_eUnitType));
			travelTime = travelTime.ToString(0,1).ToFloat();

			ballisticValues.Insert({distance, angle, travelTime});
		}

		float edgeAngle, edgeAngleRad;
		if (!m_bDirectFireMode)
		{
			edgeAngleRad = SCR_Math.ConvertToRadians(45.5, SCR_EOpticsAngleUnits.DEGREES);
			edgeAngle = Math.Round(SCR_Math.ConvertFromRadians(edgeAngleRad, m_eUnitType));
		}

		if (ballisticValues.IsEmpty())
			return false;

		int edgeElementId;
		float cutoffRadAngle = SCR_Math.ConvertToRadians(ballisticValues[edgeElementId][1], m_eUnitType);
		vector edgeValue = GenerateEdgeValues(entitySrc, cutoffRadAngle);
		if (edgeValue != vector.One * -1)
		{
			float distanceDiff = Math.AbsFloat(edgeValue[0] - ballisticValues[edgeElementId][0]);
			if (distanceDiff <= m_iRangeStep && distanceDiff > m_iRangeStep * 0.2)
				ballisticValues.InsertAt({edgeValue[0], edgeValue[1], edgeValue[2]}, 0);
		}

		edgeElementId = ballisticValues.Count() - 1;

		cutoffRadAngle = SCR_Math.ConvertToRadians(ballisticValues[edgeElementId][1], m_eUnitType);
		edgeValue = GenerateEdgeValues(entitySrc, cutoffRadAngle, true);
		if (edgeValue != vector.One * -1)
		{
			float distanceDiff = Math.AbsFloat(edgeValue[0] - ballisticValues[edgeElementId][0]);
			if (distanceDiff <= m_iRangeStep && distanceDiff > m_iRangeStep * 0.2)
				ballisticValues.Insert({edgeValue[0], edgeValue[1], edgeValue[2]});
		}

		SCR_BallisticData ballisticData = new SCR_BallisticData(ballisticValues, m_sProjectilePrefab, m_bDirectFireMode, m_iRangeStep, m_fProjectileInitSpeedCoef);
		if (!SCR_BallisticData.s_aBallistics)
			SCR_BallisticData.s_aBallistics = {};

		SCR_BallisticData.s_aBallistics.Insert(ballisticData);
		m_iBallisticDataId = SCR_BallisticData.s_aBallistics.Count() - 1;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected vector GenerateEdgeValues(notnull IEntitySource entitySrc, float cutoffRadAngle, bool upper = false)
	{
		float angleRad;
		if (m_bDirectFireMode)
		{
			if (upper)
				angleRad = SCR_Math.ConvertToRadians(m_fMaxFireAngle, SCR_EOpticsAngleUnits.DEGREES);
			else
				angleRad = SCR_Math.ConvertToRadians(m_fMinFireAngle, SCR_EOpticsAngleUnits.DEGREES);
		}
		else
		{
			if (!upper)
				angleRad = SCR_Math.ConvertToRadians(m_fMaxFireAngle, SCR_EOpticsAngleUnits.DEGREES);
			else
				angleRad = SCR_Math.ConvertToRadians(m_fMinFireAngle, SCR_EOpticsAngleUnits.DEGREES);
		}

		float distance, angle, travelTime;
		for (float f = angleRad; f < cutoffRadAngle;)
		{
			distance = BallisticTable.GetDistanceOfProjectileSource(f, travelTime, entitySrc, m_fProjectileInitSpeedCoef, m_bDirectFireMode);

			if (distance > 0 && travelTime > 0)
			{
				distance = Math.Floor(distance) * 10;
				angle = Math.Round(SCR_Math.ConvertFromRadians(f, m_eUnitType));
				travelTime = Math.Floor(travelTime * 10) * 0.1;
				return {distance, angle, travelTime};
			}

			if (float.AlmostEqual(f, cutoffRadAngle))
				break;

			f = Math.Lerp(f, cutoffRadAngle, 0.1);
		}

		return vector.One * -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if data with provided id already exists in SCR_BallisticData.s_aBallistics
	//! \param[in,out] id position in SCR_BallisticData.s_aBallistics that will be verified if it maches this config
	//! \return true if such ballistic data for this config exists in SCR_BallisticData.s_aBallistics
	bool VerifyDataExistence(inout int id = -1)
	{
		if (!SCR_BallisticData.s_aBallistics)
			return false;

		if (id >= 0 && SCR_BallisticData.s_aBallistics.IsIndexValid(id))
		{
			SCR_BallisticData ballisticData = SCR_BallisticData.s_aBallistics[id];
			if (ballisticData.IsForDirectFireMode() == m_bDirectFireMode
				&& ballisticData.GetRangeStep() == m_iRangeStep
				&& ballisticData.GetProjectileName() == m_sProjectilePrefab)
				return true;

			id = -1;
		}

		foreach (int i, SCR_BallisticData ballisticData : SCR_BallisticData.s_aBallistics)
		{
			if (ballisticData.IsForDirectFireMode() != m_bDirectFireMode)
				continue;

			if (ballisticData.GetRangeStep() != m_iRangeStep)
				continue;

			if (ballisticData.GetProjectileInitSpeedCoef() != m_fProjectileInitSpeedCoef)
				continue;

			if (ballisticData.GetProjectileName() != m_sProjectilePrefab)
				continue;

			id = i;
			return true;
		}

		return false;
	}
}
