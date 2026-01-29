[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityType, "m_Type")]
class SCR_EditableEntityCoreTypeSetting
{
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EEditableEntityType))]
	private EEditableEntityType m_Type;
	
	[Attribute("1000", UIWidgets.Auto, "")]
	private float m_fMaxDrawDistance;
	
	[Attribute("0", desc: "Used for delaying update of budgets, players are not fully initialized when entity is registered in core")]
	private bool m_bCanBePlayer;
	
	[Attribute()]
	private ref SCR_EditableEntityInteraction m_Interaction;
	
	//[Attribute()]
	//private ref array<ref SCR_EditableEntityCoreTypeInteraction> m_aInteractions;
	
	private ref map<EEditableEntityType, EEditableEntityInteraction> m_InteractionsMap = new map<EEditableEntityType, EEditableEntityInteraction>;
	
	EEditableEntityType GetType()
	{
		return m_Type;
	}
	float GetMaxDrawDistance()
	{
		return m_fMaxDrawDistance;
	}
	bool GetCanBePlayer()
	{
		return m_bCanBePlayer;
	}
	SCR_EditableEntityInteraction GetInteraction()
	{
		return m_Interaction;
	}
	/*
	EEditableEntityInteraction GetInteraction(EEditableEntityType type)
	{
		EEditableEntityInteraction interaction;
		m_InteractionsMap.Find(type, interaction);
		return interaction;
	}
	
	void SCR_EditableEntityCoreTypeSetting()
	{
		foreach (SCR_EditableEntityCoreTypeInteraction interaction: m_aInteractions)
		{
			m_InteractionsMap.Insert(interaction.GetType(), interaction.GetInteraction());
		}
	}
	*/
};