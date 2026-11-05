[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_EffectsModuleChildComponentClass : ScriptComponentClass
{
};
class SCR_EffectsModuleChildComponent : ScriptComponent
{	
	protected vector m_vParentPosition;
	protected bool m_bIsMaster;
	
	protected SCR_EffectsModuleComponent m_Parent;
	
	protected bool m_bOnDeleteCalled;
	
	//------------------------------------------------------------------------------------------------
	void InitChildServer(notnull SCR_EffectsModuleComponent effectModule, notnull SCR_EffectsModule effectConfig)
	{		
		m_bIsMaster = true;
		m_vParentPosition = effectModule.GetOwner().GetOrigin();
		
		InitChild(effectModule, effectConfig);
	}
	
	//------------------------------------------------------------------------------------------------
	void InitChild(notnull SCR_EffectsModuleComponent effectModule, notnull SCR_EffectsModule effectConfig)
	{		
		if (m_Parent)
			return;

		m_Parent = effectModule;
		//effectModule.GetOnDelete().Insert(OnParentEffectsModuleDeleted);
		effectModule.GetOnEditorOnRemovedFromParent().Insert(EditorOnParentRemoved);
		effectModule.GetOnTransformChanged().Insert(OnParentEffectsModuleUpdate);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnParentEffectsModuleUpdate(notnull SCR_EffectsModuleComponent effectModule, notnull SCR_EffectsModule effectConfig)
	{
		if (!m_bIsMaster)
		{
			if (!effectConfig.m_bSnapToTerrain)
			{
				GetOwner().OnTransformReset();
				GetOwner().Update();
			}
			
			return;
		}
		
		m_vParentPosition = effectModule.GetOwner().GetOrigin();
		
		//~ Snap to terrain		
		if (effectConfig.m_bSnapToTerrain)
		{
			vector position = GetOwner().GetOrigin();
			position[1] = SCR_TerrainHelper.GetTerrainY(position);
		
			RPC_UpdatePosition(position);
			Rpc(RPC_UpdatePosition, position);
		}
		else
		{
			GetOwner().OnTransformReset();
			GetOwner().Update();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the instigator for all triggers present on this entity
	//! \param[in] author who will be blamed for damage done by this entity
	void SetInstigator(SCR_EditableEntityAuthor author)
	{
		array<Managed> triggers = {};
		GetOwner().FindComponents(BaseProjectileComponent, triggers);
		if (triggers.IsEmpty())
			return;

		bool isGM = !author || author && author.m_bIsGM;
		int id;
		if (author)
			id = author.m_iAuthorID;

		BaseProjectileComponent trigger;
		foreach (Managed entry : triggers)
		{
			trigger = BaseProjectileComponent.Cast(entry);
			if (!trigger)
				continue;

			if (isGM)
				trigger.GetInstigator().SetInstigatorTypeToGM(id);
			else
				trigger.GetInstigator().SetInstigatorByPlayerID(id);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void EditorOnParentRemoved(SCR_EffectsModuleComponent effectModule, SCR_EffectsModule effectConfig)
	{
		delete GetOwner();
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Update position
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_UpdatePosition(vector position)
	{
		GetOwner().SetOrigin(position);
		GetOwner().OnTransformReset();
		GetOwner().Update();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (m_bIsMaster && m_Parent)
			m_Parent.OnEffectsModuleChildDeleted();
		
		if (m_Parent)
		{
			//m_Parent.GetOnDelete().Remove(OnParentEffectsModuleDeleted);
			m_Parent.GetOnEditorOnRemovedFromParent().Remove(EditorOnParentRemoved);
			m_Parent.GetOnTransformChanged().Remove(OnParentEffectsModuleUpdate);
		}
	}
};
