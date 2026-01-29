[BaseContainerProps(), SCR_BaseEntitiesEditorUIEffectTitle()]
class SCR_OpacityEntitiesEditorUIEffect : SCR_BaseEntitiesEditorUIEffect
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, params: "0 1 0.01")]
	private float m_fAlpha;

	//------------------------------------------------------------------------------------------------
	override void ApplyOn(Widget w)
	{
		w.SetOpacity(m_fAlpha);
	}
}
