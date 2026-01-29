[EntityEditorProps(insertable: false)]
class SCR_Tutorial_VehicleMaintenance_CLOSE_INVENTORYClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_VehicleMaintenance_CLOSE_INVENTORY : SCR_BaseTutorialStage
{
	SCR_InventoryMenuUI m_InventoryMenu;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_TutorialLogic_VehicleMaintenance logic = SCR_TutorialLogic_VehicleMaintenance.Cast(m_TutorialComponent.GetActiveConfig().GetCourseLogic());
		if (logic)
			GetGame().OnInputDeviceIsGamepadInvoker().Remove(logic.OnInputChanged);
		
		if (!SCR_InventoryMenuUI.GetInventoryMenu())
			m_bFinished = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return !SCR_InventoryMenuUI.GetInventoryMenu();
	}
};