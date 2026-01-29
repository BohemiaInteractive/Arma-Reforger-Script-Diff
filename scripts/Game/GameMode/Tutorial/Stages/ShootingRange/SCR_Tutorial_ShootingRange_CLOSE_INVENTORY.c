[EntityEditorProps(insertable: false)]
class SCR_Tutorial_ShootingRange_CLOSE_INVENTORYClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_ShootingRange_CLOSE_INVENTORY : SCR_BaseTutorialStage
{
	SCR_InventoryMenuUI m_InventoryMenu;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();

		SCR_TutorialLogic_ShootingRange logic = SCR_TutorialLogic_ShootingRange.Cast(m_TutorialComponent.GetActiveConfig().GetCourseLogic());
		if (logic)
			GetGame().OnInputDeviceIsGamepadInvoker().Remove(logic.OnInputChanged);
		
		if (!SCR_InventoryMenuUI.GetInventoryMenu())
		{
			m_bFinished = true;
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return !SCR_InventoryMenuUI.GetInventoryMenu();
	}
};