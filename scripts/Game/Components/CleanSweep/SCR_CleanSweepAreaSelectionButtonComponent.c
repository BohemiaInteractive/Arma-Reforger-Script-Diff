class SCR_CleanSweepAreaSelectionButtonComponent : ScriptedWidgetComponent
{
	//------------------------------------------------------------------------------------------------
	//! An event called when the button, this component is attached to, is clicked
	override bool OnClick(Widget w, int x, int y, int button)
	{
		Widget parent = w.GetParent();
		if (!parent)
			return false;
		
		CheckBoxWidget swapSidesBox = CheckBoxWidget.Cast(parent.FindAnyWidget("SwapSidesBox"));
		XComboBoxWidget selectionBox = XComboBoxWidget.Cast(parent.FindAnyWidget("SelectionBox"));
		if (!selectionBox)
			return false;
		
		// Find local player controller
		GenericEntity playerController = GetGame().GetPlayerController();
		if (!playerController)
			return false;
		
		SCR_CleanSweepNetworkComponent networkComponent = SCR_CleanSweepNetworkComponent.Cast(playerController.FindComponent(SCR_CleanSweepNetworkComponent));
		if (!networkComponent)
		{
			Print("No clean sweep network component found. SCR_CleanSweepAreaSelectionButtonComponent.OnClick()");
			return false;
		}
		
		networkComponent.ChooseArea(selectionBox.GetCurrentItem(), swapSidesBox.IsChecked());
		
		return false;
	}
}
