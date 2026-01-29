//! Confirmation dialog for unsubscribing an addon.
//! On confirm it deletes local data and unsubscribes the addon.
class SCR_DeleteAddonDialog : SCR_ConfigurableDialogUi
{
	protected ref SCR_WorkshopItem m_Item;

	//------------------------------------------------------------------------------------------------
	static SCR_DeleteAddonDialog CreateUnsubscribeAddon(SCR_WorkshopItem item)
	{
		return new SCR_DeleteAddonDialog(item, "unsubscribe");
	}

	//------------------------------------------------------------------------------------------------
	static SCR_DeleteAddonDialog CreateDeleteAddon(SCR_WorkshopItem item)
	{
		return new SCR_DeleteAddonDialog(item, "delete");
	}

	//------------------------------------------------------------------------------------------------
	private void SCR_DeleteAddonDialog(SCR_WorkshopItem item, string preset)
	{
		m_Item = item;
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_WorkshopDialogs.DIALOGS_CONFIG, preset, this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		// Clear preset name
		if (m_Item.GetEnabled())
			SCR_AddonManager.GetInstance().GetPresetStorage().ClearUsedPreset();
		else
			m_Item.SetEnabled(false);

		if (m_Item.GetSubscribed())
			m_Item.SetSubscribed(false);

		m_Item.DeleteLocally();

		super.OnConfirm();
	}
}