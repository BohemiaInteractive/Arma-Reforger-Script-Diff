[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sSound", "SFX: \"%1\"")]
/** @ingroup Editor_Effects
*/
/*!
Sound UI effect.
*/
class SCR_SoundEditorEffect: SCR_BaseEditorEffect
{
	[Attribute(desc: "Sound event from *acp file.")]
	private string m_sSound;
	
	[Attribute(desc: "If sound has priority it will terminate any other UI sfx that are played.")]
	protected bool m_bSoundHasPriority;

	override bool EOnActivate(SCR_BaseEditorComponent editorComponent, vector position = vector.Zero, set<SCR_EditableEntityComponent> entities = null)
	{
		SCR_UISoundEntity.SoundEvent(m_sSound, m_bSoundHasPriority);
		return true;
	}
};