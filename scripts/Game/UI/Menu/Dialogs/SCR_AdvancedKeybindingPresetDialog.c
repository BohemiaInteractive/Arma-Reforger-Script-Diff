[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sTag")]
class SCR_AdvancedKeybindingPresetDialog : SCR_ConfigurableDialogUiPreset
{
	[Attribute(defvalue: "", desc: "Input Filter Configurations")]
	protected ref array<ref SCR_InputFilterConfiguration> m_aInputFilterConfigurations;

	ref array<ref SCR_InputFilterConfiguration> GetInputFilterConfigurations()
	{
		return m_aInputFilterConfigurations;
	}
}