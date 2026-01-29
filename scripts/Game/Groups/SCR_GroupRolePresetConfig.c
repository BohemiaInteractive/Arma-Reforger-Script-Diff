[BaseContainerProps()]
class SCR_GroupRolePresetConfig : SCR_GroupPreset
{
	[Attribute(desc: "Localized group role")]
	protected LocalizedString m_sGroupRoleName;

	[Attribute(defvalue:"", uiwidget: UIWidgets.ResourceNamePicker, params: "et", desc: "Playable loadouts in preset. This loadout prefabs must be also set in loadout manager")]
	protected ref array<ResourceName> m_aLoadoutResources;

	[Attribute(defvalue:"1", desc: "Player can create this group, if false it will be not visible in create group menu")]
	protected bool m_bCanBeCreatedByPlayer;

	[Attribute(defvalue:"1", desc: "Can player join to group, if false, join button will be disabled, join requests cannot be sent to the group")]
	protected bool m_bCanPlayerJoin;

	//------------------------------------------------------------------------------------------------
	//! \return localized group role name
	LocalizedString GetGroupRoleName()
	{
		return m_sGroupRoleName;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] loadout
	//! \return true if loadout is in preset
	bool IsLoadoutInGroup(notnull SCR_FactionPlayerLoadout loadout)
	{
		return m_aLoadoutResources.Contains(loadout.GetDefaultLoadoutResource());
	}

	//------------------------------------------------------------------------------------------------
	//! Can be group created by player
	//! \return false - it will be not visible in create group menu
	bool CanBeCreatedByPlayer()
	{
		return m_bCanBeCreatedByPlayer;
	}

	//------------------------------------------------------------------------------------------------
	//! Can player join to group
	//! \return false - join button will be disabled and join requests cannot be sent to the group
	bool CanPlayerJoin()
	{
		return m_bCanPlayerJoin;
	}

	//------------------------------------------------------------------------------------------------
	array<ResourceName> GetLoadouts()
	{
		array<ResourceName> loadouts = {};
		loadouts.Copy(m_aLoadoutResources);

		return loadouts;
	}
}
