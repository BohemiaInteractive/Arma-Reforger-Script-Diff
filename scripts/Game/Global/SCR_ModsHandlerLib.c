//------------------------------------------------------------------------------------------------
//! SCR_ModHandlerLib Class
//!
//! Contains script functions for in game mod handling 
//! Used in Workshop and Server browser for mods state checking
//------------------------------------------------------------------------------------------------
class SCR_ModHandlerLib
{
	//------------------------------------------------------------------------------------------------
	//! Return summary size of dependencies list
	static float GetModListSize(array<Dependency> dependencies)
	{
		// Empty array check 
		if (dependencies.IsEmpty())
			return -1;
		
		// Size count 
		float size = 0;

		foreach (Dependency dependency : dependencies)
		{
			size += dependency.TotalFileSize();
		}
		
		// Result 
		return size;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return summary size of workshop item list
	static float GetModListSize(array<ref SCR_WorkshopItem> items)
	{
		// Empty array check 
		if (items.IsEmpty())
			return -1;
		
		// Size count 
		float size = 0;

		foreach (SCR_WorkshopItem item : items)
		{
			size += item.GetSizeBytes();
		}
		
		// Result 
		return size;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if all items are cached = if mod info can be read  
	//! Return true if is possible to get cached item of each mod
	//! Insert Dependency array with dependencies that needs to be loaded 
	static bool AllItemsCached(array<Dependency> aDependencies, out array<Dependency> aDepsToLoad)
	{
		for (int i = 0; i < aDependencies.Count(); i++)
		{
			WorkshopItem item = aDependencies[i].GetCachedItem();

			// Is cashed item missing?
			if(!item)
			{
				// Items to load batch
				aDepsToLoad.Insert(aDependencies[i]);
				continue;
			}
		}
		
		// Are there dependecies to load
		if (aDepsToLoad.Count() == 0)
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return true if all mods are up to date - server can be reached 
	//! TODO@wernerjak - tweak this for versioning
	static bool AllModsUpdated(array<Dependency> dependencies)
	{
		// No dependencies in list
		if (dependencies.Count() == 0)
			return true;
		
		// Fill items array to check 
		array<WorkshopItem> aRoomItems = new array<WorkshopItem>;
		
		foreach (Dependency dependency : dependencies)
		{
			WorkshopItem item = dependency.GetCachedItem();
			
			if (item)
				aRoomItems.Insert(item);
		}
		
		// Compare items
		foreach (WorkshopItem item : aRoomItems)
		{
			if (item.GetActiveRevision())
				continue;
			else 
				return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// TODO@wernejak - setup this debug when needed data are ready 
	static void SetupDebug()
	{
		// Debug menu 
		/*DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_ADDONS_INFO, "Enabled addons", "Addons");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_ADDONS_INFO, "", "Show enabled addons list", "Enabled addons", false);*/
		
		//SCR_AddonManager.GetAllAddons();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Compare current client version with required version
	//! Return true if those version match
	protected static bool ItemVersionMatch(SCR_WorkshopItem item)
	{
		// Check item 
		WorkshopItem wsItem = item.GetWorkshopItem();
		if (!wsItem)
			return false;
		
		// Dependency check 
		Dependency dependency = item.GetDependency();
		if (!dependency)
			return false;
			
		// Does active version match required?		
		return Revision.AreEqual(item.GetCurrentLocalRevision(), dependency.GetRevision());
	}
}