/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Debug
\{
*/

/*!
Immediate mode debug UI API for in-game overlays and developer tooling.

Widgets are declared every frame inside a Begin()/End() block. The system retains
no per-widget state across frames — each call rebuilds the widget from the
arguments supplied.

Per-frame usage example:
\code
	bool m_ShowDbgUI = false;
	int m_DbgListSelection = 0;
	float m_DbgSliderValue = 0.0;
	array<string> m_DbgOptions = {"one", "two", "three"};

	void OnUpdate(float timeslice)
	{
		DbgUI.Begin("Test");
		DbgUI.Check("Show DbgUI", m_ShowDbgUI);
		if (m_ShowDbgUI)
		{
			DbgUI.Text("DbgUI Test");

			string name = "";
			DbgUI.InputText("name", name);

			if (DbgUI.Button("Print name"))
				Print(name);

			DbgUI.List("test list", m_DbgListSelection, m_DbgOptions);
			DbgUI.Text("Choice = " + m_DbgListSelection.ToString());

			DbgUI.Spacer(10);
			DbgUI.SliderFloat("slider", m_DbgSliderValue, 0, 100);
			DbgUI.Text("Slider value = " + ftoa(m_DbgSliderValue));
		}
		DbgUI.End();
	}
\endcode

When widgets are driven by events rather than per-frame updates, wrap the block in
BeginCleanupScope()/EndCleanupScope() so stale widgets are removed when the event
stops firing:
\code
	int m_DbgEventCount = 0;

	void OnEvent(EventType eventTypeId, Param params)
	{
		m_DbgEventCount++;

		DbgUI.BeginCleanupScope();
		DbgUI.Begin("events", 300, 0);
		DbgUI.Text("Events count = " + m_DbgEventCount.ToString());
		DbgUI.End();
		DbgUI.EndCleanupScope();
	}
\endcode
*/
sealed class DbgUI
{
	private void DbgUI();
	private void ~DbgUI();

	//! Creates all possible DbgUI widgets. Intended for testing purposes only.
	static proto void DoUnitTest();
	//! Draws a static text label.
	static proto void Text(string label);
	/*!
	Draws a formatted static text label. Accepts up to nine optional arguments
	using the same format specifiers as PrintFormat().
	\param fmt Format string.
	*/
	static proto void TextFormat(string fmt, void param1 = NULL, void param2 = NULL, void param3 = NULL, void param4 = NULL, void param5 = NULL, void param6 = NULL, void param7 = NULL, void param8 = NULL, void param9 = NULL);
	//! Places the next widget on the same line as the previous one.
	static proto void SameLine();
	//! Places the next widget at the same position as the previous one, overlapping it.
	static proto void SameSpot();
	//! Sets left alignment for subsequent widgets within the given pixel width.
	static proto void AlignLeft(int width);
	//! Sets center alignment for subsequent widgets within the given pixel width.
	static proto void AlignCenter(int width);
	//! Sets right alignment for subsequent widgets within the given pixel width.
	static proto void AlignRight(int width);
	/*!
	Draws a scrolling live plot that appends a new sample every timeStep milliseconds.
	\param[in] label        Widget label and unique identifier.
	\param[in] sizeX        Plot width in pixels.
	\param[in] sizeY        Plot height in pixels.
	\param[in] val          Current sample value.
	\param[in] timeStep     Interval between samples in milliseconds.
	\param[in] historySize  Number of samples retained in the rolling buffer.
	\param[in] color        Plot line color in ARGB format.
	*/
	static proto void PlotLive(string label, int sizeX, int sizeY, float val, int timeStep = 100, int historySize = 30, int color = 0xFFFFFFFF);
	/*!
	Draws a scrolling live plot with a fixed visible Y range.
	\see PlotLive
	\param[in] label        Widget label and unique identifier.
	\param[in] sizeX        Plot width in pixels.
	\param[in] sizeY        Plot height in pixels.
	\param[in] val          Current sample value.
	\param[in] yMin         Minimum visible Y value.
	\param[in] yMax         Maximum visible Y value.
	\param[in] timeStep     Interval between samples in milliseconds.
	\param[in] historySize  Number of samples retained in the rolling buffer.
	\param[in] color        Plot line color in ARGB format.
	*/
	static proto void PlotLiveClamped(string label, int sizeX, int sizeY, float val, float yMin, float yMax, int timeStep = 100, int historySize = 30, int color = 0xFFFFFFFF);
	/*!
	Draws a checkbox. The checked state is written back to the caller each frame.
	\param[in]  label    Widget label and unique identifier.
	\param[out] checked  Current checked state; toggled when the user clicks the widget.
	*/
	static proto void Check(string label, out bool checked);
	/*!
	Draws a drop-down combo box populated from an array of strings.
	\param[in]  label      Widget label and unique identifier.
	\param[out] selection  Index of the currently selected element.
	\param[in]  elems      Array of display strings.
	*/
	static proto void Combo(string label, out int selection, TStringArray elems);
	/*!
	Draws a scrollable list box populated from an array of strings.
	\param[in]  label      Widget label and unique identifier.
	\param[out] selection  Index of the currently selected element.
	\param[in]  elems      Array of display strings.
	*/
	static proto void List(string label, out int selection, TStringArray elems);
	/*!
	Draws a horizontal float slider.
	\param[in]  label    Widget label and unique identifier.
	\param[out] value    Current slider value; updated when the user drags the handle.
	\param[in]  min      Minimum allowed value.
	\param[in]  max      Maximum allowed value.
	\param[in]  pxWidth  Width of the slider track in pixels.
	*/
	static proto void SliderFloat(string label, out float value, float min, float max, int pxWidth = 150);
	//! Inserts vertical blank space of the given height in pixels.
	static proto void Spacer(int height);
	/*!
	Draws a solid color rectangle with a label.
	\param[in] label   Widget label and unique identifier.
	\param[in] width   Panel width in pixels.
	\param[in] height  Panel height in pixels.
	\param[in] color   Fill color in ARGB format.
	*/
	static proto void Panel(string label, int width, int height, int color = 0xaa555555);
	/*!
	Draws a clickable button.
	\param[in] txt       Button label text.
	\param[in] minWidth  Minimum button width in pixels. Pass 0 to size to content.
	\return True on the frame the button is clicked.
	*/
	static proto bool Button(string txt, int minWidth = 0);
	/*!
	Draws an image loaded from the given resource path.
	\param[in] label    Widget label and unique identifier.
	\param[in] imgPath  Path to the image resource.
	\param[in] width    Display width in pixels.
	\param[in] height   Display height in pixels.
	*/
	static proto void Image(string label, ResourceName imgPath, int width, int height);
	/*!
	Draws a single-line text input field.
	\param[in]  txt      Widget label and unique identifier.
	\param[out] value    Current text value; updated as the user types.
	\param[in]  pxWidth  Input field width in pixels.
	*/
	static proto void InputText(string txt, out string value, int pxWidth = 150);
	/*!
	Draws an integer input field.
	\param[in]  txt      Widget label and unique identifier.
	\param[out] value    Current integer value; updated as the user edits.
	\param[in]  pxWidth  Input field width in pixels.
	*/
	static proto void InputInt(string txt, out int value, int pxWidth = 150);
	/*!
	Draws a float input field.
	\param[in]  txt      Widget label and unique identifier.
	\param[out] value    Current float value; updated as the user edits.
	\param[in]  pxWidth  Input field width in pixels.
	*/
	static proto void InputFloat(string txt, out float value, int pxWidth = 150);
	/*!
	Opens a cleanup scope. Any widget created inside the scope that is not
	declared in a subsequent frame is automatically removed. Must be paired
	with EndCleanupScope(). Use this pattern when widgets are created from
	event handlers rather than from a per-frame update.
	*/
	static proto void BeginCleanupScope();
	//! Closes a cleanup scope opened by BeginCleanupScope().
	static proto void EndCleanupScope();
	/*!
	Opens a debug UI window scope. Must be paired with End(). Call every frame that
	the window should be visible.
	\param[in] windowTitle  Title shown in the window title bar and used as unique identifier.
	\param[in] x            Initial horizontal position in pixels from the left edge of the screen. Pass 0 to use automatic placement.
	\param[in] y            Initial vertical position in pixels from the top edge of the screen. Pass 0 to use automatic placement.
	*/
	static proto void Begin(string windowTitle, float x = 0, float y = 0);
	//! Closes a debug UI window scope opened by Begin().
	static proto void End();
	/*!
	Pushes an identifier onto the widget ID stack to disambiguate widgets that
	share the same label string. Must be paired with PopID().
	\param[in] id  Any value whose string representation is used as the ID component.
	*/
	static proto void PushID(void id);
	//! Pops the identifier pushed by the last PushID() call.
	static proto void PopID();
}

/*!
\}
*/
