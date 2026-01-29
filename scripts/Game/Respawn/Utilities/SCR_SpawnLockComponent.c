class SCR_SpawnLockComponentClass : ScriptComponentClass
{
}

//! This components allows to lock SCR_SpawnRequestComponent(s) from issuing any more requests
//! until the issued one has been processed by the authority and a response has been received.
//! The expected hierarchy is:
//! 	PlayerController
//! 		- ...
//! 		- SCR_SpawnLockComponent
//! 		- ...
class SCR_SpawnLockComponent : ScriptComponent
{
	//! Set of all instances that issued a lock on this component.
	//! Each instance should make sure to properly engage and disengage the lock.
	protected ref set<Managed> m_RequestLocks = new set<Managed>();
	
	//! Set of all instances that issued a lock on this component.
	//! Each instance should make sure to properly engage and disengage the lock.
	//! Disclaimer: This set is *never* relevant to a proxy.
	protected ref set<Managed> m_AuthorityLocks = new set<Managed>();

	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (!PlayerController.Cast(owner))
			Debug.Error(string.Format("%1 is not attached to a %2!", Type().ToString(), PlayerController));
	}

	//------------------------------------------------------------------------------------------------
	//! Locks the component with the provided instance.
	//! \param[in] source Lock instigator.
	//! \param[in] auth
	void Lock(Managed source, bool auth)
	{
		if (auth)
			m_AuthorityLocks.Insert(source);
		else
			m_RequestLocks.Insert(source);
		
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintLocks("SCR_SpawnLockComponent::Lock()", source, auth);
		#endif
	}

	//------------------------------------------------------------------------------------------------
	//! Unlocks the component with the provided instance.
	//! \param[in] source Unlock instigator.
	//! \param[in] auth
	void Unlock(Managed source, bool auth)
	{
		if (auth)
		{
			int index = m_AuthorityLocks.Find(source);
			if (index == -1)
				return;

			m_AuthorityLocks.Remove(index);
		}
		else
		{
			int index = m_RequestLocks.Find(source);
			if (index == -1)
				return;

			m_RequestLocks.Remove(index);
		}
		
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintLocks("SCR_SpawnLockComponent::Unlock()", source, auth);
		#endif
	}

	//------------------------------------------------------------------------------------------------
	//! Is this lock engaged?
	//! 	The lock is engaged if any instance has engaged it. (using Lock)
	//! 	The lock is lifted once all instances that have previously engaged it disengage it. (using Unlock)
	//! \param[in] auth
	//! \return
	bool IsLocked(bool auth)
	{
		if (auth)
			return !m_AuthorityLocks.IsEmpty();
		else
			return !m_RequestLocks.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Try to engage lock, return true on success; false otherwise.
	//! \param[in] source
	//! \param[in] auth
	//! \return
	bool TryLock(Managed source, bool auth)
	{
		if (IsLocked(auth))
			return false;
		
		Lock(source, auth);
		return true;
	}
	
	#ifdef _ENABLE_RESPAWN_LOGS

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] ctx
	//! \param[in] src
	//! \param[in] auth
	void PrintLocks(string ctx, Managed src, bool auth)
	{		
		Print(string.Format("%1 (reqCnt: %2 | authCnt: %3 | (src: %4, auth: %5)",
			ctx, m_RequestLocks.Count(), m_AuthorityLocks.Count(), src, auth), LogLevel.NORMAL);
	}	
	#endif
}
