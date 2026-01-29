[EntityEditorProps(category: "GameScripted/UI/RadialMenu", description: "Common radial menu.")]
class SCR_RadialMenuGameModeComponentClass : ScriptComponentClass
{
}

class SCR_RadialMenuGameModeComponent : ScriptComponent
{
	[Attribute()]
	protected ref SCR_RadialMenu m_Menu;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] timeSlice
	void Update(float timeSlice)
	{		
		if (!m_Menu)
		{
			Print("[SCR_RadialMenuGameModeComponent] - Radial menu not assigned! Can't update!", LogLevel.WARNING);
			return;
		}

		m_Menu.Update(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		ConnectToRadialMenuSystem();
	}
	
	override void OnDelete(IEntity owner)
	{
		DisconnectFromRadialMenuSystem();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_RadialMenu GetMenu()
	{
		return m_Menu;
	}
	
	protected void ConnectToRadialMenuSystem()
	{
		World world = GetOwner().GetWorld();
		RadialMenuSystem updateSystem = RadialMenuSystem.Cast(world.FindSystem(RadialMenuSystem));
		if (!updateSystem)
			return;
		
		updateSystem.Register(this);
	}
	
	protected void DisconnectFromRadialMenuSystem()
	{
		World world = GetOwner().GetWorld();
		RadialMenuSystem updateSystem = RadialMenuSystem.Cast(world.FindSystem(RadialMenuSystem));
		if (!updateSystem)
			return;
		
		updateSystem.Unregister(this);
	}
}
