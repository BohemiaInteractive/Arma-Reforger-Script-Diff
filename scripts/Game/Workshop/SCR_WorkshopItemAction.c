/*
SCR_WorkshopItemAction adds a concept of an asynchronous operation which can be
started, paused, canceled, resumed, failed, restarted.

It also provides event handlers for state changes.

The action can be interacted with through public API.


From this class other classes are derived which represent specific operation types.


!!! Don't create an action yourself! Use the action returned by SCR_WorkshopItem instead.
*/

class SCR_WorkshopItemAction
{	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Old untyped script invokers, const ints instead of an enum, and a code architecture issue with m_Wrapper, as SCR_WorkshopItem and SCR_WorkshopItemAction include each other and are supposed to be tigtly coupled, but they can also end up holding unrelated instances: add actions persisting after failing/canceling for UI display purposes, and states often updating the next frame at the earliest, and it gets rather hard to ensure what data is available at what time
	
	ref ScriptInvoker m_OnActivated = new ScriptInvoker;
	ref ScriptInvoker m_OnCompleted = new ScriptInvoker;
	ref ScriptInvoker m_OnCanceled = new ScriptInvoker;
	ref ScriptInvoker m_OnResumed = new ScriptInvoker;
	ref ScriptInvoker m_OnPaused = new ScriptInvoker;
	ref ScriptInvoker m_OnFailed = new ScriptInvoker;
	ref ScriptInvoker m_OnChanged = new ScriptInvoker;
	
	// ---- States ----
	protected const int STATE_INACTIVE = 0;
	protected const int STATE_ACTIVE = 1;
	protected const int STATE_COMPLETED = 2;
	protected const int STATE_FAILED = 3;
	protected const int STATE_CANCELED = 4;
	protected const int STATE_PAUSED = 5;
	
	// ---- Protected ----
	protected int m_State;

	SCR_WorkshopItem m_Wrapper;
	
//---- REFACTOR NOTE END ----
	
	protected bool m_bAttached;
	protected ref Managed m_UserData; // Arbitrary user data, use GetUserData() SetUserData().

	// Cached basic data about the addon. Because action might outlive the WorkshopItem object.
	protected string m_sAddonName;
	protected string m_sAddonId;
	
	//-----------------------------------------------------------------------------------------------
	// !!! Don't create an action yourself! Use API in SCR_WorkshopItem instead.
	void SCR_WorkshopItemAction(SCR_WorkshopItem wrapper)
	{
		#ifdef WORKSHOP_DEBUG
		_print("NEW");
		#endif
		
		m_Wrapper = wrapper;
		m_bAttached = true;
		
		// Cache basic addon data
		m_sAddonName = wrapper.GetName();
		m_sAddonId = wrapper.GetId();
	}
	
	
	//-----------------------------------------------------------------------------------------------
	void ~SCR_WorkshopItemAction()
	{
		#ifdef WORKSHOP_DEBUG
		_print("DELETE");
		#endif
	}
	
	// ---- Public API ----
		
	//-----------------------------------------------------------------------------------------------
	// Public getters for internal state
	bool IsInactive() { return m_State == STATE_INACTIVE; }
	bool IsActive() { return m_State == STATE_ACTIVE; }
	bool IsCompleted() { return m_State == STATE_COMPLETED; }
	bool IsCanceled() { return m_State == STATE_CANCELED; }
	bool IsPaused() { return m_State == STATE_PAUSED; }
	bool IsFailed() { return m_State == STATE_FAILED; }
	
	//-----------------------------------------------------------------------------------------------
	//! When action is attached, it means that it wasn't completed or wasn't replaced by another action.
	//! It is detached when another action takes its place, when it is completed or canceled.
	//! If an action is detached, you can only read its state, but you can't do any operations like activate, restart, pause, etc.
	bool IsAttached() { return m_bAttached; }
	
	//-----------------------------------------------------------------------------------------------
	SCR_WorkshopItem GetWorkshopItem() { return m_Wrapper; }
	
	//-----------------------------------------------------------------------------------------------
	void SetUserData(Managed data) { m_UserData = data; }
	
	//-----------------------------------------------------------------------------------------------
	Managed GetUserData(Managed data) { return m_UserData; }
	
