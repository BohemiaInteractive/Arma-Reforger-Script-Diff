[EntityEditorProps(category: "GameScripted/Triggers", description: "")]
class SCR_BaseTriggerEntityClass: ScriptedGameTriggerEntityClass
{
}

//! Basic scripted trigger which offers external invoekrs for major events like activation or deactivation.
class SCR_BaseTriggerEntity : ScriptedGameTriggerEntity
{
	protected ref ScriptInvoker m_OnActivate = new ScriptInvoker();		// TODO: ScriptInvokerEntity
	protected ref ScriptInvoker m_OnDeactivate = new ScriptInvoker();	// TODO: ScriptInvokerVoid
	
	protected bool IsAlive(IEntity entity)
	{
		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.GetDamageManager(entity);
		if (damageManager)
			return damageManager.GetState() != EDamageState.DESTROYED;
		else
			return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Invoker called on server when the trigger is activated
	ScriptInvoker GetOnActivate()
	{
		return m_OnActivate;
	}
	//------------------------------------------------------------------------------------------------
	//! \return Invoker called on server when the trigger is deactivated
	ScriptInvoker GetOnDeactivate()
	{
		return m_OnDeactivate;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected event void OnActivate(IEntity ent)
	{
		m_OnActivate.Invoke(ent);
	}

	//------------------------------------------------------------------------------------------------
	override protected event void OnDeactivate(IEntity ent)
	{
		m_OnDeactivate.Invoke();
	}
}
