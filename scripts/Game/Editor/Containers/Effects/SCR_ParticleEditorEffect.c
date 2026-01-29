[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_Particle", true, "PFX: \"%1\"")]
/** @ingroup Editor_Effects
*/
/*!
Particle UI effect.
*/
class SCR_ParticleEditorEffect: SCR_BaseEditorEffect
{
	[Attribute(params: "ptc", desc: "Particle effect file")]
	private ResourceName m_Particle;
	
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EEditorEffectTarget))]
	private EEditorEffectTarget m_Target;
	
	override bool EOnActivate(SCR_BaseEditorComponent editorComponent, vector position = vector.Zero, set<SCR_EditableEntityComponent> entities = null)
	{
		switch (m_Target)
		{
			case EEditorEffectTarget.CURSOR:
			{
				Print("ToDo: CURSOR particle effect");
				break;
			}
			case EEditorEffectTarget.ROOT_ENTITY:
			{
				if (!entities || entities.Count() == 0) return false;
				vector pos;
				if (entities[0].GetPos(pos))
				{
					ParticleEffectEntitySpawnParams spawnParams = new ParticleEffectEntitySpawnParams();
					spawnParams.Transform[3] = pos;
					spawnParams.UseFrameEvent = true;
					ParticleEffectEntity.SpawnParticleEffect(m_Particle, spawnParams);
				}
				break;
			}
			case EEditorEffectTarget.ALL_ENTITIES:
			{
				if (!entities || entities.Count() == 0) return false;
				vector pos;
				foreach (SCR_EditableEntityComponent entity: entities)
				{
					if (entity.GetPos(pos))
					{
						ParticleEffectEntitySpawnParams spawnParams = new ParticleEffectEntitySpawnParams();
						spawnParams.Transform[3] = pos;
						spawnParams.UseFrameEvent = true;
						ParticleEffectEntity.SpawnParticleEffect(m_Particle, spawnParams);
					}
				}
				break;
			}
		}
		return true;
	}
};

enum EEditorEffectTarget
{
	ROOT_ENTITY,
	ALL_ENTITIES,
	CURSOR
};