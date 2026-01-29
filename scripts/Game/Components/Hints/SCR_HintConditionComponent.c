[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_HintConditionComponentClass : ScriptComponentClass
{
	[Attribute()]
	protected ref SCR_HintConditionList m_Hints;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	void Init(IEntity owner)
	{
		m_Hints.Init(owner);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	void Exit(IEntity owner)
	{
		m_Hints.Exit(owner);
	}
}

class SCR_HintConditionComponent : ScriptComponent
{
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode(owner))
			return;
		
		SCR_HintConditionComponentClass prefabData = SCR_HintConditionComponentClass.Cast(GetComponentData(owner));
		if (prefabData)
			prefabData.Init(owner);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (SCR_Global.IsEditMode(owner))
			return;
		
		SCR_HintConditionComponentClass prefabData = SCR_HintConditionComponentClass.Cast(GetComponentData(owner));
		if (prefabData)
			prefabData.Exit(owner);
	}
}
