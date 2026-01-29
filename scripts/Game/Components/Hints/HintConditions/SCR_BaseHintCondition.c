[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_BaseHintCondition
{
	[Attribute()]
	protected ref SCR_HintUIInfo m_Info;
	
	protected bool m_bInit;
	protected Managed m_Owner;
	
	//------------------------------------------------------------------------------------------------
	protected void Activate()
	{
		SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
		if (!hintManager)
			return;
		
		if (!hintManager.WasShown(m_Info))
		{
			//--- Call with delay, so other invoker listeners which may have reacted to the same event have time to react first
			GetGame().GetCallqueue().CallLater(SCR_HintManagerComponent.ShowHint, 1, false, m_Info, false, false);
		}
		else
		{
			ExitCondition(m_Owner, hintManager);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Deactivate()
	{
		SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
		if (!hintManager)
			return;
		
		hintManager.Hide(m_Info);
		
		//--- Stop tracking
		if (hintManager.WasShown(m_Info))
			ExitCondition(m_Owner, hintManager);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	//! \param[in] hintManager
	void InitCondition(Managed owner, SCR_HintManagerComponent hintManager)
	{
		if (!hintManager.WasShown(m_Info))
		{
			m_bInit = true;
			m_Owner = owner;
			OnInitCondition(owner);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	//! \param[in] hintManager
	void ExitCondition(Managed owner, SCR_HintManagerComponent hintManager)
	{
		if (m_bInit)
		{
			m_bInit = false;
			OnExitCondition(owner);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInitCondition(Managed owner);

	//------------------------------------------------------------------------------------------------
	protected void OnExitCondition(Managed owner);
	
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_BaseHintCondition()
	{
		if (m_Info && m_Info.GetType() == -1)
		{
			m_Info.Log("Cannot initialize hint condition, hint type is undefined! ", LogLevel.WARNING);	
		}
	}
}

class SCR_BaseContainerHintCondition : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		BaseContainer info = source.GetObject("m_Info");
		if (!info)
			return false;
		
		EHint type;
		info.Get("m_Type", type);
		title = typename.EnumToString(EHint, type);
		
		if (type == 0)
		{
			string name;
			info.Get("Name", name);
			title += string.Format(" (\"%1\")", name);
		}

		return true;
	}
}
