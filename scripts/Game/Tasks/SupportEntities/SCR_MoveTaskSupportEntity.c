//------------------------------------------------------------------------------------------------
//[Obsolete(), EntityEditorProps(category: "GameScripted/Tasks", description: "Move task support entity.", color: "0 0 255 255")]
//class SCR_MoveTaskSupportEntityClass: SCR_EditorTaskSupportEntityClass
//{
//};

//------------------------------------------------------------------------------------------------
//[Obsolete()]
//class SCR_MoveTaskSupportEntity : SCR_EditorTaskSupportEntity
//{
//	//------------------------------------------------------------------------------------------------
//	void CreateMoveTask(vector targetPosition)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		SCR_MoveTask moveTask = SCR_MoveTask.Cast(CreateTask());
//		if (!moveTask)
//			return;
//		
//		MoveTask(targetPosition, moveTask.GetTaskID());
//	}
//};