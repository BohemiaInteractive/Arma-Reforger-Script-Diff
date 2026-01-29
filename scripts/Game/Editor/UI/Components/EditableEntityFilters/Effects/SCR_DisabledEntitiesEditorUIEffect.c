[BaseContainerProps(), SCR_BaseEntitiesEditorUIEffectTitle()]
class SCR_DisabledEntitiesEditorUIEffect : SCR_BaseEntitiesEditorUIEffect
{
	//------------------------------------------------------------------------------------------------
	override void ApplyOn(Widget w)
	{
		w.SetEnabled(false);
	}
}
