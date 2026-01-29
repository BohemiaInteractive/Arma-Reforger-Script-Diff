[EntityEditorProps(category: "GameScripted/DeployableItems", description: "")]
class SCR_DeployableInventoryItemReplacementComponentClass : ScriptComponentClass
{
}

void OnCompositionDestroyedDelegate(IEntity instigator);
typedef func OnCompositionDestroyedDelegate;
typedef ScriptInvokerBase<OnCompositionDestroyedDelegate> OnCompositionDestroyed_Invoker;

//! Holds Position of where the deployable item will be attached to.
class SCR_DeployableInventoryItemReplacementComponent : ScriptComponent
{
	[Attribute()]
	protected ref PointInfo m_vItemPosition;
	
	protected ref OnCompositionDestroyed_Invoker m_OnCompositionDestroyed;
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] mat
	void GetItemTransform(out vector mat[4])
	{
		if (m_vItemPosition)
			m_vItemPosition.GetTransform(mat);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	OnCompositionDestroyed_Invoker GetOnCompositionDestroyed()
	{
		if (!m_OnCompositionDestroyed)
			m_OnCompositionDestroyed = new OnCompositionDestroyed_Invoker();
		
		return m_OnCompositionDestroyed;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDamageStateChanged()
	{
		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.GetDamageManager(GetOwner());
		IEntity instigator;
		if (damageManager)
			instigator = damageManager.GetInstigator().GetInstigatorEntity();
		
		if (m_OnCompositionDestroyed && damageManager.IsDestroyed())
			m_OnCompositionDestroyed.Invoke(instigator);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.GetDamageManager(owner);
		damageManager.GetOnDamageStateChanged().Insert(OnDamageStateChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}
}
