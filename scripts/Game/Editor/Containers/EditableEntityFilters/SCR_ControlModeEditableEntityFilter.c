[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
/** @ingroup Editor_Components_Entities
*/
/*!
*/
class SCR_ControlModeEditableEntityFilter : SCR_BaseEditableEntityFilter
{
	protected void OnControlModeChanged(AIGroup group, EGroupControlMode mode)
	{
		SCR_EditableEntityComponent editableGroup = SCR_EditableEntityComponent.GetEditableEntity(group);
		if (editableGroup)
		{
			Validate(editableGroup);
			for (int i, count = editableGroup.GetChildrenCount(); i < count; i++)
			{
				Validate(editableGroup.GetChild(i));
			}
		}
	}
	override bool CanAdd(SCR_EditableEntityComponent entity)
	{
		SCR_EditableEntityComponent group = SCR_EditableGroupComponent.Cast(entity);// entity.GetAIGroup(); //--- Only on groups, not characters or waypoints
		if (!group)
			return false;
		
		SCR_AIGroupInfoComponent groupInfoComponent = SCR_AIGroupInfoComponent.Cast(group.GetOwner().FindComponent(SCR_AIGroupInfoComponent));
		if (!groupInfoComponent)
			return false;
		
		return groupInfoComponent.GetGroupControlMode() == EGroupControlMode.AUTONOMOUS;
	}
	override void EOnEditorActivate()
	{
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if (aiWorld)
			aiWorld.GetOnControlModeChanged().Insert(OnControlModeChanged);
	}
	override void EOnEditorDeactivate()
	{
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if (aiWorld)
			aiWorld.GetOnControlModeChanged().Remove(OnControlModeChanged);
	}
};