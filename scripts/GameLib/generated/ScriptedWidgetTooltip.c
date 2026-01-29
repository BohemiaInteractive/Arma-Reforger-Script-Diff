/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
Base class for custom UI tooltips.
*/
class ScriptedWidgetTooltip: ScriptAndConfig
{
	//! Adjust tooltip position on screen edges
	static proto void CheckOverflow(WorkspaceWidget pWorkspace, inout float desiredPosX, inout float desiredPosY, float desiredSizeX, float desiredSizeY);

	// callbacks

	event void Show(WorkspaceWidget pWorkspace, Widget pToolTipWidget, float desiredPosX, float desiredPosY);
	event void Hide(WorkspaceWidget pWorkspace, Widget pToolTipWidget);
	//! Create UI widgets for tooltip class (each tooltip class has just one instance of its widgets)
	static event Widget CreateTooltipWidget();
}
