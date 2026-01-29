[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditableExplosiveChargeComponentClass : SCR_EditableSystemComponentClass
{
	[Attribute(desc: "If charge should be armed by default when spawned by the GM.")]
	protected bool m_bArmedByDefault;

	[Attribute("120", desc: "Default fuze time that will be used when object is spawned by GM.")]
	protected float m_fDefaultFuzeTime;

	//------------------------------------------------------------------------------------------------
	bool IsArmedByDefault()
	{
		return m_bArmedByDefault;
	}

	//------------------------------------------------------------------------------------------------
	float GetDefaultFuzeTime()
	{
		return m_fDefaultFuzeTime;
	}
}

//! @ingroup Editable_Entities
class SCR_EditableExplosiveChargeComponent : SCR_EditableSystemComponent
{
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		//Activate charge on server one frame later so the TriggerComponent can initialize
		if (IsServer())
			GetGame().GetCallqueue().CallLater(ActivateCharge);
	}

	//------------------------------------------------------------------------------------------------
	//! Arms the charge with 120s fuze
	protected void ActivateCharge()
	{
		SCR_EditableExplosiveChargeComponentClass data = SCR_EditableExplosiveChargeComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return;

		SCR_ExplosiveChargeComponent explosiveChargeComp = SCR_ExplosiveChargeComponent.Cast(GetOwner().FindComponent(SCR_ExplosiveChargeComponent));
		if (explosiveChargeComp)
		{
			explosiveChargeComp.SetFuzeTime(data.GetDefaultFuzeTime(), true);
			if (data.IsArmedByDefault())
				explosiveChargeComp.ArmWithTimedFuze(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDestroyed()
	{
		//~ If this component exists it is not destroyed
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanDestroy()
	{
		//~ If this component exists it can be destroyed
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool Destroy(int editorPlayerID)
	{
		if (!IsServer())
			return false;

		SCR_ExplosiveTriggerComponent explosiveChargeTrigger = SCR_ExplosiveTriggerComponent.Cast(GetOwner().FindComponent(SCR_ExplosiveTriggerComponent));
		if (explosiveChargeTrigger)
		{
			explosiveChargeTrigger.SetInstigator(Instigator.CreateInstigatorGM(editorPlayerID));
			explosiveChargeTrigger.UseTrigger();
			return true;
		}

		return false;
	}
}
