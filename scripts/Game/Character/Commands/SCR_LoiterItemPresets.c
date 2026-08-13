[BaseContainerProps(configRoot: true)]
class SCR_LoiterItemPresets
{
	[Attribute(desc: "Set of predefined configurations used by loiter animations.")]
	protected ref array<ref SCR_LoiterItemDataBase> m_aLoiterItemsData;

	protected ref map<SCR_ELoiterItemID, ref SCR_LoiterItemDataBase> m_mDataMap;

	//------------------------------------------------------------------------------------------------
	//! \param[in] id
	//! \return 
	SCR_LoiterItemDataBase GetData(SCR_ELoiterItemID id)
	{
		return m_mDataMap.Get(id);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_LoiterItemPresets()
	{
		m_mDataMap = new map<SCR_ELoiterItemID, ref SCR_LoiterItemDataBase>();
		foreach(SCR_LoiterItemDataBase data : m_aLoiterItemsData)
		{
			SCR_ELoiterItemID id = data.GetId();
			if (id == SCR_ELoiterItemID.NONE)
				continue; // disabled entry

			if (m_mDataMap.Get(id) != null)
			{
				Debug.Error(string.Format("SCR_LoiterItemPresets encountered duplicated usage of ID = %1 while processing {911269C6C40E808B}Configs/Character/LoiterItemPresets.conf.\nOnly one instance of given ID is allowed, and any duplicate will be discarded!", typename.EnumToString(SCR_ELoiterItemID, id)));
				continue;
			}

			m_mDataMap.Insert(data.GetId(), data);
		}
	}
}
