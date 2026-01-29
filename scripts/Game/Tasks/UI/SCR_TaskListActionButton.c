//------------------------------------------------------------------------------------------------
class SCR_TaskListActionButton : ScriptedWidgetComponent
{
	// todo(koudelkaluk): delet
	[Attribute("0", UIWidgets.ComboBox, "Action type", "", ParamEnumArray.FromEnum(ETaskListActions) )]
	ETaskListActions m_Action;
};

enum ETaskListActions
{
	NONE,
	CANCEL,
	DETAIL,
	ASSIGN,
	UNASSIGN,
	ASSIGN_TO,
	SHOW_ON_MAP,
	CLOSE_TASK_LIST,
	CLOSE_PICK_ASSIGNEE,
	CLOSE_DETAIL
};