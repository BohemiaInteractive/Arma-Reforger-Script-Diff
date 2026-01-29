[ComponentEditorProps(category: "GameScripted/Editor", description: "Camera for in-game editor. Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_CameraFiltersEditorComponentClass: SCR_BaseEditorComponentClass
{
	[Attribute()]
	private ref array<ref SCR_CameraFilterEditor> m_Filters;
	
	int GetFilters(notnull array<SCR_CameraFilterEditor> outFilters)
	{
		outFilters.Clear();
		foreach (SCR_CameraFilterEditor filter: m_Filters)
		{
			outFilters.Insert(filter);
		}
		return outFilters.Count();
	}
	int GetFiltersCount()
	{
		return m_Filters.Count();
	}
	SCR_CameraFilterEditor GetFilter(int index)
	{
		return m_Filters[index];
	}
};

/** @ingroup Editor_Components
*/
class SCR_CameraFiltersEditorComponent : SCR_BaseEditorComponent
{
	private int m_iCurrentFilter;
	private int m_iDefaultFilter;
	
	int GetFilters(notnull array<SCR_CameraFilterEditor> outFilters)
	{
		SCR_CameraFiltersEditorComponentClass prefabData = SCR_CameraFiltersEditorComponentClass.Cast(GetEditorComponentData());
		if (!prefabData) return 0;
		
		return prefabData.GetFilters(outFilters);
	}
	void RefreshCurrentFilter(SCR_ManualCamera manualCamera)
	{
		SetCurrentFilter(m_iCurrentFilter);
	}
	void SetCurrentFilter(int index)
	{
		SCR_CameraFiltersEditorComponentClass prefabData = SCR_CameraFiltersEditorComponentClass.Cast(GetEditorComponentData());
		if (!prefabData || index < 0 || index >= prefabData.GetFiltersCount()) return;
		
		SCR_CameraFilterEditor filter = prefabData.GetFilter(m_iCurrentFilter);
		if (filter) filter.Revert();
		
		m_iCurrentFilter = index;
		
		filter = prefabData.GetFilter(m_iCurrentFilter);
		if (filter) filter.Apply();
	}
	int GetCurrentFilter()
	{
		return m_iCurrentFilter;
	}
	override void ResetEditorComponent()
	{
		SetCurrentFilter(m_iDefaultFilter);
	}
	override void EOnEditorPostActivate()
	{
		SCR_CameraEditorComponent cameraManager = SCR_CameraEditorComponent.Cast(SCR_CameraEditorComponent.GetInstance(SCR_CameraEditorComponent, true));
		if (!cameraManager) return;
		
		if (cameraManager.GetCamera())
			RefreshCurrentFilter(null);
		else
			cameraManager.GetOnCameraCreate().Insert(RefreshCurrentFilter);
	}
	override void EOnEditorDeactivate()
	{
		SCR_CameraEditorComponent cameraManager = SCR_CameraEditorComponent.Cast(SCR_CameraEditorComponent.GetInstance(SCR_CameraEditorComponent));
		if (cameraManager) cameraManager.GetOnCameraCreate().Remove(RefreshCurrentFilter);
	}
	override void EOnEditorInit()
	{
		m_iDefaultFilter = m_iCurrentFilter;
	}
};

[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_CameraFilterEditor
{
	[Attribute()]
	private LocalizedString m_sDisplayName;
	
	[Attribute()]
	private ref array<ref SCR_CameraFilterEffectEditor> m_Effects;
	
	string GetDisplayName()
	{
		return m_sDisplayName;
	}
	
	void Apply()
	{
		SCR_PostProcessCameraComponent postProcessComponent = GetPostProcessComponent();
		if (!postProcessComponent) return;
		
		foreach (SCR_CameraFilterEffectEditor effect: m_Effects)
		{
			effect.Apply(postProcessComponent);
		}
	}
	void Revert()
	{
		SCR_PostProcessCameraComponent postProcessComponent = GetPostProcessComponent();
		if (!postProcessComponent) return;
		
		foreach (SCR_CameraFilterEffectEditor effect: m_Effects)
		{
			effect.Revert(postProcessComponent);
		}
	}
	protected SCR_PostProcessCameraComponent GetPostProcessComponent()
	{
		SCR_ManualCamera camera = SCR_CameraEditorComponent.GetCameraInstance();
		if (!camera) return null;
		
		return SCR_PostProcessCameraComponent.Cast(camera.FindComponent(SCR_PostProcessCameraComponent));
	}
};
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(PostProcessEffectType, "m_Type")]
class SCR_CameraFilterEffectEditor
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(PostProcessEffectType))]
	private PostProcessEffectType m_Type;
	
	[Attribute(params: "emat")]
	private ResourceName m_MaterialPath;
	
	void Apply(SCR_PostProcessCameraComponent postProcessComponent)
	{
		SCR_CameraPostProcessEffect effect = postProcessComponent.FindEffect(m_Type);
		if (effect)
			effect.SetMaterial(m_MaterialPath);
		else
			Print(string.Format("Post-process effect '%1' not found on editor camera!", Type().EnumToString(PostProcessEffectType, m_Type)), LogLevel.ERROR);
	}
	void Revert(SCR_PostProcessCameraComponent postProcessComponent)
	{
		SCR_CameraPostProcessEffect effect = postProcessComponent.FindEffect(m_Type);
		if (effect) effect.SetMaterial();
	}
};