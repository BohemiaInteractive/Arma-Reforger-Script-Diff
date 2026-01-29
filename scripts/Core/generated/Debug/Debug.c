/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Debug
\{
*/

sealed class Debug
{
	private void Debug();
	private void ~Debug();

	const int MB_PRESSED_MASK = 0x80000000;

	/*!
	Prints current call stack (stack trace). Call stack is printed to log or to 'output' variable if argument is used.
	\param output optional argument to get call stack to string variable instead of output
	\code
		Debug.DumpStack(); // print stack to log

		string text;
		Debug.DumpStack(text); // print stack to 'text' variable
	\endcode

	Output:
	\verbatim
		-- Stack trace --
		 SaveFile() Scripts\Entities\Modules\ModuleBase\ModuleFileHandler.c : 51
		 SaveConfigToFile() Scripts\Entities\Modules\ModuleBase\ModuleFileHandler\ModuleLocalProfile.c : 114
		 SaveParameterArray() Scripts\Entities\Modules\ModuleBase\ModuleFileHandler\ModuleLocalProfile.c : 133
		 SetParameterArray() Scripts\Entities\Modules\ModuleBase\ModuleFileHandler\ModuleLocalProfile.c : 231
		 PresetAdd() Scripts\Entities\Modules\ModuleBase\ModuleFileHandler\ModuleLocalProfile\ModuleLocalProfileUI.h : 46
		 OnKeyPress() Scripts/mission/missionGameplay.c : 215
		 OnKeyPress() Scripts/DayZGame.c : 334
		 -----------------
	\endverbatim
	*/
	static proto void DumpStack(out string output = string.Empty);
	/*!
	Dump all allocated script objects with callstack of its allocation into
	output/log. This can only be used together with `-checkInstance` CLI param.
	*/
	static proto void DumpInstances(bool csvFormatting);
	//! Dialog box with error message.
	static proto void Error2(string title, string err);
	//! Dialog box with error message.
	static proto void Error(string err);
	/*!
	Constructs a string that when used in log represents a clickable link. When clicked on a camera moves to view the entity.
	\param[in] entity used to generate the link
	\return clickable link in format: ENTITY:id (['name'] 'className') at \<x y z\>
	*/
	static proto string GetEntityLinkString(IEntity entity);
	/*!
	Constructs a string that when used in log represents a clickable link. When clicked on a camera moves to view the location.
	\param[in] position location in the world
	\param[in] size represents bounding box around the location
	\return clickable link in format: POSITION: \<x y z\> size
	*/
	static proto string GetPositionLinkString(vector position, float size);
	//! Starts measuring time until EndTimeMeasure() is called.
	static proto void BeginTimeMeasure();
	//! Ends time measurement which began with last BeginTimeMeasure() call.
	static proto void EndTimeMeasure(string title);
	//! Prints content of variable to console/log. Should be used for critical messages so it will appear in debug log.
	static proto void DPrint(string var);
	//! \warning Internal function for native (C++) code developers.
	static proto void Break(bool condition = true, void param1 = NULL, void param2 = NULL, void param3 = NULL, void param4 = NULL, void param5 = NULL, void param6 = NULL, void param7 = NULL, void param8 = NULL, void param9 = NULL);
	//! \warning Internal function for native (C++) code developers.
	static proto void CompileBreak();
	/*!
	Gets key state.
	\param key Key code.
	\return 0 when not pressed, Bit 15 is set when pressed, bits 0-14 contain
	count of presses.
	*/
	static proto int KeyState(KeyCode key);
	/*!
	Returns state of mouse button. It's combination of number of
	release/pressed edges and mask Debug.MB_PRESSED_MASK that is set when
	button is pressed. If you want just to check if button is pressed, use:
	\code
		if(GetMouseState(MouseState.LEFT) & Debug.MB_PRESSED_MASK))
			Print("left button pressed");
	\endcode
	*/
	static proto int GetMouseState(MouseState index);
	/*!
	Clears the key state. Call this function if you want to overcome
	autorepeating in reporting key state. If called, the KeyState returns
	pressed only after the key is released and pressed again.
	*/
	static proto void ClearKey(KeyCode key);
}

/*!
\}
*/
