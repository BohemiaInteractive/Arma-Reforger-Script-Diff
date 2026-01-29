// Confirmation dialog for unsubscribing multiple addons.
// On confirm it deletes local data and unsubscribes the addons.
// TODO: centralize handling of deletion, this should not be done by a dialog
// TODO: show a list of selcted mods
class SCR_DeleteAddonsListDialog : SCR_ConfigurableDialogUi
{
	protected ref array<SCR_WorkshopItem> m_aItems = {};

	//------------------------------------------------------------------------------------------------
	static SCR_DeleteAddonsListDialog CreateDialog(array<SCR_WorkshopItem> items)
	{
		return new SCR_DeleteAddonsListDialog(items);
	}

	//------------------------------------------------------------------------------------------------
	private void SCR_DeleteAddonsListDialog(array<SCR_WorkshopItem> items)
	{
		m_aItems = items;
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_WorkshopDialogs.DIALOGS_CONFIG, "deleteAll", this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		foreach (SCR_WorkshopItem item : m_aItems)
		{
			if (item.GetEnabled())
				SCR_AddonManager.GetInstance().GetPresetStorage().ClearUsedPreset();

			if (item.GetSubscribed())
				item.SetSubscribed(false);

			item.DeleteLocally();
		}

		super.OnConfirm();
	}
}