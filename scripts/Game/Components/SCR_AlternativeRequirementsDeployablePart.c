class SCR_AlternativeRequirementsDeployablePart : SCR_RequiredDeployablePart
{
	[Attribute(desc: "Alternative Requirement")]
	protected ref SCR_RequiredDeployablePart m_AlternativeRequirement;

	[Attribute(defvalue: "1", desc: "Can game use X elements from this requirement and Y elements from alternative requirement to reach this required number of elements")]
	protected bool m_bCanMix;

	protected bool m_bRequirementMet;

	//------------------------------------------------------------------------------------------------
	override bool EvaluateFoundEntity(vector origin, float distanceSqToFoundEntity, notnull BaseContainer foundPrefab, notnull IEntity foundEntity)
	{
		bool isAnAlternative = m_AlternativeRequirement.EvaluateFoundEntity(origin, distanceSqToFoundEntity, foundPrefab, foundEntity);
		if (isAnAlternative)
		{
			if (!m_bCanMix)
				return true;
		}
		else
		{
			bool result = super.EvaluateFoundEntity(origin, distanceSqToFoundEntity, foundPrefab, foundEntity);
			if (result && m_iNumberOfRequiredPrefabs == m_aFoundEntities.Count())
				m_bRequirementMet = true;

			return result;
		}

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
	override bool IsRequirementMet(out int difference)
	{
		bool result = super.IsRequirementMet(difference);
		if (m_bCanMix || result)
			return result;

		return m_AlternativeRequirement.IsRequirementMet(difference);
	}

	//------------------------------------------------------------------------------------------------
	override LocalizedString GetMissingPartName()
	{
		if (m_bCanMix)
			return m_sPartName;

		if (m_AlternativeRequirement.GetNumberOfFoundEntities() > 0 && super.GetNumberOfFoundEntities() == 0)
			return m_AlternativeRequirement.GetMissingPartName();
		
		return m_sPartName;
	}

	//------------------------------------------------------------------------------------------------
	override int GetNumberOfFoundEntities()
	{
		if (m_bCanMix)
			return super.GetNumberOfFoundEntities();

		int difference;
		if (super.IsRequirementMet(difference))
			return m_aFoundEntities.Count();

		return m_AlternativeRequirement.GetNumberOfFoundEntities();
	}

	//------------------------------------------------------------------------------------------------
	override array<IEntity> GetFoundEntities()
	{
		if (m_bCanMix)
			return super.GetFoundEntities();

		array<IEntity> combinedList = m_AlternativeRequirement.GetFoundEntities();
		if (!combinedList)
			return super.GetFoundEntities();

		if (!m_aFoundEntities)
			return combinedList;

		combinedList.InsertAll(m_aFoundEntities);
		return combinedList;
	}

	//------------------------------------------------------------------------------------------------
	override bool DeleteFoundEntity(int id, notnull IEntity user)
	{
		if (!m_bCanMix)
		{//if we cannot mix 
			//and we completed this requirement
			if (m_bRequirementMet)
				return super.DeleteFoundEntity(id, user);

			//otherwise we operate on the alternative requirement
			return m_AlternativeRequirement.DeleteFoundEntity(id, user);
		}

		//if we have not found anything then pass the delition request to the alternative
		if (!m_aFoundEntities || m_aFoundEntities.IsEmpty())
			return m_AlternativeRequirement.DeleteFoundEntity(id, user);

		int numberOfFoundElements = m_aFoundEntities.Count();
		if (id >= numberOfFoundElements)
			return m_AlternativeRequirement.DeleteFoundEntity(id - numberOfFoundElements, user);

		return super.DeleteFoundEntity(id, user);
	}

	//------------------------------------------------------------------------------------------------
	override SCR_RequiredDeployablePart DeepClone()
	{
		SCR_AlternativeRequirementsDeployablePart copy = SCR_AlternativeRequirementsDeployablePart.Cast(super.DeepClone());

		copy.AlternativeRequirementOverride(m_AlternativeRequirement.DeepClone());
		return copy;
	}

	//------------------------------------------------------------------------------------------------
	void AlternativeRequirementOverride(SCR_RequiredDeployablePart newRequirement)
	{
		m_AlternativeRequirement = newRequirement;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_AlternativeRequirementsDeployablePart()
	{
		if (m_bCanMix && m_AlternativeRequirement)
			m_AlternativeRequirement.SetNumberOfRequiredPrefabs(m_iNumberOfRequiredPrefabs);
	}
}