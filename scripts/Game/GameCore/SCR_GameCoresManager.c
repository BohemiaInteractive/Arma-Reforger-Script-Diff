/** @ingroup GameCore
*/

/*!
Manager of SCR_GameCoreBase classes, operated from class Game.
*/
[BaseContainerProps(configRoot: true)]
class SCR_GameCoresManager
{
	protected static const ResourceName CORE_CONFIG_PATH = "{A9ACCDC72AD70995}Configs/Core/Cores.conf";
	
	[Attribute()]
	protected ref array<ref SCR_GameCoreBase> m_aCores;
	
	protected ref array<SCR_GameCoreBase> m_CoresSorted = new array<SCR_GameCoreBase>;
	protected static ref map<typename, SCR_GameCoreBase> m_CoresMap = new map<typename, SCR_GameCoreBase>;
	
	void OnAfterInit()
	{
		foreach (SCR_GameCoreBase core: m_CoresSorted)
		{
			if (core)
				core.OnAfterInit();
		}
	}
	void OnGameStart()
	{
		foreach (SCR_GameCoreBase core: m_CoresSorted)
		{
			if (core)
				core.OnGameStart();
		}
	}
	void OnWorldPostProcess(World world)
	{
		foreach (SCR_GameCoreBase core: m_CoresSorted)
		{
			if (core)
				core.OnWorldPostProcess(world);
		}
	}
	void OnUpdate(float timeSlice)
	{
		foreach (SCR_GameCoreBase core: m_CoresSorted)
		{
			if (core)
				core.OnUpdate(timeSlice);
		}
	}
	void OnGameEnd()
	{
		foreach (SCR_GameCoreBase core: m_CoresSorted)
		{
			if (core)
				core.OnGameEnd();
		}
	}
	
	/*!
	Get game core of given type.
	\param type Class inherited from SCR_GameCoreBase
	\return Core class (if it exists, otherwise null is returned)
	*/
	static SCR_GameCoreBase GetCore(typename type)
	{
		return m_CoresMap.Get(type);
	}
	/*!
	Create cores manager.
	\return Manager class
	*/
	static SCR_GameCoresManager CreateCoresManager()
	{
		Resource container = BaseContainerTools.LoadContainer(CORE_CONFIG_PATH);
		if (!container || !container.IsValid())
		{
			Print(string.Format("Core config '%1' not found!", CORE_CONFIG_PATH), LogLevel.ERROR);
			return null;
		}
		
		Managed instance = BaseContainerTools.CreateInstanceFromContainer(container.GetResource().ToBaseContainer());
		SCR_GameCoresManager manager = SCR_GameCoresManager.Cast(instance);
		if (!manager)
		{
			Print(string.Format("SCR_GameCoresManager class not found in '%1'!", CORE_CONFIG_PATH), LogLevel.ERROR);
			delete instance;
			return null;
		}
		
		return manager;
	}
	void SCR_GameCoresManager()
	{
		m_CoresMap = new map<typename, SCR_GameCoreBase>;
		foreach (SCR_GameCoreBase core: m_aCores)
		{
			if (!core.IsEnabled() || !core.CanCreate())
				continue;
			
			if (!m_CoresMap.Contains(core.Type()))
			{
				//--- Add by priority
				int priority = core.GetPriority();
				int index = 0;
				for (int c = 0, count = m_CoresSorted.Count(); c < count; c++)
				{
					index = c;
					if (priority > m_CoresSorted[c].GetPriority())
						break;
				}
				m_CoresSorted.InsertAt(core, index);
				m_CoresMap.Insert(core.Type(), core);
			}
			else
			{
				Print(string.Format("Duplicate definition of core '%1'!", core.Type()), LogLevel.ERROR);
			}
		}
		
		OnAfterInit();
	}
};