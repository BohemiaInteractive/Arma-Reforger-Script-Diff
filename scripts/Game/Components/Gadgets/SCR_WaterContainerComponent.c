[ComponentEditorProps(category: "GameScripted/Gadgets", description: "Stores the current and maximum water amount of an item.")]
class SCR_WaterContainerComponentClass : ScriptComponentClass
{
}

class SCR_WaterContainerComponent : ScriptComponent
{
	[Attribute("0.75", UIWidgets.EditBox, "Maximum amount of water stored by this item, liters", "0 inf", category: "Water")]
	protected float m_fMaximumWater;

	[Attribute("0.75", UIWidgets.EditBox, "Initial amount of water stored by this item, liters", "0 inf", category: "Water"), RplProp()]
	protected float m_fCurrentWater;

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_WaterContainerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_fMaximumWater = Math.Max(m_fMaximumWater, 0);
		m_fCurrentWater = Math.Clamp(m_fCurrentWater, 0, m_fMaximumWater);
	}

	//------------------------------------------------------------------------------------------------
	float GetMaximumWater()
	{
		return Math.Max(m_fMaximumWater, 0);
	}

	//------------------------------------------------------------------------------------------------
	float GetCurrentWater()
	{
		return Math.Clamp(m_fCurrentWater, 0, GetMaximumWater());
	}

	//------------------------------------------------------------------------------------------------
	float GetWaterFraction()
	{
		float maximumWater = GetMaximumWater();
		if (maximumWater <= 0)
			return 0;

		return GetCurrentWater() / maximumWater;
	}

	//------------------------------------------------------------------------------------------------
	bool IsEmpty()
	{
		return GetCurrentWater() <= 0;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] amount
	//! \return true if water can be removed from the canteen
	bool CanRemoveWater(float amount)
	{
		return amount <= 0 || (CanModifyWater() && GetCurrentWater() >= amount);
	}

	//------------------------------------------------------------------------------------------------
	//!	Add a requested amount of water to the water container, authority method
	//! \param[in] amount
	//! \return amount of water added
	float AddWater_S(float amount)
	{
		if (amount <= 0 || !CanModifyWater())
			return 0;
	
		float waterBefore = GetCurrentWater();
		float waterAfter = Math.Clamp(waterBefore + amount, 0, GetMaximumWater());
		float addedWater = waterAfter - waterBefore;
		if (addedWater <= 0)
			return 0;
	
		m_fCurrentWater = waterAfter;
		Replication.BumpMe();
		return addedWater;
	}

	//------------------------------------------------------------------------------------------------
	//! Remove specified amount of water from the water container, authority method
	//! \param[in] amount
	//! \return how much water was removed
	float RemoveWater_S(float amount)
	{
		if (amount <= 0 || !CanModifyWater())
			return 0;
	
		float waterBefore = GetCurrentWater();
		float waterAfter = Math.Clamp(waterBefore - amount, 0, GetMaximumWater());
		float removedWater = waterBefore - waterAfter;
		if (removedWater <= 0)
			return 0;
	
		m_fCurrentWater = waterAfter;
		Replication.BumpMe();
		return removedWater;
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanModifyWater()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return false;
	
		RplComponent replication = SCR_EntityHelper.GetEntityRplComponent(owner);
		return replication && replication.Role() == RplRole.Authority;
	}
}
