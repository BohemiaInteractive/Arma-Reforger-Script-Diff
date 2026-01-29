/*!
Handles lodaing of multiple addon patch sizes
*/
//------------------------------------------------------------------------------------------------
void ScriptInvoker_AddonPatchSizeLoad(SCR_AddonPatchSizeLoader loader, bool allLoaded);
typedef func ScriptInvoker_AddonPatchSizeLoad;

//------------------------------------------------------------------------------------------------
class SCR_AddonPatchSizeLoader
{
	protected ref array<SCR_WorkshopItem> m_aItems = {};
	protected ref array<SCR_WorkshopItem> m_aItemsFailed = {};
	protected int m_iLoadedPatches;
	
	protected ref ScriptInvokerBase<ScriptInvoker_AddonPatchSizeLoad> m_OnAllPatchSizeLoaded;
	
	//------------------------------------------------------------------------------------------------
	// Public
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void InsertItem(SCR_WorkshopItem item)
	{
		m_aItems.Insert(item);
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveItem(SCR_WorkshopItem item)
	{
		m_aItems.RemoveItem(item);
	}
	
	//------------------------------------------------------------------------------------------------
	void ClearItems()
	{
		m_aItems.Clear();
	}
	
	protected ref array<ref BackendCallback> m_aCallbacks = {};
	
	//------------------------------------------------------------------------------------------------
	//! Call compute patch size and setup response reaction
	void LoadPatchSizes()
	{
		foreach (SCR_WorkshopItem item : m_aItems)
		{
			Revision revision = item.GetCurrentLocalRevision();
			Revision target = item.GetItemTargetRevision();
			
			// Is matching
			if (Revision.AreEqual(revision, target))
			{
				m_iLoadedPatches++;
				continue;
			}
			
			// Compute patch size
			SCR_BackendCallbackWorkshopItem callback = new SCR_BackendCallbackWorkshopItem(item);
			callback.SetOnSuccess(OnPatchSizeLoadResponse);
			callback.SetOnError(OnPatchSizeLoadError);
			m_aCallbacks.Insert(callback);
			
			target.ComputePatchSize(callback);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Protected
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected void OnPatchSizeLoadResponse(SCR_BackendCallbackWorkshopItem callback)
	{
		SCR_WorkshopItem item = callback.GetItem();
		
		float size;
		if (item.GetItemTargetRevision().GetPatchSize(size))
		{
			// Setup size
			item.SetTargetRevisionPatchSize(size);
			m_iLoadedPatches++;
		}
		else
		{
			// Patch size not loaded
			m_aItemsFailed.Insert(item);
		}
		
		// All loaded 
		if (m_iLoadedPatches + m_aItemsFailed.Count() == m_aItems.Count())
		{
			bool allLoaded = m_aItemsFailed.IsEmpty();
			
			if (m_OnAllPatchSizeLoaded)
				m_OnAllPatchSizeLoaded.Invoke(this, allLoaded);
		}
		
		m_aCallbacks.RemoveItem(callback);	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPatchSizeLoadError(SCR_BackendCallbackWorkshopItem callback)
	{
		SCR_WorkshopItem item = callback.GetItem();
		
		// Fail or timeout
		m_aItemsFailed.Insert(item);
		
		// All loaded 
		if (m_iLoadedPatches + m_aItemsFailed.Count() == m_aItems.Count())
		{
			bool allLoaded = m_aItemsFailed.IsEmpty();
			
			if (m_OnAllPatchSizeLoaded)
				m_OnAllPatchSizeLoaded.Invoke(this, allLoaded);
		}

		m_aCallbacks.RemoveItem(callback);	
	}
	
	//------------------------------------------------------------------------------------------------
	// Get set
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<ScriptInvoker_AddonPatchSizeLoad> GetOnAllPatchSizeLoaded()
	{
		if (!m_OnAllPatchSizeLoaded)
			m_OnAllPatchSizeLoaded = new ScriptInvokerBase<ScriptInvoker_AddonPatchSizeLoad>();
		
		return m_OnAllPatchSizeLoaded;
	}
	
	//------------------------------------------------------------------------------------------------
	// Construct
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_AddonPatchSizeLoader()
	{
		m_aItems.Clear();
		m_aItemsFailed.Clear();
		
		if (m_OnAllPatchSizeLoaded)
			m_OnAllPatchSizeLoaded.Clear();
	}
}

//------------------------------------------------------------------------------------------------
class SCR_BackendCallbackWorkshopItem : BackendCallback
{
	protected ref SCR_WorkshopItem m_Item;
	
	//------------------------------------------------------------------------------------------------
	SCR_WorkshopItem GetItem()
	{
		return m_Item;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_BackendCallbackWorkshopItem(SCR_WorkshopItem item)
	{
		m_Item = item;
	}
}