//------------------------------------------------------------------------------------------------	
class SCR_ScriptPlatformRequestCallback : PrivilegeCallback
{
	ref ScriptInvoker m_OnResult = new ScriptInvoker; // (UserPrivilege privilege, UserPrivilegeResult result)
	
	
	//------------------------------------------------------------------------------------------------
	override void OnPrivilegeResult(UserPrivilege privilege, UserPrivilegeResult result)
	{
		m_OnResult.Invoke(privilege, result);
	}	
};

////////////////////////////////////////////////////////////
// Callbacks for specific workshop requests
////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------------
// WorkshopItem.AskDetails
class SCR_WorkshopItemCallback_AskDetails : BackendCallback
{
	WorkshopItem m_Item;
	
	//-----------------------------------------------------------------------------------------------
	void SCR_WorkshopItemCallback_AskDetails(WorkshopItem item)
	{
		m_Item = item;
	}
};

//-----------------------------------------------------------------------------------------------
// ImageScale.Download
class SCR_WorkshopItemCallback_DownloadImage : BackendCallback
{
	ImageScale m_Scale;
};