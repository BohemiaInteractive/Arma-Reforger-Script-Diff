//------------------------------------------------------------------------------------------------
class SCR_DamageManagerData
{
	int m_iNextFreeIndex = -1;
	protected ref ScriptInvoker m_OnDamage;
	protected ref ScriptInvoker m_OnDamageOverTimeAdded;
	protected ref ScriptInvoker m_OnDamageOverTimeRemoved;
	protected ref ScriptInvoker m_OnDamageStateChanged;
	protected ref ScriptInvoker m_OnDamageEffectAdded;
	protected ref ScriptInvoker m_OnDamageEffectRemoved;

	//------------------------------------------------------------------------------------------------
	void Reset()
	{
		delete m_OnDamage;
		delete m_OnDamageStateChanged;
		delete m_OnDamageEffectAdded;
		delete m_OnDamageEffectRemoved;
		delete m_OnDamageOverTimeAdded;
		delete m_OnDamageOverTimeRemoved;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDamage(bool createNew = true)
	{
		if (!m_OnDamage && createNew)
			m_OnDamage = new ScriptInvoker();
		return m_OnDamage;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDamageOverTimeAdded(bool createNew = true)
	{
		if (!m_OnDamageOverTimeAdded && createNew)
			m_OnDamageOverTimeAdded = new ScriptInvoker();
		return m_OnDamageOverTimeAdded;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDamageOverTimeRemoved(bool createNew = true)
	{
		if (!m_OnDamageOverTimeRemoved && createNew)
			m_OnDamageOverTimeRemoved = new ScriptInvoker();
		return m_OnDamageOverTimeRemoved;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDamageStateChanged(bool createNew = true)
	{
		if (!m_OnDamageStateChanged && createNew)
			m_OnDamageStateChanged = new ScriptInvoker();
		return m_OnDamageStateChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDamageEffectAdded(bool createNew = true)
	{
		if (!m_OnDamageEffectAdded && createNew)
			m_OnDamageEffectAdded = new ScriptInvoker();
		return m_OnDamageEffectAdded;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDamageEffectRemoved(bool createNew = true)
	{
		if (!m_OnDamageEffectRemoved && createNew)
			m_OnDamageEffectRemoved = new ScriptInvoker();
		return m_OnDamageEffectRemoved;
	}
};