[BaseContainerProps(insertable: false)]
/** @ingroup Editor_Effects
*/
/*!
Base class for audio-visual effects.
*/
class SCR_BaseEditorEffect
{
	/*!
	Activate given editor effects.
	\param effects List of effects
	\param editorComponent Editor component controlling the effect
	\param entities Relevant editable entities
	*/
	static void Activate(array<ref SCR_BaseEditorEffect> effects, SCR_BaseEditorComponent editorComponent, vector position = vector.Zero, set<SCR_EditableEntityComponent> entities = null)
	{
		if (!effects) return;
		
		SCR_EditorBaseEntity owner = SCR_EditorBaseEntity.Cast(editorComponent.GetOwner());
		if (!owner) return;
		
		SCR_EditorManagerEntity manager = owner.GetManager();
		if (!manager) return;
		
		//--- When attached to editor mode, ignore when the editor is opening or closing (ToDo: Parametrize?)
		if (owner != manager && manager.IsInTransition()) return;
		
		foreach (SCR_BaseEditorEffect effect: effects)
		{
			if (effect.EOnActivate(editorComponent, position, entities))
			{
				editorComponent.OnEffectBase(effect);
				editorComponent.GetOnEffect().Invoke(effect);
			}
		}
	}
	protected bool EOnActivate(SCR_BaseEditorComponent editorComponent, vector position = vector.Zero, set<SCR_EditableEntityComponent> entities = null);
};