[ComponentEditorProps(category: "GameScripted/Test", description: "This brief script description.")]
class SCR_InteractableBoxComponentClass : ScriptComponentClass
{
}

class SCR_InteractableBoxComponent : ScriptComponent
{
	private bool m_bIsDragging = false;
	private bool m_bIsOnFire = false;
	private IEntity m_Owner = null;
	private IEntity m_User = null;
	private ParticleEffectEntity m_Fire = null;
	private float m_Lifetime = 10.0; // object can burn for 15 seconds before dying
	private bool m_bIsDead = false;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] particle
	//! \param[in] offset
	void ToggleIsOnFire(ResourceName particle, vector offset)
	{
		if (m_bIsDead)
		{
			if (m_Fire)
			{
				delete m_Fire;
				m_Fire = null;
				m_bIsOnFire = false;
			}
			return;
		}
		
		m_bIsOnFire = !m_bIsOnFire;		
		if (m_bIsOnFire && particle != string.Empty)
		{
			if (!m_Fire)
			{
				ParticleEffectEntitySpawnParams spawnParams = new ParticleEffectEntitySpawnParams();
				spawnParams.Parent = m_Owner;
				spawnParams.Transform[3] = offset;
				m_Fire = ParticleEffectEntity.SpawnParticleEffect(particle, spawnParams);
			}
		}
		else
		{
			if (m_Fire)
			{
				m_Fire.Stop();
				delete m_Fire;
				m_Fire = null;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool IsDragging()
	{
		return m_bIsDragging;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	string GetActionName()
	{
		if (m_bIsDragging)
		{
			return "Let Go";
		}
		else
		{
			return "Drag";
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	string GetActionDescription()
	{
		if (!m_bIsDragging)
		{
			return "Start dragging this object around.";
		}
		else
		{
			return "Let go of this object.";
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	string GetFireActionDescription()
	{
		if (!m_bIsOnFire)
		{
			return "Use lighter to set this object on fire!";
		}
		else
		{
			return "Attempt to extinguish this object.";
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	string GetFireActionName()
	{
		if (!m_bIsOnFire)
		{
			return "Burn";
		}
		else
		{
			return "Extinguish";
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool CanPerformDragAction()
	{
		if (m_bIsOnFire)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool CanPerformToggleFire()
	{
		if (m_bIsDragging)
			return false;
		
		return (!m_bIsDead);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void CancelDragAction()
	{
		m_bIsDragging = false;
		m_User = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] user
	void PerformDragAction(IEntity user)
	{	
		if (!m_Owner)
			return;
		
		if (m_bIsOnFire)
			return;
		
		m_bIsDragging = !m_bIsDragging;
		if (m_bIsDragging)
		{
			m_User = user;
			return;
		}
		else
		{
			m_User = null;
			return;			
		}		
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnSimulate(IEntity owner, float timeSlice)
	{
		if (m_User && m_Owner && m_bIsDragging)
		{
			Physics physics = m_Owner.GetPhysics();
			if (!physics)
			{
				m_bIsDragging = false;
				m_User = null;
			}
			
			vector userMat[4];
			m_User.GetWorldTransform(userMat);					
			vector userTgtPos = userMat[3] + userMat[2] * 1.3;
			vector selfOrigin = m_Owner.GetOrigin();
			vector dir = userTgtPos - selfOrigin;
			dir[1] = Math.Clamp(dir[1], -0.5, 0.5);			
			float distance = vector.Distance(userTgtPos, selfOrigin);			
			if (distance > 1.5)
			{
				m_bIsDragging = false;
				m_User = null;
				return;
			}
			
			float upMag = userMat[3][1] - selfOrigin[1];
			dir += Vector(0, upMag, 0);
			physics.ApplyImpulse(dir * physics.GetMass());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnDeath()
	{
		VObject obj = m_Owner.GetVObject();		
		string materials[256];
		int numMats = obj.GetMaterials(materials);
		string remap = "";
		for (int i = 0; i < numMats; i++)
		{
			remap += "$remap '" + materials[i] + "' '{DD86C0AE16B22569}Assets/Props/Garbage/Data/MI_RubberForTrashBinContainer_01.emat';";
		}
		m_Owner.SetObject(obj, remap);
		
		ToggleIsOnFire(string.Empty, vector.Zero);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (m_bIsOnFire)
			m_Lifetime -= timeSlice;
		if (m_Lifetime < 0)
		{
			if (!m_bIsDead)
			{
				OnDeath();
				m_bIsDead = true;
			}
			m_Lifetime = 0.0;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME | EntityEvent.SIMULATE);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_InteractableBoxComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_Owner = ent;
	}
}
