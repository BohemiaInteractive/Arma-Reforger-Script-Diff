class SCR_RequiredDeployablePart : ScriptAndConfig
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Prefab which is going to be required in order to deploy the main entity", params: "et")]
	protected ResourceName m_sPrefab;

	[Attribute(defvalue: "1", desc: "How many required prefabs need to be found in order to meet this requirement", params: "1 inf 1")]
	protected int m_iNumberOfRequiredPrefabs;

	[Attribute(desc: "Localized string used to tell the player which part he is missing when wont be found")]
	protected LocalizedString m_sPartName;

	[Attribute(defvalue: "1", desc: "Dictates whether elements found for this requirement are meant to be deleted upon deployment")]
	protected bool m_bDeletePartsOnDeployment;

	[Attribute(defvalue: "0", desc: "Should game detach the magazines from weapons that are used for this requirement")]
	protected bool m_bDetachMagazinesWhenUsed;

	protected ref array<IEntity> m_aFoundEntities;

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetPrefab()
	{
		return m_sPrefab;
	}

	//------------------------------------------------------------------------------------------------
	void SetNumberOfRequiredPrefabs(int newValue)
	{
		m_iNumberOfRequiredPrefabs = newValue;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetNumberOfRequiredPrefabs()
	{
		return m_iNumberOfRequiredPrefabs;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	LocalizedString GetPartName()
	{
		return m_sPartName;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	LocalizedString GetMissingPartName()
	{
		return m_sPartName;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] difference between number of found entities and required amount
	//! \return true if requirement is fulfilled
	bool IsRequirementMet(out int difference)
	{
		difference = m_iNumberOfRequiredPrefabs;
		if (!m_aFoundEntities)
			return false;

		difference -= m_aFoundEntities.Count();
		return difference == 0;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetNumberOfFoundEntities()
	{
		if (!m_aFoundEntities)
			return 0;

		return m_aFoundEntities.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool ShouldDeleteFoundEntities()
	{
		return m_bDeletePartsOnDeployment;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	array<IEntity> GetFoundEntities()
	{
		return m_aFoundEntities;
	}

	//------------------------------------------------------------------------------------------------
	//! Evaluates found entity against requirements and if it meets them, then it is stored as found entity
	//! If max number of required elements is already reached, then it will check if this new entity is closer than others, and in such case replace one of them
	//! \param[in] origin position from which search is being done
	//! \param[in] distanceSqToFoundEntity distance from the origin to the position of the found entity
	//! \param[in] foundPrefab
	//! \param[in] foundEntity
	//! \return false if found entity doesnt meet the requirements, otherwise true
	bool EvaluateFoundEntity(vector origin, float distanceSqToFoundEntity, notnull BaseContainer foundPrefab, notnull IEntity foundEntity)
	{
		Resource res = Resource.Load(m_sPrefab);
		if (!res.IsValid())
			return false;

		BaseResourceObject resObject = res.GetResource();
		if (!resObject)
			return false;

		BaseContainer requiredPrefab = resObject.ToBaseContainer();
		if (requiredPrefab != foundPrefab)
			return false;

		if (!m_aFoundEntities || m_aFoundEntities.Count() < m_iNumberOfRequiredPrefabs)
		{
			AddFoundEntity(foundEntity);
			return true;
		}

		foreach (int id, IEntity foundElement : m_aFoundEntities)
		{
			if (vector.DistanceSq(foundElement.GetOrigin(), origin) <= distanceSqToFoundEntity)
				continue;

			ReplaceFoundEntity(foundEntity, id);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Attempts to delete one of the found entities and remove it from the array
	//! \note NOTICE: When entry in the array is deleted, then in its place will be moved last element of the array
	//! \param[in] id of the entity that should be deleted
	//! \return false if deletion failed as there is no entity to delete 
	bool DeleteFoundEntity(int id, notnull IEntity user)
	{
		if (!m_aFoundEntities || !m_aFoundEntities.IsIndexValid(id))
			return false;

		IEntity deletedEntity = m_aFoundEntities[id];
		if (!deletedEntity)
			return false;

		if (m_bDetachMagazinesWhenUsed)
			SCR_MultiPartDeployableItemComponent.TryRemoveMagazineFromWeapons(deletedEntity, user);

		GetGame().GetCallqueue().CallLater(SCR_EntityHelper.DeleteEntityAndChildren, param1: deletedEntity);//to give the time for the RPC calls to be sent
		m_aFoundEntities.Remove(id);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Attempts to add entity to the list of found elements as long as its not already on that list
	//! \param[in] entity
	void AddFoundEntity(notnull IEntity entity)
	{
		if (!m_aFoundEntities)
			m_aFoundEntities = {entity};
		else if (!m_aFoundEntities.Contains(entity))
			m_aFoundEntities.Insert(entity);
	}

	//------------------------------------------------------------------------------------------------
	//! Attempts to replace entity which is in specified place of the array with provided entity
	//! \param[in] entity new element that will replace the old one
	//! \param[in] id place in which new element will replace the old one
	void ReplaceFoundEntity(notnull IEntity entity, int id)
	{
		if (!m_aFoundEntities || m_aFoundEntities.IsEmpty())
			return;

		if (!m_aFoundEntities.IsIndexValid(id) || m_aFoundEntities.Contains(entity))
			return;

		m_aFoundEntities[id] = entity;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns deep copy of this instance
	//! Normal Clone does a shallow copy, thus if you need to clone content of a child class then override this and do it manually
	SCR_RequiredDeployablePart DeepClone()
	{
		return SCR_RequiredDeployablePart.Cast(Clone());
	}
}