class SCR_ComboRequiredDeployableParts : SCR_RequiredDeployablePart
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Second prefab that must be found in order to complete this requirement", params: "et")]
	protected ResourceName m_sOtherPrefab;

	[Attribute(defvalue: "1", desc: "How many required prefabs need to be found in order to meet this requirement", params: "1 inf 1")]
	protected int m_iNumberOfOtherRequiredPrefabs;

	[Attribute(desc: "Localized string used to tell the player which part he is missing when wont be found")]
	protected LocalizedString m_sOtherPartName;

	protected ref array<IEntity> m_aOtherFoundEntities;

	//------------------------------------------------------------------------------------------------
	override bool EvaluateFoundEntity(vector origin, float distanceSqToFoundEntity, notnull BaseContainer foundPrefab, notnull IEntity foundEntity)
	{
		if (super.EvaluateFoundEntity(origin, distanceSqToFoundEntity, foundPrefab, foundEntity))
			return true;

		ResourceName mainPartPrefab = m_sPrefab;
		m_sPrefab = m_sOtherPrefab;

		array<IEntity> mainPartStorage = m_aFoundEntities;
		m_aFoundEntities = m_aOtherFoundEntities;

		int requiredNumberOfMainParts = m_iNumberOfRequiredPrefabs;
		m_iNumberOfRequiredPrefabs = m_iNumberOfOtherRequiredPrefabs;

		bool finalResult = super.EvaluateFoundEntity(origin, distanceSqToFoundEntity, foundPrefab, foundEntity);

		m_aOtherFoundEntities = m_aFoundEntities;
		m_sPrefab = mainPartPrefab;
		m_aFoundEntities = mainPartStorage;
		m_iNumberOfRequiredPrefabs = requiredNumberOfMainParts;

		return finalResult;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsRequirementMet(out int difference)
	{
		if (!super.IsRequirementMet(difference))
			return false;

		difference = m_iNumberOfOtherRequiredPrefabs;
		if (!m_aOtherFoundEntities)
			return false;

		difference -= m_aOtherFoundEntities.Count();

		return difference == 0;
	}

	//------------------------------------------------------------------------------------------------
	override LocalizedString GetMissingPartName()
	{
		if (!m_aFoundEntities || m_aFoundEntities.Count() < m_iNumberOfRequiredPrefabs)
			return m_sPartName;

		return m_sOtherPartName;
	}

	//------------------------------------------------------------------------------------------------
	override bool DeleteFoundEntity(int id, notnull IEntity user)
	{
		bool result = super.DeleteFoundEntity(id, user);
		
		array<IEntity> mainPartStorage = m_aFoundEntities;
		m_aFoundEntities = m_aOtherFoundEntities;

		result &= super.DeleteFoundEntity(id, user);

		m_aFoundEntities = mainPartStorage;
		return result;
	}

	//------------------------------------------------------------------------------------------------
	override array<IEntity> GetFoundEntities()
	{
		array<IEntity> combinedList;
		if (m_aOtherFoundEntities)
		{
			combinedList = {};
			combinedList.Copy(m_aOtherFoundEntities);
		}

		if (!m_aFoundEntities)
			return combinedList;

		if (!combinedList)
			return m_aFoundEntities;

		combinedList.InsertAll(m_aFoundEntities);
		return combinedList;
	}
}