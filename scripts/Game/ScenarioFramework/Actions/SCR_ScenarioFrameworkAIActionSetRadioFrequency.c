//! Action to set a radio frequency
[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionSetRadioFrequency : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Radio frequency")]
	protected int m_iFrequency;

	[Attribute(desc: "Use encryption key if you want to set the frequency at specific faction radio. If for an example character has both US and USSR radio and only one is supposed to be tuned. If empty, first found radio will be tuned.")]
	protected string m_sEncryptionKey;

	//------------------------------------------------------------------------------------------------
	//! Returns a gadget of EGadgetType.RADIO
	protected BaseRadioComponent GetCommunicationDevice(notnull IEntity controlledEntity, string encryptionKey)
	{
		SCR_GadgetManagerComponent gagdetManager = SCR_GadgetManagerComponent.Cast(controlledEntity.FindComponent(SCR_GadgetManagerComponent));
		if (!gagdetManager)
			return null;

		if (SCR_StringHelper.IsEmptyOrWhiteSpace(encryptionKey))
		{
			IEntity ent = gagdetManager.GetGadgetByType(EGadgetType.RADIO);
			if (ent)
				return BaseRadioComponent.Cast(ent.FindComponent(BaseRadioComponent));
			else
				return null;
		}
		else
		{
			array<SCR_GadgetComponent> radioGadgets = gagdetManager.GetGadgetsByType(EGadgetType.RADIO);
			if (radioGadgets.IsEmpty())
				return null;

			IEntity ent;
			BaseRadioComponent radio;
			foreach (SCR_GadgetComponent gadget : radioGadgets)
			{
				if (!gadget)
					continue;

				ent = gadget.GetOwner();
				if (!ent)
					continue;

				radio = BaseRadioComponent.Cast(ent.FindComponent(BaseRadioComponent));
				if (radio && radio.GetEncryptionKey() == encryptionKey)
					return radio;
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		{
			BaseRadioComponent radio = GetCommunicationDevice(object, m_sEncryptionKey);
			if (!radio)
				return;

			BaseTransceiver tsv = radio.GetTransceiver(0);
			if (!tsv)
				return;

			tsv.SetFrequency(m_iFrequency);
		}
	}
}
