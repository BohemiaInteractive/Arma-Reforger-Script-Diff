[EntityEditorProps(category: "GameScripted/Components", description: "Mine inventory item component.")]
class SCR_ReusableDeployableInventoryComponentClass : SCR_DeployableInventoryItemInventoryComponentClass
{
}

class SCR_ReusableDeployableInventoryComponent : SCR_DeployableInventoryItemInventoryComponent
{
	[Attribute(defvalue: "1", desc: "Max number of uses that this item has", params: "1 inf")]
	protected int m_iMaxNumberOfUses;

	[RplProp()]
	protected int m_iRemainingUses;

	//------------------------------------------------------------------------------------------------
	int GetRemainingUses()
	{
		return m_iRemainingUses;
	}

	//------------------------------------------------------------------------------------------------
	int GetMaxNumberOfUses()
	{
		return m_iMaxNumberOfUses;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority method that sets new amount of uses that this item has left, and replicates the information to clients
	//! Automatically adjusts the weight of the item to be equal to the remaining fraction of the the initial weight 
	//! \param[in] newValue
	void S_SetRemainingUsesCount(int newValue)
	{
		m_iRemainingUses = newValue;
		float weightChange = GetTotalWeight();
		weightChange = m_iRemainingUses / m_iMaxNumberOfUses * weightChange - weightChange;
		SetAdditionalWeight(weightChange); // reduce the weight by the amount of removed mass
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	private void SCR_ReusableDeployableInventoryComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_iRemainingUses = m_iMaxNumberOfUses;
	}
}
