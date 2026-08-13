[SCR_BaseContainerCountedResourceNameTitle("m_iNumberOfRequiredUses", "m_sPrefab", "%2 with %1 uses")]
class SCR_ReusableRequiredDeployablePart : SCR_RequiredDeployablePart
{
	[Attribute(defvalue: "1", desc: "How many many uses does the part need to have to fulfill this requirement.\nNumber that entity can offer is determined by its SCR_ReusableDeployableInventoryComponent.m_iRemainingUses value.", params: "1 inf 1")]
	protected int m_iNumberOfRequiredUses;

	[Attribute(defvalue: "1", desc: "True if assembly should be possible by combining multiple items which individually dont have enough to fulfill the requirement for number of uses.\nFalse if assembly should only be possible by providing an item which individually meets the requirement for number of uses.")]
	protected bool m_bCanCombineItemsForUses;
	
	[Attribute(defvalue: "#AR-UserAction_Assemble_MetersLongItem", desc: "Formatting string to which game will put into %1 value from m_iNumberOfRequiredUses, and to %2 m_sPartName")]
	protected string m_sMissingInfoFormat;

	//------------------------------------------------------------------------------------------------
	//! Evaluates found entity against requirements and if it meets them, then it is stored as found entity
	//! If max number of required elements is already reached, then it will check if this new entity is closer than others, and in such case replace one of them
	//! \param[in] origin position from which search is being done
	//! \param[in] distanceSqToFoundEntity distance from the origin to the position of the found entity
	//! \param[in] foundPrefab
	//! \param[in] foundEntity
	//! \return false if found entity doesnt meet the requirements, otherwise true
	override bool EvaluateFoundEntity(vector origin, float distanceSqToFoundEntity, notnull BaseContainer foundPrefab, notnull IEntity foundEntity)
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

		SCR_ReusableDeployableInventoryComponent newIIC = SCR_ReusableDeployableInventoryComponent.Cast(foundEntity.FindComponent(SCR_ReusableDeployableInventoryComponent));
		if (!newIIC)
			return false;

		int remainingUses = newIIC.GetRemainingUses();
		if (remainingUses < 1)
			return false;

		if (!m_bCanCombineItemsForUses && remainingUses < m_iNumberOfRequiredUses)
			return false;

		int quantityDiff, usesDiff;
		if (!m_aFoundEntities || !IsRequirementMet(quantityDiff, usesDiff))
		{
			AddFoundEntity(foundEntity);
			return true;
		}

		if (usesDiff == 0)
			return false; // if it is perfect then dont play tetris with it to min max the uses

		int maxReplacedNumberOfUses = remainingUses - usesDiff; // minus as usesDiff will be here negative and we need it to be positive

		int replacedId = -1;
		int numberOfUses;
		int highestUsesValue = -1;
		SCR_ReusableDeployableInventoryComponent oldIIC;
		foreach (int id, IEntity foundElement : m_aFoundEntities)
		{
			if (distanceSqToFoundEntity != 0 && vector.DistanceSq(foundElement.GetOrigin(), origin) <= distanceSqToFoundEntity)
				continue;

			oldIIC = SCR_ReusableDeployableInventoryComponent.Cast(foundElement.FindComponent(SCR_ReusableDeployableInventoryComponent));
			numberOfUses = oldIIC.GetRemainingUses();
			if (numberOfUses <= highestUsesValue)
				continue;

			if (numberOfUses > maxReplacedNumberOfUses)
				continue; // cant replace this one as it would remove too many uses

			highestUsesValue = numberOfUses;
			if (remainingUses >= highestUsesValue)
				continue;

			replacedId = id; // replace the one that is the least used up
			if (highestUsesValue == maxReplacedNumberOfUses)
				break;
		}

		if (replacedId < 0)
			return false;

		ReplaceFoundEntity(foundEntity, replacedId);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] difference between number of found entities and required amount
	//! \return true if requirement is fulfilled
	override bool IsRequirementMet(out int difference)
	{
		int usesDiff;
		return IsRequirementMet(difference, usesDiff);
	}

	//------------------------------------------------------------------------------------------------
	bool IsRequirementMet(out int quantityDiff, out int usesDiff)
	{
		quantityDiff = m_iNumberOfRequiredPrefabs;
		if (!m_aFoundEntities)
			return false;

		quantityDiff -= m_aFoundEntities.Count();
		usesDiff = m_iNumberOfRequiredUses;
		SCR_ReusableDeployableInventoryComponent reusableItemComp;
		foreach (IEntity requiredElement : m_aFoundEntities)
		{
			if (!requiredElement)
				continue;

			reusableItemComp = SCR_ReusableDeployableInventoryComponent.Cast(requiredElement.FindComponent(SCR_ReusableDeployableInventoryComponent));
			usesDiff -= reusableItemComp.GetRemainingUses();
		}

		if (usesDiff > 0 && quantityDiff <= 0)
			quantityDiff = 1; // force it to 1 so it shows the player that he is missing at least one element

		return quantityDiff <= 0 && usesDiff <= 0;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	override LocalizedString GetMissingPartName()
	{
		return WidgetManager.Translate(m_sMissingInfoFormat, m_iNumberOfRequiredUses, m_sPartName);
	}

	//------------------------------------------------------------------------------------------------
	//! Attempts to delete one of the found entities and remove it from the array
	//! \note NOTICE: When entry in the array is deleted, then in its place will be moved last element of the array
	//! \param[in] id of the entity that should be deleted
	//! \return false if deletion failed as there is no entity to delete 
	override bool DeleteFoundEntity(int id, notnull IEntity user)
	{
		if (!m_aFoundEntities || !m_aFoundEntities.IsIndexValid(id))
			return false;

		if (m_iNumberOfRequiredUses < 1)
			return true; // if we already satisfied the requirement then quit early

		IEntity deletedEntity = m_aFoundEntities[id];
		if (!deletedEntity)
			return false;

		SCR_ReusableDeployableInventoryComponent iic = SCR_ReusableDeployableInventoryComponent.Cast(deletedEntity.FindComponent(SCR_ReusableDeployableInventoryComponent));
		if (!iic)
			return false;

		int currentRemainingNumberOfUses = iic.GetRemainingUses();
		int remainingNumberOfUses = currentRemainingNumberOfUses - m_iNumberOfRequiredUses;
		m_iNumberOfRequiredUses -= currentRemainingNumberOfUses;

		if (remainingNumberOfUses <= 0) // only delete the entity if it is used up
			return super.DeleteFoundEntity(id, user);

		iic.S_SetRemainingUsesCount(remainingNumberOfUses);
		return true;
	}
}