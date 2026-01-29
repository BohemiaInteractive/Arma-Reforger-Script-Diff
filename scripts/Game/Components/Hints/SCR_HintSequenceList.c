[BaseContainerProps(configRoot: true)]
class SCR_HintSequenceList
{
	[Attribute()]
	protected ref array<ref SCR_HintUIInfo> m_aHints;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] info
	//! \return
	int FindHint(SCR_HintUIInfo info)
	{
		return m_aHints.Find(info);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	int CountHints()
	{
		return m_aHints.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] index
	//! \return
	SCR_HintUIInfo GetHint(int index)
	{
		return m_aHints[index];
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_HintSequenceList()
	{
		for (int i, count = m_aHints.Count(); i < count; i++)
		{
			m_aHints[i].InitSequence(i + 1, count);
		}
	}
}
