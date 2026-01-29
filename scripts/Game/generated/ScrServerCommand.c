/*
===========================================
Do not modify, this script is generated
===========================================
*/

class ScrServerCommand
{
	//! Command's name without the '#' prefix
	event string GetKeyword();
	//! Update PENDING request.
	event protected ref ScrServerCmdResult OnUpdate();
	//! Server-side rcon command execution
	event protected ref ScrServerCmdResult OnRCONExecution(array<string> argv);
	//! Required permission (ERCONPermissions) to run RCON execution
	event protected int RequiredRCONPermission();
	//! True if the command should be passed to the server. False for only client-side execution.
	event bool IsServerSide();
	//! The main chat execution that is executed on the server (if IsServerSide is true)
	event protected ref ScrServerCmdResult OnChatServerExecution(array<string> argv, int playerId);
	//! The first chat execution stage (always on the client side).
	event protected ref ScrServerCmdResult OnChatClientExecution(array<string> argv, int playerId);
	//! Required permission (EPlayerRole) to run chat execution
	event protected int RequiredChatPermission();
}
