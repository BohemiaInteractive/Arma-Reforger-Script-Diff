/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup System
\{
*/

/*! Module containing compiled scripts.
\code
string watch = "GetGame().InPlayMode()";
string code = string.Format("string WatchResult() { return string.ToString(%1); }", watch);
string logText;
int logLine;
ScriptModule mod = ScriptModule.CompileScript(GetGame().GetScriptModule(), code, logText, logLine);

if (mod)
{
	string output;
	mod.Call(null, "WatchResult", false, output);
	Print(output);
}
else
{
	Print(logText);
	Print(logLine);
}
\endcode
*/
class ScriptModule
{
	private void ScriptModule();

	/*!
	Dynamic call of function.
	when `inst == NULL`, it's global function call, otherwise it's method of class.
	If `async` is `true`, creates new thread (so it's legal to use sleep/wait).
	Otherwise main thread is used and call is blocking.
	Return value of called method is returned via `returnVal` (only when `async` is false!)
	Returns `true`, when success.
	*/
	proto bool Call(Class inst, string function, bool async, out void returnVal, void param1 = NULL, void param2 = NULL, void param3 = NULL, void param4 = NULL, void param5 = NULL, void param6 = NULL, void param7 = NULL, void param8 = NULL, void param9 = NULL);
	/*!
	Load script and create ScriptModule for it.
	Available in developer builds (workbench or diag) and in headless server.
	\param parentModule Module
	\param scriptFile Script file path
	\returns Loaded scripted module
	*/
	static proto ref ScriptModule LoadScript(ScriptModule parentModule, string scriptFile);
	/*!
	Create ScriptModule from string input.
	Available in developer builds (workbench or diag) and in headless server.
	\param parentModule Module
	\param text to compile
	\param errorText output from compiler
	\param errorLine output from compiler
	\returns Loaded scripted module
	*/
	static proto ref ScriptModule CompileScript(ScriptModule parentModule, string text, out string errorText, out int errorLine);
}

/*!
\}
*/
