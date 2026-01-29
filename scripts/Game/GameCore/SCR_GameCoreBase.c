/** @defgroup GameCore Game Core
Cores attached to ArmaReforgerScripted
*/

/** @ingroup GameCore
*/

/*!
Game core base class.

A game core is a class attached to ArmaReforgerScripted, similar to how components are attached to entities.
It contains several pre-defined functions which can be overridden by child classes.

__Use a core instead of declaring static variables and functions in your classes!__

The core is created when the game starts and persists for its whole duration, even between worlds.
__Make sure to clean world-specific variables in OnGameEnd()!__

## How to Create a Core
### Script
Create a new class inherited from this base class in _GameCore_ folder.
Name it `SCR_MyNameCore`, where _MyName_ is a name of your choosing.
Make sure the class name and the file name are the same!

__Example:__ _SCR_MyNameCore.c_ contains:
~~~
class SCR_MyNameCore: SCR_GameCoreBase
{
};
~~~

### Config
1. Open Resource Browser
2. Navigate to _Configs/Core/_ folder. This is important, cores will not loaded from other folders!
3. Click on _Create_ and select _Config File_
4. Name it _MyNameCore_ - the same name as the script class, but without SCR_ prefix
5. Choose `SCR_MyNameCore` class from the list
6. Open _CoresManager.conf_ and drag your _MyNameCore.conf_ into _Cores_ attribute 

After this, the core class will be loaded automatically upon game start.

*/
[BaseContainerProps()]
class SCR_GameCoreBase
{
	[Attribute(defvalue: "1", desc: "When disabled, the core will not be initialized upon start of the game.")]
	private bool m_bEnabled;
	
	[Attribute(desc: "Core processing priority. Higher value = higher priority")]
	private int m_iPriority;
	
	//------------------------------------------------------------------------------------------------
	//! Executed when the game starts (whole game, not just a world)
	void OnAfterInit();
	
	//------------------------------------------------------------------------------------------------
	//! Executed after world entities are initialized
	void OnGameStart();
	
	//------------------------------------------------------------------------------------------------
	//! Executed once loading of all entities of the world have been finished. (still within the loading)
	void OnWorldPostProcess(World world);
	
	//------------------------------------------------------------------------------------------------
	//! Executed every frame
	//! \param timeSlice Time passed since previous frame
	void OnUpdate(float timeSlice);
	
	//------------------------------------------------------------------------------------------------
	//! Executed when the world ends
	void OnGameEnd();
	
	//------------------------------------------------------------------------------------------------
	//! Get core priority. Cores with higher number will be executed first.
	//! \return priority
	int GetPriority()
	{
		return m_iPriority;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if the core is enabled.
	//! \return True when enabled
	bool IsEnabled()
	{
		return m_bEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if the core can be created.
	//! Override to setup custom condition
	//! \return true
	bool CanCreate()
	{
		return true;
	}
	
	//---
	//! Get instance of a core class.
	//! Example:
	//! \code
	//! SCR_MyNameCore myCore = SCR_MyNameCore.GetInstance(SCR_MyNameCore);
	//! \code
	//! \param type Type of the core class
	//! \return Core instance
	static SCR_GameCoreBase GetInstance(typename type)
	{
		return SCR_GameCoresManager.GetCore(type);
	}
}
