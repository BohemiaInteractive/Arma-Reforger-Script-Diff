class SCR_AISmartActionComponentClass : AISmartActionComponentClass
{
}

void SCR_AIActionUserInvoker_Callback(AIAgent actionUser);
typedef func SCR_AIActionUserInvoker_Callback;
typedef ScriptInvokerBase<SCR_AIActionUserInvoker_Callback> SCR_AIActionUserInvoker;

class SCR_AISmartActionComponent : AISmartActionComponent
{
	protected ref SCR_AIActionUserInvoker Event_EOnActionEnd;
	protected ref SCR_AIActionUserInvoker Event_EOnActionFailed;
	protected SCR_CharacterDamageManagerComponent m_pDamageManager;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	void ReserveAction(AIAgent owner)
	{
		// listen on death of the owner
		IEntity ent = owner.GetControlledEntity();
		if (!ent)
			return;
		m_pDamageManager = SCR_CharacterDamageManagerComponent.Cast(ent.FindComponent(SCR_CharacterDamageManagerComponent));
		if (m_pDamageManager)
			m_pDamageManager.GetOnDamageStateChanged().Insert(OnDamageStateChanged);
		SetActionAccessible(false);	
	}	
	
	//------------------------------------------------------------------------------------------------
	//!
	void ReleaseAction()
	{
		if(m_pDamageManager)
		{
			m_pDamageManager.GetOnDamageStateChanged().Remove(OnDamageStateChanged);
			m_pDamageManager = null;
		}	
		SetActionAccessible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] state
	void OnDamageStateChanged(EDamageState state)
	{
		if (state == EDamageState.DESTROYED)
			ReleaseAction();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionEnd(IEntity owner)
	{
		super.OnActionEnd(owner);
		ReleaseAction();
		if(Event_EOnActionEnd)
			Event_EOnActionEnd.Invoke(GetUser());
	}	
	
	//------------------------------------------------------------------------------------------------
	override void OnActionFailed(IEntity owner)
	{
		super.OnActionFailed(owner);
		ReleaseAction();
		if(Event_EOnActionEnd)
			Event_EOnActionFailed.Invoke(GetUser());
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] createNew
	//! \return
	// Invokers for the completion of the smartaction to be used by the children // 
	SCR_AIActionUserInvoker GetOnActionEnd(bool createNew = true)
	{
		if (!Event_EOnActionEnd && createNew)
			Event_EOnActionEnd = new SCR_AIActionUserInvoker();
		return Event_EOnActionEnd;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] createNew
	//! \return
	SCR_AIActionUserInvoker GetOnActionFailed(bool createNew = true)
	{
		if (!Event_EOnActionFailed && createNew)
			Event_EOnActionFailed = new SCR_AIActionUserInvoker();
		return Event_EOnActionFailed;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (m_pDamageManager)
			m_pDamageManager.GetOnDamageStateChanged().Remove(OnDamageStateChanged);
	}
}
