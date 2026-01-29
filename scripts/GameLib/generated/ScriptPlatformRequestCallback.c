/*
===========================================
Do not modify, this script is generated
===========================================
*/

class ScriptPlatformRequestCallback: PlatformRequestCallback
{
	event void OnPrivilegeResult(UserPrivilege privilege, UserPrivilegeResult result) { Print(string.Format("ScriptPlatformRequestCallback::OnPrivilegeResult %1, %2", privilege, result)); };
}
