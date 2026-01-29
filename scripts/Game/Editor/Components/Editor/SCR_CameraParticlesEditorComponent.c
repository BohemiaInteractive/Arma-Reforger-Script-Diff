[ComponentEditorProps(category: "GameScripted/Editor", description: "Camera for in-game editor. Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_CameraParticlesEditorComponentClass: SCR_BaseEditorComponentClass
{
	[Attribute()]
	private ref array<ref SCR_CameraParticleEditor> m_Effects;
	
	int GetEffects(notnull array<SCR_CameraParticleEditor> outEffects)
	{
		outEffects.Clear();
		foreach (SCR_CameraParticleEditor effect: m_Effects)
		{
			outEffects.Insert(effect);
		}
		return outEffects.Count();
	}
	int GetEffectsCount()
	{
		return m_Effects.Count();
	}
	SCR_CameraParticleEditor GetEffect(int index)
	{
		return m_Effects[index];
	}
};

/** @ingroup Editor_Components
*/
class SCR_CameraParticlesEditorComponent : SCR_BaseEditorComponent
{
	private int m_iCurrentEffect;
	private SCR_CameraEditorComponent m_CameraManager;
	
	int GetEffects(notnull array<SCR_CameraParticleEditor> outEffects)
	{
		SCR_CameraParticlesEditorComponentClass prefabData = SCR_CameraParticlesEditorComponentClass.Cast(GetEditorComponentData());
		if (!prefabData) return 0;
		
		return prefabData.GetEffects(outEffects);
	}
	void RefreshCurrentEffect(SCR_ManualCamera manualCamera)
	{
		SetCurrentEffect(m_iCurrentEffect);
	}
	void SetCurrentEffect(int index)
	{
		SCR_CameraParticlesEditorComponentClass prefabData = SCR_CameraParticlesEditorComponentClass.Cast(GetEditorComponentData());
		if (!prefabData || index < 0 || index >= prefabData.GetEffectsCount()) return;
				
		SCR_CameraParticleEditor effect = prefabData.GetEffect(m_iCurrentEffect);
		if (effect) effect.Destroy();
		
		m_iCurrentEffect = index;
		
		effect = prefabData.GetEffect(m_iCurrentEffect);
		if (effect) effect.Create(m_CameraManager.GetCamera());
	}
	int GetCurrentEffect()
	{
		return m_iCurrentEffect;
	}
	override void EOnEditorPostActivate()
	{
		m_CameraManager = SCR_CameraEditorComponent.Cast(SCR_CameraEditorComponent.GetInstance(SCR_CameraEditorComponent, true));
		if (!m_CameraManager) return;
		
		if (m_CameraManager.GetCamera())
			RefreshCurrentEffect(null);
		else
			m_CameraManager.GetOnCameraCreate().Insert(RefreshCurrentEffect);
	}
	override void EOnEditorDeactivate()
	{
		if (m_CameraManager) m_CameraManager.GetOnCameraCreate().Remove(RefreshCurrentEffect);
		
		SCR_CameraParticlesEditorComponentClass prefabData = SCR_CameraParticlesEditorComponentClass.Cast(GetEditorComponentData());
		if (prefabData)
		{
			SCR_CameraParticleEditor effect = prefabData.GetEffect(m_iCurrentEffect);
			if (effect) effect.Destroy();
		}
	}
};

[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_CameraParticleEditor
{
	[Attribute()]
	private LocalizedString m_sDisplayName;
	
	[Attribute(params: "ptc", uiwidget: UIWidgets.ResourcePickerThumbnail)]
	private ResourceName m_Effect;
	
	private ParticleEffectEntity m_Emitter;
	
	string GetDisplayName()
	{
		return m_sDisplayName;
	}
	
	void Create(IEntity target)
	{
		if (m_Effect.IsEmpty()) return;
		
		ParticleEffectEntitySpawnParams spawnParams = new ParticleEffectEntitySpawnParams();
		spawnParams.Parent = target;
		m_Emitter = ParticleEffectEntity.SpawnParticleEffect(m_Effect, spawnParams);
	}
	void Destroy()
	{
		delete m_Emitter;
	}
};