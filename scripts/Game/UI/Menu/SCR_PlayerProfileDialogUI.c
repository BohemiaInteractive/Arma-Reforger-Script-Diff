//------------------------------------------------------------------------------------------------
class SCR_PlayerProfileDialogUI : SCR_LoginProcessDialogUI
{
	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		BohemiaAccountApi.Unlink(m_Callback);
		
		super.OnConfirm();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnSuccess(BackendCallback callback)
	{
		Close();
	}
}
