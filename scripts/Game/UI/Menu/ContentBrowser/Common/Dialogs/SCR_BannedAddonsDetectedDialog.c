//! Dialog which lists names of all banned addons
class SCR_BannedAddonsDetectedDialog : SCR_ConfigurableDialogUi
{
	ref array<string> m_aItemNames = {};

	//------------------------------------------------------------------------------------------------
	void SCR_BannedAddonsDetectedDialog(array<string> items)
	{
		m_aItemNames.Copy(items);
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_WorkshopDialogs.DIALOGS_CONFIG, "banned_addons_detected", this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		string message = "#AR-Workshop_Baned_Item";
		foreach (string name : m_aItemNames)
		{
			message = message + "\n- " + name;
		}

		SetMessage(message);

		super.OnMenuOpen(preset);
	}
}