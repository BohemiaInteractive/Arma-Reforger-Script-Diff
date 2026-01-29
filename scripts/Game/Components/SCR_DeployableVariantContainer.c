class SCR_DeployableVariantContainer : ScriptAndConfig
{
	[Attribute(defvalue: "0", desc: "Variant id which must be unique", params: "-1 inf")]
	protected int m_iVariantId;

	[Attribute(defvalue: "", desc: "Prefab which will be spawned when this variant is deployed", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	protected ResourceName m_sReplacementPrefab;

	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Model or prefab that will be shown as a preview", params: "xob et", category: "Setup")]
	protected ResourceName m_sPreviewObject;

	[Attribute(desc: "List of prefabs that must be met in order to deploy this entity", category: "Setup")]
	protected ref array<ref SCR_RequiredDeployablePart> m_aRequiredElements;

	[Attribute(desc: "List of additionall prefabs that should be spawned when item is deployed", params: "et", category: "Setup")]
	protected ref array<ref SCR_AdditionalDeployablePart> m_aAdditionalPrefabs;

	[Attribute(desc: "Coordinate describing the bounding box volume used for obstruction check.\nIf left zero, then game will try to load it from data. This will work only if there is an override of the bouding volume.\nEnsure that individual components of this vector are smaller than their counterparts in 'Replacement Prefab Bounding Box Maxs'", category: "Validation")]
	protected vector m_vReplacementPrefabBoundingBoxMins;

	[Attribute(desc: "Coordinate describing the bounding box volume used for obstruction check.\nIf left zero, then game will try to load it from data of replacement prefab. This will work only if there is an override of the bouding volume", category: "Validation")]
	protected vector m_vReplacementPrefabBoundingBoxMaxs;

	[Attribute(defvalue: "0", desc: "Use this part rotation and position for the positioning of the replacement entity")]
	protected bool m_bUsePartRotationAndPosition;

	[Attribute(desc: "Additional offset to the position at which entity will be spawned", category: "Setup")]
	protected vector m_vAdditionalPlacementOffset;

	[Attribute(desc: "Additional rotation that will applied to the spawned entity", category: "Setup")]
	protected vector m_vAdditionalPlacementRotation;

	[Attribute("-1", desc: "Max allowed tilt (in dgrees) from zero value on give axis.\nValue below 0 means that there is no angle restriction", params: "-1 180")]
	protected float m_fMaxAllowedTilt;

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetVariantId()
	{
		return m_iVariantId;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetReplacementPrefab()
	{
		return m_sReplacementPrefab;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns a copy of the list of required prefabs
	//! \param[out] output is not cleared before it is filled
	//! \return number of elements in the output array
	int GetRequiredElements(out notnull array<ref SCR_RequiredDeployablePart> output)
	{
		if (!m_aRequiredElements || m_aRequiredElements.IsEmpty())
			return 0;

		foreach (SCR_RequiredDeployablePart partReq : m_aRequiredElements)
		{
			output.Insert(partReq.DeepClone());
		}

		return output.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns pointer to a shared array of requirements
	array<ref SCR_RequiredDeployablePart> GetRequiredElementsRaw()
	{
		return m_aRequiredElements;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	array<ref SCR_AdditionalDeployablePart> GetAdditionalPrefabList()
	{
		if (!m_aAdditionalPrefabs || m_aAdditionalPrefabs.IsEmpty())
			return null;

		array<ref SCR_AdditionalDeployablePart> output = {};
		foreach (SCR_AdditionalDeployablePart part : m_aAdditionalPrefabs)
		{
			output.Insert(SCR_AdditionalDeployablePart.Cast(part.Clone()));
		}

		return output;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] mins
	//! \param[out] maxs
	//! \return true if system was able to fetch valid data
	bool GetReplacementPrefabBoundingBox(out vector mins, out vector maxs)
	{
		if (m_vReplacementPrefabBoundingBoxMins == vector.Zero && m_vReplacementPrefabBoundingBoxMaxs == vector.Zero)
		{
			Resource res = Resource.Load(m_sReplacementPrefab);
			if (!res)
				return false;

			BaseResourceObject resObject = res.GetResource();
			if (!resObject)
				return false;

			IEntitySource entSource = resObject.ToEntitySource();
			if (!entSource)
				return false;

			IEntityComponentSource compSource;
			BaseContainer boundingBox;
			for (int i, count = entSource.GetComponentCount(); i < count; i++)
			{
				compSource = entSource.GetComponent(i);
				if (!compSource)
					continue;

				if (compSource.GetClassName().ToType() != MeshObject)
					continue;

				boundingBox = compSource.GetObject("BoundingBox");
				if (!boundingBox)
					return false;

				boundingBox.Get("Mins", m_vReplacementPrefabBoundingBoxMins);
				boundingBox.Get("Maxs", m_vReplacementPrefabBoundingBoxMaxs);
				//NOTE: this only works when there is an override of BB, but in other case there is no other way besides spawning it and asking for it :(
			}
		}

		mins = m_vReplacementPrefabBoundingBoxMins;
		maxs = m_vReplacementPrefabBoundingBoxMaxs;
		return mins != vector.Zero || maxs != vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsUsingPartPositionAndRotation()
	{
		return m_bUsePartRotationAndPosition;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetAdditionaPlacementOffset()
	{
		return m_vAdditionalPlacementOffset;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetAdditionaPlacementRotation()
	{
		return m_vAdditionalPlacementRotation;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetMaxAllowedTilt()
	{
		return m_fMaxAllowedTilt;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetPreviewResource()
	{
		return m_sPreviewObject;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_DeployableVariantContainer()
	{
		if (m_vReplacementPrefabBoundingBoxMins != vector.Zero || m_vReplacementPrefabBoundingBoxMaxs != vector.Zero)
		{
			if (m_vReplacementPrefabBoundingBoxMins[0] > m_vReplacementPrefabBoundingBoxMaxs[0])
				Print("[SCR_DeployableVariantContainer] Variant with id = " + m_iVariantId + " 'Replacement Prefab Bounding Box Mins' has larger X component than one in 'Replacement Prefab Bounding Box Maxs'. This is not allowed!", LogLevel.ERROR);

			if (m_vReplacementPrefabBoundingBoxMins[1] > m_vReplacementPrefabBoundingBoxMaxs[1])
				Print("[SCR_DeployableVariantContainer] Variant with id = " + m_iVariantId + " 'Replacement Prefab Bounding Box Mins' has larger Y component than one in 'Replacement Prefab Bounding Box Maxs'. This is not allowed!", LogLevel.ERROR);

			if (m_vReplacementPrefabBoundingBoxMins[2] > m_vReplacementPrefabBoundingBoxMaxs[2])
				Print("[SCR_DeployableVariantContainer] Variant with id = " + m_iVariantId + " 'Replacement Prefab Bounding Box Mins' has larger Z component than one in 'Replacement Prefab Bounding Box Maxs'. This is not allowed!", LogLevel.ERROR);
		}
	}
}