[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "")]
class SCR_EditableEntityVisibilityChildComponentClass: SCR_EditableEntityBaseChildComponentClass
{
}

//! @ingroup Editable_Entities

//! Show editable entity only when its state is RENDERED.
class SCR_EditableEntityVisibilityChildComponent : SCR_EditableEntityBaseChildComponent
{
	//------------------------------------------------------------------------------------------------
	override void EOnStateChanged(EEditableEntityState states, EEditableEntityState changedState, bool toSet)
	{
		if (m_Owner.IsDeleted())
			return;
		
		if ((states & m_State) > 0)
			m_Owner.SetFlags(EntityFlags.VISIBLE, true);
		else
			m_Owner.ClearFlags(EntityFlags.VISIBLE, true);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		owner.ClearFlags(EntityFlags.VISIBLE, true);
		UpdateFromCurrentState();
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		//--- Cannot ClearFlags() here, child entities are not yet spawned
		SetEventMask(owner, EntityEvent.INIT);
		super.OnPostInit(owner);
	}
}
