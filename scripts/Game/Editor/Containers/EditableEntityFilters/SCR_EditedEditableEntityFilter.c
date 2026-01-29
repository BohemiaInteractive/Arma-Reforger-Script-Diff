[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
/** @ingroup Editor_Components_Entities
*/
/*!
Entities which are currently being transformed.
*/
class SCR_EditedEditableEntityFilter : SCR_BaseEditableEntityFilter
{
	protected void OnTransformationStart(set<SCR_EditableEntityComponent> editedEntities)
	{
		Replace(editedEntities);
	}
	protected void OnTransformationEnd(set<SCR_EditableEntityComponent> editedEntities)
	{
		Clear();
	}

	override void EOnEditorActivate()
	{
		SCR_TransformingEditorComponent transformingManager = SCR_TransformingEditorComponent.Cast(SCR_TransformingEditorComponent.GetInstance(SCR_TransformingEditorComponent, true));
		if (!transformingManager) return;
		
		transformingManager.GetOnTransformationStart().Insert(OnTransformationStart);
		transformingManager.GetOnTransformationConfirm().Insert(OnTransformationEnd);
		transformingManager.GetOnTransformationCancel().Insert(OnTransformationEnd);
	}
	override void EOnEditorDeactivate()
	{
		SCR_TransformingEditorComponent transformingManager = SCR_TransformingEditorComponent.Cast(SCR_TransformingEditorComponent.GetInstance(SCR_TransformingEditorComponent));
		if (!transformingManager) return;
		
		transformingManager.GetOnTransformationStart().Remove(OnTransformationStart);
		transformingManager.GetOnTransformationConfirm().Remove(OnTransformationEnd);
		transformingManager.GetOnTransformationCancel().Remove(OnTransformationEnd);
	}
};