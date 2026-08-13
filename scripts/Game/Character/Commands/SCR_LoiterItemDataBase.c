[BaseContainerProps(configRoot: true)]
class SCR_LoiterItemDataBase
{
	[Attribute(defvalue: SCR_ELoiterItemID.NONE.ToString(), desc: "Unique id of this loiter animation item data.\nValue NONE will disable that entry.", uiwidget: UIWidgets.ComboBox, enumType: SCR_ELoiterItemID)]
	protected SCR_ELoiterItemID m_eID;

	//------------------------------------------------------------------------------------------------
	//! \return the id of the config
	SCR_ELoiterItemID GetId()
	{
		return m_eID;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns prefabs for the purpose of the loiter animation
	//! \param[in] animationOwner entity which is playing the animation
	//! \param[in] seed value which will be the same for all clients during playback of this instance of the animation
	//! \return list of prefabs that were spawned, or null if none were spawned
	array<IEntity> SpawnPrefabs(notnull IEntity animationOwner, int seed);
}