	//-----------------------------------------------------------------------------------------------
	//! Returns the cached addon name, even if WorkshopItem object is already deleted.
	string GetAddonName()
	{
		if (m_Wrapper)
			return m_Wrapper.GetName();
		else
			return m_sAddonName;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns the cached addon ID, even if WorkshopItem object is already deleted.
	string GetAddonId()
	{
		if (m_Wrapper)
			return m_Wrapper.GetId();
		else
			return m_sAddonId;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Public methods to interact with the action
	//! They return true if the operation was successful.
	//! If operation was not successful, false is returned, and action is in Failed state.
	//-----------------------------------------------------------------------------------------------
	
	//-----------------------------------------------------------------------------------------------
	//! Activates an inactive action, or resumes a paused action.
	bool Activate()
	{
		#ifdef WORKSHOP_DEBUG
		_print("Activate()");
		#endif
		
		if (!m_Wrapper || !m_bAttached)
			return false;
		
		if (m_State == STATE_INACTIVE)
		{
			#ifdef WORKSHOP_DEBUG
			_print("OnActivate()");
			#endif
			bool success = OnActivate();
			if (success)
			{
				m_State = STATE_ACTIVE;
				m_OnActivated.Invoke(this);
				InvokeOnChanged();
				m_Wrapper.Internal_OnChanged();
			}
			else
			{
				Fail();
			}
			return success;
		}
		if (m_State == STATE_PAUSED)
		{
			return this.Resume();
		}
		else if (m_State == STATE_ACTIVE)
		{
			return true;
		}
		
		return false;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Cancels an action. Valid in active, inactive, paused states.
	bool Cancel()
	{
		if (!m_Wrapper || !m_bAttached)
			return false;
		
		#ifdef WORKSHOP_DEBUG
		_print("Cancel()");
		#endif
		
		if (m_State == STATE_ACTIVE || m_State == STATE_INACTIVE || m_State == STATE_PAUSED)
		{
			#ifdef WORKSHOP_DEBUG
			_print("OnCancel()");
			#endif
			
			bool success = OnCancel();
			if (success)
			{
				m_State = STATE_CANCELED;
				m_OnCanceled.Invoke(this);
				InvokeOnChanged();
				m_Wrapper.Internal_OnChanged();
			}
			else
			{
				Fail();
			}
			return success;
		}
		else if (m_State == STATE_CANCELED)
		{
			return true;
		}
		
		return false;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Pauses an active action.
	bool Pause()
	{
		if (!m_Wrapper || !m_bAttached)
			return false;
		
		#ifdef WORKSHOP_DEBUG
		_print("Pause()");
		#endif
		
		if (m_State == STATE_ACTIVE)
		{
			#ifdef WORKSHOP_DEBUG
			_print("OnPause()");
			#endif
			
			bool success = OnPause();
			if (success)
			{
				m_State = STATE_PAUSED;
				m_OnPaused.Invoke(this);
				InvokeOnChanged();
				m_Wrapper.Internal_OnChanged();
			}
			else
			{
				Fail();
			}
			return success;
		}
		else if (m_State == STATE_PAUSED)
		{
			return true;
		}
		
		return false;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Resumes a paused action.
	bool Resume()
	{
		if (!m_Wrapper || !m_bAttached)
			return false;
		
		#ifdef WORKSHOP_DEBUG
		_print("Resume()");
		#endif		
		
		if (m_State == STATE_PAUSED)
		{
			#ifdef WORKSHOP_DEBUG
			_print("OnResume()");
			#endif
			
			bool success = OnResume();
			if (success)
			{
				m_State = STATE_ACTIVE;
				m_OnResumed.Invoke(this);
				InvokeOnChanged();
				m_Wrapper.Internal_OnChanged();
			}
			else
			{
				Fail();
			}
			return success;
		}
		else if (m_State == STATE_INACTIVE)
		{
			Activate();
		}
		else if (m_State == STATE_ACTIVE)
		{
			return true; // Resuming an active action doesn't mean an error
		}
		
		return false;
	}

	//-----------------------------------------------------------------------------------------------
	//! Starts action again Only valid in failed state.
	bool Reactivate()
	{
		if (!m_Wrapper || !m_bAttached)
			return false;
		
		#ifdef WORKSHOP_DEBUG
		_print("Reactivate()");
		#endif
		
		if (m_State == STATE_FAILED)
		{
			#ifdef WORKSHOP_DEBUG
			_print("OnReactivate()");
			#endif
			
			bool success = OnReactivate();
			if (success)
			{
				m_State = STATE_ACTIVE;
				m_OnActivated.Invoke(this);
				InvokeOnChanged();
				m_Wrapper.Internal_OnChanged();
			}
			return success;
		}
		else if (m_State == STATE_ACTIVE)
		{
			return true;
		}
		
		return false;
	}
	
	
	// ---- Protected / private ----
	
	
	//-----------------------------------------------------------------------------------------------
	//! Action must call this when it has failed
	protected void Fail(int reason = -1)
	{
		if (!m_Wrapper || !m_bAttached)
			return;
		
		#ifdef WORKSHOP_DEBUG
		_print("Fail()");
		#endif
		
		if (m_State != STATE_FAILED)
		{
			m_State = STATE_FAILED;
			OnFail(reason);
			m_OnFailed.Invoke(this, reason);
			InvokeOnChanged();
			m_Wrapper.Internal_OnChanged();
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Action must call this when it is completed
	protected void Complete()
	{
		if (!m_Wrapper || !m_bAttached)
			return;
		
		#ifdef WORKSHOP_DEBUG
		_print("Complete()");
		#endif
		
		if (m_State != STATE_COMPLETED)
		{
			m_State = STATE_COMPLETED;
			OnComplete();
			m_OnCompleted.Invoke(this);
			InvokeOnChanged();
			m_Wrapper.Internal_OnChanged();
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Call this in inherited classes to invoke the OnChanged event handler.
	protected void InvokeOnChanged()
	{
		m_OnChanged.Invoke(this);
	}
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Just a formatting note: as far as I know we don't want inline return
		
	//------------------------------------------------------------------------------------------------
	//! These methods must be overridden in derived classes to implement the functionality.
	//! If the specific action can't be paused, canceled, or resumed, the methods must return false.
	protected bool OnActivate() { return false; }
	protected bool OnCancel() { return false; }
	protected bool OnPause() { return false; }
	protected bool OnResume() { return false; }
	protected bool OnReactivate() { return false; }
	protected void OnFail(int reason = -1);
	protected void OnComplete();
	
//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	//! Called on each frame by the SCR_WorkshopItem which owns it.
	//! Here we can perform polling to detect state transitions
	void Internal_Update(float timeSlice);
	
	//------------------------------------------------------------------------------------------------
	void Internal_Detach()
	{
		m_bAttached = false;
	}
	
	//------------------------------------------------------------------------------------------------
	void _print(string str, LogLevel logLevel = LogLevel.DEBUG)
	{
		Print(string.Format("[SCR_AddonManager] %1 %2", this, str), logLevel);
	}
};

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// auto

//! Composite action which includes multiple subactions
class SCR_WorkshopItemActionComposite : SCR_WorkshopItemAction
{
	ref array<ref SCR_WorkshopItemAction> m_aActions = new array<ref SCR_WorkshopItemAction>;
	
	//-----------------------------------------------------------------------------------------------
	//! Returns a copy of the internal array of subactions.
	array<ref SCR_WorkshopItemAction> GetActions()
	{
		array<ref SCR_WorkshopItemAction> actions = new array<ref SCR_WorkshopItemAction>;
		
		foreach (auto a : m_aActions)
			actions.Insert(a);
		
		return actions;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void RegisterAction(SCR_WorkshopItemAction action)
	{
		m_aActions.Insert(action);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void UnregisterAction(SCR_WorkshopItemAction action)
	{
		m_aActions.RemoveItem(action);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void UnregisterActions(array<SCR_WorkshopItemAction> actions)
	{
		foreach (auto i : actions)
			UnregisterAction(i);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected bool AllCompleted()
	{
		int n = 0;
		
		foreach (auto a : m_aActions)
			if (a.IsCompleted())
				n++;
		
		return n == m_aActions.Count();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected bool AnyFailed()
	{
		foreach (auto a : m_aActions)
			if (a.IsFailed())
				return true;
		
		return false;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void CancelAll()
	{
		foreach (auto a : m_aActions)
			a.Cancel();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void PauseAll()
	{
		foreach (auto a : m_aActions)
			a.Pause();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void ResumeAll()
	{
		foreach (auto a : m_aActions)
			a.Resume();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void ActivateAll()
	{
		foreach (auto a : m_aActions)
			a.Activate();
	}
	
	//---- REFACTOR NOTE END ----
};