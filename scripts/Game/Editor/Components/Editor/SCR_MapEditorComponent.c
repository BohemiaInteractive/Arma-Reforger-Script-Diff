[ComponentEditorProps(category: "GameScripted/Editor", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_MapEditorComponentClass: SCR_BaseEditorComponentClass
{
};

/** @ingroup Editor_Components
Manager of editable entities which players control (i.e., their avatars).
*/
class SCR_MapEditorComponent : SCR_BaseEditorComponent
{	
	[Attribute(SCR_SoundEvent.SOUND_HUD_MAP_OPEN, UIWidgets.EditBox)]
	protected string m_sOpeningEditorMapSfx;
	
	[Attribute(SCR_SoundEvent.SOUND_HUD_MAP_CLOSE, UIWidgets.EditBox)]
	protected string m_sClosingEditorMapSfx;
	
	protected ResourceName m_sMapConfigEditorPrefab;
	protected SCR_MapEntity m_MapEntity;
	protected SCR_EditorManagerEntity m_EditorManager;
	protected MapItem m_CameraIcon;
	protected bool m_bWasMapOpened;
	protected bool m_bIsInit;
	
	protected SCR_MapEditorUIComponent m_MapHandler;
	
	protected ref Color m_CameraIconColor = Color.White;
	
	bool GetMapAvailable()
	{
		return m_bIsInit;
	}
	
	void SetMapHandler(SCR_MapEditorUIComponent mapHandler)
	{
		m_MapHandler = mapHandler;
		ShowMap(m_bWasMapOpened);
	}
	
	ResourceName GetMapConfigPrefab()
	{
		return m_sMapConfigEditorPrefab;
	}
	
	void ToggleMap()
	{
		ShowMap(!IsEditorMapOpen());
	}
	
	void ShowMap(bool show)
	{
		if (!m_MapHandler)
		{
			return;
		}
		
		//--- In limited editor, map can be opened only if map gadget is in player's inventory
		if (show && GetManager().IsLimited())
		{
			IEntity playerEntity = SCR_PlayerController.GetLocalControlledEntity();
			if (!playerEntity)
				return; //--- Limited editor works only if player has an entity, so this should never happen.
			
			SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.Cast(playerEntity.FindComponent(SCR_GadgetManagerComponent));
			if (gadgetManager && !gadgetManager.GetGadgetByType(EGadgetType.MAP))
				return;
		}
		
		if (show)
			SCR_UISoundEntity.SoundEvent(m_sOpeningEditorMapSfx, true);
		else
			SCR_UISoundEntity.SoundEvent(m_sClosingEditorMapSfx, true);
		
		m_MapHandler.ToggleMap(show, m_sMapConfigEditorPrefab);
		m_bWasMapOpened = show;
	}
	
	void OnPlacingPreviewCreate()
	{
		if (IsEditorMapOpen())
		{
			ShowMap(false);
		}
	}
	
	protected bool IsEditorMapOpen()
	{
		if (m_MapHandler)
		{
			return m_MapHandler.IsEditorMapOpen();
		}
		return false;
	}
	
	protected void OnLimitedChange(bool isLimited)
	{
		//--- Close the map when the editor became limited
		if (isLimited && IsEditorMapOpen())
		{
			ShowMap(false);
		}
	}
	protected SCR_MapEditorUIComponent FindMapHandler()
	{
		SCR_MenuEditorComponent menuComponent = SCR_MenuEditorComponent.Cast(SCR_MenuEditorComponent.GetInstance(SCR_MenuEditorComponent));
		if (!menuComponent || !menuComponent.GetMenu())
			return null;
		
		SCR_MapEditorUIComponent mapEditorComponent = SCR_MapEditorUIComponent.Cast(menuComponent.GetMenu().GetRootComponent().FindComponent(SCR_MapEditorUIComponent, true));
		return mapEditorComponent;
	}
	
	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;
		super.OnPostInit(owner);
		
		m_MapEntity = SCR_MapEntity.GetMapInstance();
		if (!m_MapEntity)
			return;
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		SCR_MapConfigComponent configComp = SCR_MapConfigComponent.Cast(gameMode.FindComponent(SCR_MapConfigComponent));
		if (configComp)
			m_sMapConfigEditorPrefab = configComp.GetEditorMapConfig();
		
		m_EditorManager = SCR_EditorManagerEntity.Cast(owner);
		if (!m_EditorManager)
			return;
		
		m_EditorManager.GetOnLimitedChange().Insert(OnLimitedChange);
		
		SCR_PreviewEntityEditorComponent previewEntityComponent = SCR_PreviewEntityEditorComponent.Cast(SCR_PreviewEntityEditorComponent.GetInstance(SCR_PreviewEntityEditorComponent));
		if (previewEntityComponent)
		{
			previewEntityComponent.GetOnPreviewCreate().Insert(OnPlacingPreviewCreate);
		}
		
		m_bIsInit = true;
	}
	
	override void EOnDeactivate(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		if (m_CameraIcon)
		{
			delete m_CameraIcon;
		}
	}
};