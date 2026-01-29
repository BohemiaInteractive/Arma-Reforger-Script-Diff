[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditableEffectsModuleComponentClass: SCR_EditableDescriptorComponentClass
{
};

/** @ingroup Editable_Entities
*/

/*!
Editable Effect Module.
*/
class SCR_EditableEffectsModuleComponent: SCR_EditableSystemComponent
{
	//------------------------------------------------------------------------------------------------
	override bool IsDestroyed()
	{
		return !CanDestroy();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanDestroy()
	{	
		GenericEntity child = GenericEntity.Cast(GetOwner().GetChildren());
		while (child)
		{		
			if (child.FindComponent(SCR_PressureTriggerComponent) && child.FindComponent(SCR_EffectsModuleChildComponent))
				return true;
			
			child = GenericEntity.Cast(child.GetSibling());
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Destroys one entity. Will return true if all entities are destroyed
	override bool Destroy(int editorPlayerID)
	{		
		SCR_PressureTriggerComponent pressureTriggerComponent;
		
		GenericEntity child = GenericEntity.Cast(GetOwner().GetChildren());
		while (child)
		{	
			if (!child.FindComponent(SCR_EffectsModuleChildComponent))
			{
				child = GenericEntity.Cast(child.GetSibling());
				continue;
			}
		
			pressureTriggerComponent = SCR_PressureTriggerComponent.Cast(child.FindComponent(SCR_PressureTriggerComponent));
			if (!pressureTriggerComponent)
			{
				child = GenericEntity.Cast(child.GetSibling());
				continue;
			}
			
			pressureTriggerComponent.SetInstigator(Instigator.CreateInstigatorGM(editorPlayerID));
			pressureTriggerComponent.TriggerManuallyServer();
			break;
		}
		
		return IsDestroyed();
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetTransform(vector transform[4], bool changedByUser = false)
	{
		super.SetTransform(transform, changedByUser);
		
		if (!IsServer() || !m_Owner)
			return;
		
		SCR_EffectsModuleComponent effectModuleComponent = SCR_EffectsModuleComponent.Cast(m_Owner.FindComponent(SCR_EffectsModuleComponent));
		if (!effectModuleComponent)
			return;
		
		//~ Snaps non-editable children to terrain again if need be
		effectModuleComponent.OnTransformChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	override void RemoveParentEntity()
	{		
		RPC_RemoveParentEntityBroadcast();
		Rpc(RPC_RemoveParentEntityBroadcast);
		
		super.RemoveParentEntity();
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_RemoveParentEntityBroadcast()
	{
		SCR_EffectsModuleComponent effectModuleComponent = SCR_EffectsModuleComponent.Cast(GetOwner().FindComponent(SCR_EffectsModuleComponent));
		if (effectModuleComponent)
			effectModuleComponent.EditorOnRemovedFromParent();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool Delete(bool changedByUser = false, bool updateNavmesh = true)
	{
		SCR_EffectsModuleComponent effectModuleComponent = SCR_EffectsModuleComponent.Cast(GetOwner().FindComponent(SCR_EffectsModuleComponent));
		if (!effectModuleComponent || effectModuleComponent.EditorInstantDelete())
			return super.Delete(changedByUser, updateNavmesh);
		
		RemoveParentEntity();
		return true;
	}
}