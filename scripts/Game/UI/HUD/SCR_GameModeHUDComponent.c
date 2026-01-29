[ComponentEditorProps(category: "GameScripted/GameMode", description: "Base UI Component for deathmatch game mode")]
class SCR_GameModeHUDComponentClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
//! Deathmatch HUD component. Expects to be attached onto SCR_GameModeDeathmatch entity
class SCR_GameModeHUDComponent : ScriptComponent
{
	[Attribute("{5E5A12EF04688432}UI/layouts/HUD/Deathmatch/DeathmatchLayout.layout", UIWidgets.ResourceNamePicker, "layout", "")]
	protected ResourceName m_sLayout; 
	protected Widget m_RootWidget;
	protected bool m_bShow = true;
	
	
	//------------------------------------------------------------------------------------------------
	void Show(bool show)
	{
		m_bShow = show;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_GameModeHUDComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (m_sLayout != string.Empty)
		{
			ArmaReforgerScripted game = GetGame();
			if (game)
			{
				SCR_HUDManagerComponent manager = game.GetHUDManager();
				if (manager)
				{
					m_RootWidget = manager.CreateLayout(m_sLayout,EHudLayers.LOW);
				}
			}
		}
		
		if (m_RootWidget)
			m_RootWidget.SetVisible(m_bShow);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_GameModeHUDComponent()
	{
		if (m_RootWidget)
			m_RootWidget.RemoveFromHierarchy();
		
		m_RootWidget = null;
	}
};