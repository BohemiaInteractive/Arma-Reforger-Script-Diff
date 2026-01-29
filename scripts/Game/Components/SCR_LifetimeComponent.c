[ComponentEditorProps(category: "GameScripted", description: "")]
class SCR_LifetimeComponentClass : ScriptComponentClass
{
}

//! Delete entity after given duration.
class SCR_LifetimeComponent : ScriptComponent
{
	[Attribute(defvalue: "1")]
	protected float m_fLifeTime;
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (m_fLifeTime < 0)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(owner);
			return;
		}
		m_fLifeTime -= timeSlice;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.FRAME);
	}
}
