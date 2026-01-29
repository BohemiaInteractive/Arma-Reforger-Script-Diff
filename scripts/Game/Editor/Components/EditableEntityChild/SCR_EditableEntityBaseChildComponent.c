[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "", insertable: false)]
class SCR_EditableEntityBaseChildComponentClass : ScriptComponentClass
{
}

//! @ingroup Editable_Entities

//! Base class for child editable entity component.
//!
//! When this component is attached to editable entity or one of its children in hierarchy (default hierarchy, not editor one),
//! it will receive events when state of the editable entity changes.
//! Used to manage visualization of entity avatar's, e.g., to show virtual entity and change its color when it's selected.
class SCR_EditableEntityBaseChildComponent : ScriptComponent
{
	[Attribute("0", UIWidgets.Flags, "", enums: ParamEnumArray.FromEnum(EEditableEntityState))]
	protected EEditableEntityState m_State;
	
	protected GenericEntity m_Owner;
	
	//------------------------------------------------------------------------------------------------
	//! Event called every time state of editable entity changes.
	//! \param[in] states currently active states
	//! \param[in] changedState changed state
	//! \param[in] toSet true if the state was enabled, false if it was disabled
	void EOnStateChanged(EEditableEntityState states, EEditableEntityState changedState, bool toSet);
	
	//------------------------------------------------------------------------------------------------
	//! Update based on current state of editable entity.
	void UpdateFromCurrentState()
	{
		//--- Call event on existing states
		IEntity parentTemp = m_Owner;
		SCR_EditableEntityComponent entity;
		while (parentTemp)
		{
			GenericEntity parentGeneric = GenericEntity.Cast(parentTemp);
			if (parentGeneric)
			{
				entity = SCR_EditableEntityComponent.Cast(parentGeneric.FindComponent(SCR_EditableEntityComponent));
				if (entity)
				{
					//--- Mark the entity as virtual
					if (!entity.HasEntityFlag(EEditableEntityFlag.VIRTUAL))
						entity.SetEntityFlag(EEditableEntityFlag.VIRTUAL, true);
					
					//--- Get all active states
					EEditableEntityState states = entity.GetEntityStates();
					EOnStateChanged(states, states, true);
					break;
				}
			}
			parentTemp = parentTemp.GetParent();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if effects controlled y this component can be applied when given state changes.
	//! \param[in] changedState Changed state
	//! \return True if the component can be applied
	bool CanApply(EEditableEntityState changedState)
	{
		return (m_State & changedState) && m_Owner;
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_EditableEntityBaseChildComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_Owner = GenericEntity.Cast(ent);
	}
}
