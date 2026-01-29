[ComponentEditorProps(category: "GameScripted/Tasks", description: "")]
class SCR_TextsTaskManagerComponentClass : ScriptComponentClass
{
	[Attribute()]
	protected ref array<ref SCR_TextsTaskManagerEntry> m_aEntries;
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] type
	//! \param[out] outInfos
	//! \return
	int GetTexts(ETaskTextType type, out notnull array<ref SCR_UIDescription> outInfos)
	{
		foreach (SCR_TextsTaskManagerEntry entry : m_aEntries)
		{
			if (entry.m_TextType == type)
			{
				int count = entry.m_aTexts.Count();
				for (int i; i < count; i++)
				{
					outInfos.Insert(entry.m_aTexts[i]);
				}

				return count;
			}
		}
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] type
	//! \param[in] index
	//! \return
	SCR_UIDescription GetText(ETaskTextType type, int index)
	{
		foreach (SCR_TextsTaskManagerEntry entry : m_aEntries)
		{
			if (entry.m_TextType == type)
				return entry.m_aTexts[index];
		}

		return null;
	}
}

class SCR_TextsTaskManagerComponent : ScriptComponent
{
	protected static SCR_TextsTaskManagerComponent s_Instance;
	
	static const string TASK_AMOUNT_COMPLETED_TEXT = "#AR-Tasks_AmountCompleted";
	static const string TASK_PROGRESS_TEXT = "#AR-Tasks_StatusProgress-UC";
	static const string TASK_FINISHED_TEXT = "#AR-Tasks_StatusFinished-UC";
	static const string TASK_AVAILABLE_TEXT = "#AR-Tasks_StatusNew-UC";
	static const string TASK_HINT_TEXT = "#AR-Tasks_Hint";
	static const string TASK_CANCELLED_TEXT = "#AR-Tasks_StatusCancelled-UC";
	static const string TASK_COMPLETED_TEXT = "#AR-Tasks_StatusCompleted-UC";
	static const string TASK_FAILED_TEXT = "#AR-Tasks_StatusFailed-UC";
	
	//------------------------------------------------------------------------------------------------
	//! Get instance of this class.
	//! \return SCR_TextsTaskManagerComponent attached to SCR_BaseTaskManager
	static SCR_TextsTaskManagerComponent GetInstance()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	//! Get all texts for given text type.
	//! \param[in] type Text type
	//! \param[out] outInfos Array to be filled with UI infos
	//! \return
	int GetTexts(ETaskTextType type, out notnull array<ref SCR_UIDescription> outInfos)
	{
		SCR_TextsTaskManagerComponentClass prefabData = SCR_TextsTaskManagerComponentClass.Cast(GetComponentData(GetOwner()));
		if (prefabData)
			return prefabData.GetTexts(type, outInfos);
		else
			return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Get specific text for given type.
	//! \param[in] type Text type
	//! \param[in] index Index of text entry
	//! \return UI info
	SCR_UIDescription GetText(ETaskTextType type, int index)
	{
		SCR_TextsTaskManagerComponentClass prefabData = SCR_TextsTaskManagerComponentClass.Cast(GetComponentData(GetOwner()));
		if (prefabData)
			return prefabData.GetText(type, index);
		else
			return null;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_TextsTaskManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (!s_Instance)
			s_Instance = this;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_TextsTaskManagerComponent()
	{
		s_Instance = null;
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ETaskTextType, "m_TextType")]
class SCR_TextsTaskManagerEntry
{
	[Attribute(SCR_Enum.GetDefault(ETaskTextType.NONE), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ETaskTextType))]
	ETaskTextType m_TextType;
	
	[Attribute()]
	ref array<ref SCR_UIDescription> m_aTexts;
}

enum ETaskTextType
{
	NONE,
	CUSTOM,
	MOVE,
	DEFEND,
	ATTACK
}
