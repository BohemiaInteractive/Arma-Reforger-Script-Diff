[EntityEditorProps(category: "GameScripted/TreeDestructionV2", description: "A part of tree that reacts to damage.", color: "0 0 255 255", visible: false, dynamicBox: true)]
class SCR_TreePartV2Class: TreeClass
{
};

//------------------------------------------------------------------------------------------------
//Encapsulates the functionality of a tree part entity in the world.
class SCR_TreePartV2 : Tree
{
	//********************//
	//ATTRIBUTES - PHYSICS//
	//********************//
	[Attribute("1", UIWidgets.EditBox, "Enter the mass of this tree part.", category: "Physics settings")]
	private float m_fMass;
	[Attribute("9", UIWidgets.EditBox, "Enter the max linear acceleration of this tree part for each local axis.", category: "Physics settings")]
	private float m_fMaxAccelerationLinear;
	[Attribute("5 5 5", UIWidgets.EditBox, "Enter the max angular acceleration of this tree part.", category: "Physics settings")]
	private vector m_vMaxAccelerationAngular;
	[Attribute("0.05", UIWidgets.EditBox, "Enter the max speed of rotation around local Y axis of this tree part.", category: "Physics settings")]
	private float m_fMaxYRotationVelocity;
	[Attribute("1", UIWidgets.CheckBox, "Allow linear movement of this object?", category: "Physics settings")]
	private bool m_bAllowLinearMovement;
	[Attribute("1", UIWidgets.CheckBox, "Allow rotation around global Y axis of this object?", category: "Physics settings")]
	private bool m_bAllowGlobalYRotation;
	[Attribute("200", UIWidgets.EditBox, "Enter the max joint load.", category: "Physics settings")]
	private float m_fMaxJointLoad;
	
	//*******************//
	//ATTRIBUTES - DAMAGE//
	//*******************//
	[Attribute("1", UIWidgets.EditBox, "Enter the minimum impact that should be able to move with this object. Impact = reduced impulse.", category: "Damage settings")]
	private float m_fMinImpact;
	
	
	[Attribute("1", UIWidgets.EditBox, "Enter explosion resistance of this tree part. Every explosive impulse on this tree part will be divided by this number.", category: "Damage settings")]
	private float m_fExplosionResistance;
	[Attribute("1", UIWidgets.EditBox, "Enter kinetic resistance of this tree part. Every kinetic (projectile) impulse on this tree part will be divided by this number.", category: "Damage settings")]
	private float m_fKineticResistance;
	[Attribute("1", UIWidgets.EditBox, "Enter collision resistance of this tree part. Every collision (vehicle) impulse on this tree part will be divided by this number.", category: "Damage settings")]
	private float m_fCollisionResistance;
	[Attribute("1", UIWidgets.EditBox, "Enter melee resistance of this tree part. Every melee impulse on this tree part will be divided by this number.", category: "Damage settings")]
	private float m_fMeleeResistance;

	[Attribute("0", UIWidgets.CheckBox, "Allows you to debug values that are damage related.", category: "Damage settings")]
	bool m_bDebugDamage;
	
	//************************//
	//ATTRIBUTES - RECOGNITION//
	//************************//
	[Attribute("0", UIWidgets.EditBox, "Enter the index of this tree part.", category: "Recognition settings")]
	private int m_iTreePartIndex;
	[Attribute("-1", UIWidgets.ResourceNamePicker, "Pick the root dirt prefab.", category: "Root dirt settings")]
	private ResourceName m_RootDirt;
	[Attribute("0 0 0", UIWidgets.Auto, "The offset of the root dirt object.", category: "Root dirt settings")]
	private vector m_vRootDirtOffset;
	
#ifdef ENABLE_DESTRUCTION
	
	static SCR_DestructibleTreesSynchManager synchManager = null;
	
	//*********************//
	//GLOBAL SLEEP SETTINGS//
	//*********************//
	static const float GO_TO_STATIC_THRESHOLD = 0.2;
	static const float GO_TO_STATIC_ANGULAR_THRESHOLD = 0.2;
	static const int GO_TO_STATIC_TIME_THRESHOLD = 2;
	static const int GO_TO_STATIC_TIME_THRESHOLD_MAX = 20;
	
	//*******************************//
	//GLOBAL SYNCHRONIZATION SETTINGS//
	//*******************************//
	static const int TARGET_RPC_COUNT = 4;
	static const float NET_TELEPORT_DISTANCE = 10;
	static const float NET_TELEPORT_ANGLE = 5;
	
	//****************************//
	//MEMBER COMPONENTS & ENTITIES//
	//****************************//
	private Physics m_Physics = null;
	
	//TODO Minimize the stored data amount
	//************************//
	//RUNTIME MEMBER VARIABLES//
	//************************//
	private vector m_vCenterOfMass;
	private float m_fLastSpeedLinear = 0;
	private vector m_vLastSpeedAngular = "0 0 0";
	private bool m_bSpawnedRootBall = false;
	private IEntity m_RootsEntity = null;
	private Physics m_RootsPhysics = null;
	private vector m_vBBOXMin;
	private vector m_vBBOXMax;
	private vector m_vSynchVelocity;
	private vector m_vLerpStartVector;
	private vector m_vLerpTargetPosition;
	private float m_fLerpStartQuat[4];
	private float m_fLerpTargetQuat[4];
	private float m_fLerpAmountPos = -1;
	private float m_fLerpAmountRot = -1;
	private ref array<IEntity> m_aQuerriedEnts = new array<IEntity>();
	private float m_fSynchTime = 0;
	vector m_vLockedOrigin;
	SCR_DestructibleTreeV2 m_ParentTree = null;
	
	private vector m_vNetPosition;
	private vector m_vNetVelocityLinear;
	private vector m_vNetVelocityAngular;
	private float m_fTimeSinceLastTick;
	private float m_fNetRotation[4];
	private bool m_bExtrapolate = false;
	
	//************//
	//DATA CACHING//
	//************//
	private bool m_bApplyCachedRotation = false;
	private float m_fCachedRotation[4];
	private bool m_bApplyCachedPosition = false;
	private vector m_vCachedPosition;
	private bool m_bBreak = false;
	private vector m_vImpulseVector = "0 0 0";
	private vector m_vPositionVector = "0 0 0";
	private EDamageType m_eDamageType;
	
	//*****//
	//SLEEP//
	//*****//
	private float m_fThresholdTime = 0;
	private float m_fThresholdMaxTime = 0;
	
	//*****************//
	//PHYSICS SWITCHING//
	//*****************//
	private bool m_bWakeUp = false;
	private bool m_bSwitchToDynamic;
	private bool m_bSwitchToStatic;
	private ref SCR_HybridPhysicsInfo m_HybridPhysicsInfo = null;
	private int m_iContactsCount = 0;
	private bool m_bBreakFromParent = false;
	
	//******//
	//JOINTS//
	//******//
	private bool m_bCreatedJoints = false;
	private ref array<PhysicsJoint> m_aChildrenJoints = new array<PhysicsJoint>();
	private PhysicsJoint m_ParentJoint;
	
	//***************//
	//SYNCHRONIZATION//
	//***************//
	private bool m_bSynchronizeHasParent = true;
	private static float m_fTargetSynchTime = -1;
	
	//*****//
	//SOUND//
	//*****//
	private bool m_bSoundHitGroundPlayed = false;
	
	//------------------------------------------------------------------------------------------------
	//! Clear the dynamic physics info
	private void ClearPhysicsInfo()
	{
		if (!m_HybridPhysicsInfo)
			return;
		
		delete m_HybridPhysicsInfo;
		m_HybridPhysicsInfo = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create and store dynamic physics info
	private void StorePhysicsInfo(IEntity owner)
	{
		if (!m_Physics || !owner)
			return;
		
		ClearPhysicsInfo();
		
		m_HybridPhysicsInfo = new SCR_HybridPhysicsInfo;
		m_HybridPhysicsInfo.m_fMass = m_Physics.GetMass();
		
		int numGeoms = m_Physics.GetNumGeoms();
		for (int i = 0; i < numGeoms; i++)
		{
			m_HybridPhysicsInfo.m_aLayerMasks.Insert(m_Physics.GetGeomInteractionLayer(i));
		}
		
		m_Physics.Destroy();
		m_Physics = Physics.CreateStatic(owner, -1);
		
		int numStoredGeoms = m_HybridPhysicsInfo.m_aLayerMasks.Count();
		numGeoms = m_Physics.GetNumGeoms();
		for (int i = 0; i < numStoredGeoms; i++)
		{
			if (i >= numGeoms)
				break;
			
			m_Physics.SetGeomInteractionLayer(i, m_HybridPhysicsInfo.m_aLayerMasks.Get(i));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Apply stored physics info to dynamic physics object
	private void ApplyPhysicsInfo(IEntity owner)
	{
		if (!m_Physics)
		{
			ClearPhysicsInfo();
			return;
		}
		if (!owner || !m_HybridPhysicsInfo)
			return;
		
		m_Physics.Destroy();
		m_Physics = Physics.CreateDynamic(owner, m_HybridPhysicsInfo.m_fMass, -1);
		
		int numStoredGeoms = m_HybridPhysicsInfo.m_aLayerMasks.Count();
		int numGeoms = m_Physics.GetNumGeoms();
		for (int i = 0; i < numStoredGeoms; i++)
		{
			if (i >= numGeoms)
				break;
			
			m_Physics.SetGeomInteractionLayer(i, m_HybridPhysicsInfo.m_aLayerMasks.Get(i));
		}
		
		ClearPhysicsInfo();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetToBreak(int treePartIdx = -1, vector positionVector = "0 0 0", vector impulseVector = "0 0 0", EDamageType damageType = EDamageType.MELEE)
	{
		m_bBreak = true;
		
		if (RplSession.Mode() == RplMode.Client || !synchManager)
			return;
		
		if (treePartIdx == -1)
			treePartIdx = m_iTreePartIndex;
		
		if (!m_ParentTree)
			return;
		
		synchManager.SynchronizeSetToBreak(treePartIdx, positionVector, impulseVector, damageType, m_ParentTree.GetID());
		
		m_ParentTree.SetFlags(EntityFlags.ACTIVE);
		
		SetEventMask(EntityEvent.FRAME);
		m_vImpulseVector = impulseVector * 50;
		m_vPositionVector = positionVector;
		ResetThresholdTime();
		m_eDamageType = damageType;
	}
	
	//------------------------------------------------------------------------------------------------
	//Break this tree part from it's parent and apply impulse.
	private void Break()
	{
		WakeUpHierarchy();
		
		if (!m_Physics || !m_Physics.IsDynamic())
			return;
		
		if (!m_bSpawnedRootBall && m_iTreePartIndex == 0)
			SpawnRootDirt();
		
		RemoveFromParent();
		
		if (m_vPositionVector == "0 0 0")
			m_vPositionVector = (m_vCenterOfMass + GetOrigin());
		
		//Apply the impulse to this tree part.
		if (m_eDamageType != EDamageType.COLLISION)
		{
			m_Physics.ApplyImpulseAt(m_vPositionVector, m_vImpulseVector);
			LimitAcceleration();
		}
		
		if (!m_bAllowLinearMovement)
		{
			SetOrigin(m_vLockedOrigin);
			m_Physics.SetVelocity("0 0 0");
		}
		
		WakeUp(true);
		WakeUpOthers();
		
		// SOUND
		if (!m_bSoundHitGroundPlayed) // ensure that SOUND_BREAK does not play after SOUND_HIT_GROUND
		{
			if (synchManager)
			{
				BaseSoundComponent soundComponent = synchManager.GetSoundComponent();
				if (soundComponent)
				{
					vector mat[4];
					Math3D.MatrixIdentity3(mat);
					mat[3] = m_vPositionVector;
					soundComponent.SetTransformation(mat);
					soundComponent.PlayStr(SCR_SoundEvent.SOUND_BREAK);
				}
			}
		}
		
		m_bBreak = false;
	}
	
	//------------------------------------------------------------------------------------------------
	//TODO: Update this 
	private void SpawnRootDirt()
	{
		if (m_RootDirt.GetPath() == "-1")
			return;
		
		Resource resource = Resource.Load(m_RootDirt);
		
		if (!resource.IsValid())
			return;
		
		vector worldPos = GetOrigin();
		
		//ALIGN TO NORMAL
		vector mat[4];
		TraceParam traceParam = new TraceParam();
		traceParam.Start = worldPos + vector.Up;
		traceParam.End = worldPos - vector.Up;
		traceParam.Flags = TraceFlags.WORLD;
		GetWorld().TraceMove(traceParam, null);
		GetTransform(mat);
		
		if (traceParam.TraceNorm != vector.Zero)
		{
			vector newUp = traceParam.TraceNorm;
			newUp.Normalize();
			
			//Shape shape;
			//shape = Shape.Create(ShapeType.LINE, ARGB(255, 0, 255, 0), ShapeFlags.NOZBUFFER, worldPos, worldPos + newUp);
			//m_aDebugShapes.Insert(shape);
			vector newRight = newUp * mat[2];
			newRight.Normalize();
			//shape = Shape.Create(ShapeType.LINE, ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, worldPos, worldPos + newRight);
			//m_aDebugShapes.Insert(shape);
			vector newForward = newRight * newUp;
			newForward.Normalize();
			//shape = Shape.Create(ShapeType.LINE, ARGB(255, 0, 0, 255), ShapeFlags.NOZBUFFER, worldPos, worldPos + newForward);
			//m_aDebugShapes.Insert(shape);
			
			mat[0] = newRight;
			mat[1] = newUp;
			mat[2] = newForward;
		}
		mat[3] = m_vRootDirtOffset;
		
		//Spawn parameters setup
		EntitySpawnParams param = new EntitySpawnParams();
		param.TransformMode = ETransformMode.LOCAL;
		param.Transform = mat;
		param.Parent = this;
		
		ArmaReforgerScripted game = GetGame();
		
		if (!game)
			return;
		
		m_RootsEntity = game.SpawnEntityPrefab(resource, GetWorld(), param);
		
		if (!m_RootsEntity)
			return;
		
		m_bSpawnedRootBall = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//Remove this tree part from it's parent, move it to correct world position and break it's parent joint.
	void RemoveFromParent()
	{
		IEntity parent = GetParent();
		if (!parent)
			return;
		
		vector parentMat[4];
		
		parent.RemoveChild(this);
		parent.GetTransform(parentMat);
		
		SetTransform(parentMat);
		SCR_TreePartV2 parentTreePart = SCR_TreePartV2.Cast(parent);
		BreakJoint(m_ParentJoint, this, parentTreePart);
		m_bSynchronizeHasParent = false;
		
		if (RplSession.Mode() == RplMode.Client)
			return;
		
		if (m_ParentTree && synchManager)
			synchManager.SynchronizeRemoveFromParent(m_iTreePartIndex, m_ParentTree.GetID());
	}
	
	//------------------------------------------------------------------------------------------------
	//Returns parent joint of this tree part.
	PhysicsJoint GetParentJoint()
	{
		return m_ParentJoint;
	}
	
	//------------------------------------------------------------------------------------------------
	//Removes the given joint and sets the pointer to null.
	void RemoveJoint(PhysicsJoint physicsJoint)
	{
		if (!physicsJoint)
			return;
		
		if (physicsJoint == m_ParentJoint)
		{
			m_ParentJoint = null;
			return;
		}
		
		int count = m_aChildrenJoints.Count();
		for (int i = 0; i < count; i++)
		{
			if (physicsJoint == m_aChildrenJoints.Get(i))
			{
				m_aChildrenJoints.Remove(i);
				return;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//Removes all the joins from this tree part.
	void DestroyAllJoints()
	{
		array<IEntity> children = new array<IEntity>();
		GetAllChildren(this, children);
		int count = children.Count();
		
		for (int i = 0; i < count; i++)
		{
			SCR_TreePartV2 treePart = SCR_TreePartV2.Cast(children[i]);
			if (treePart)
			{
				PhysicsJoint physicsJoint = treePart.GetParentJoint();
				if (physicsJoint)
					BreakJoint(physicsJoint, this, treePart);
				treePart.DestroyAllJoints();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//Switches physics of this tree part to static.
	private void SwitchPhysicsToStatic(bool switchChildrenPhysics)
	{
		if (m_Physics)
		{
			//If dynamic physics is present, destroy it.
			if (m_Physics.IsDynamic())
			{
				StorePhysicsInfo(this);
				DestroyPhysics();
			}
			else
			{
				m_bSwitchToStatic = false;
				return; //Only if static physics is already present.
			}
		}
		
		IEntity parent = GetParent();
		if (parent)
		{
			vector mat[4];
			parent.GetTransform(mat);
			SetTransform(mat);
		}
		
		m_bSwitchToStatic = false;
		m_bCreatedJoints = false;
		
		//Reset the sleep timer.
		ResetThresholdTime();
		
		//Generate new physics component.
		m_Physics = Physics.CreateStatic(this, -1);
		if (m_RootsEntity && !m_RootsPhysics)
		{
			m_RootsPhysics = Physics.CreateStatic(m_RootsEntity, -1);
		}
		
		//Switches all children physics to static.
		if (switchChildrenPhysics)
		{
			DestroyAllJoints();
			m_bCreatedJoints = false;
			array<IEntity> children = new array<IEntity>();
			SCR_Global.GetHierarchyEntityList(this, children);
			if (children)
			{
				int count = children.Count();
				for (int i = 0; i < count; i++)
				{
					SCR_TreePartV2 treePart = SCR_TreePartV2.Cast(children[i]);
					if (treePart)
						treePart.SwitchPhysicsToStatic(false);
				}
			}
		}
		
		ClearEventMask(EntityEvent.SIMULATE | EntityEvent.POSTSIMULATE);
		
		if (RplSession.Mode() == RplMode.Client)
			return;
		
		float q[4];
		vector mat[4];
		
		GetTransform(mat);
		Math3D.MatrixToQuat(mat, q);
		
		if (m_ParentTree && synchManager)
		{
			synchManager.SynchronizeStaticRotation(m_iTreePartIndex, q, m_ParentTree.GetID());
			synchManager.SynchronizeStaticPosition(m_iTreePartIndex, mat[3], m_ParentTree.GetID());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//Switches physics to dynamic.
	private void SwitchPhysicsToDynamic()
	{
		if (m_Physics && m_Physics.IsDynamic())
			return;
		
		//Go through all children of this tree part and switch them to dynamic physics.
		array<IEntity> children = new array<IEntity>();
		SCR_Global.GetHierarchyEntityList(this, children);
		if (children)
		{
			int count = children.Count();
			for (int i = 0; i < count; i++)
			{
				SCR_TreePartV2 treePart = SCR_TreePartV2.Cast(children[i]);
				if (treePart)
					treePart.SwitchPhysicsToDynamic();
			}
		}
		
		vector mat[4];
		GetTransform(mat);
		
		m_bSwitchToDynamic = false;
		ResetThresholdTime();
		
		if (m_RootsPhysics)
		{
			m_RootsPhysics.Destroy();
			m_RootsPhysics = null;
		}
		
		if (m_Physics)
		{
			//Static physics is present, destroy it.
			if (!m_Physics.IsDynamic())
				DestroyPhysics();
			else
			{
				m_bSwitchToDynamic = false;
				return; //Dynamic physics is present, don't create it again.
			}
		}
		
		m_Physics = Physics.CreateDynamic(this, m_fMass, -1);
		
		if (!m_Physics)
			return;
		
		ApplyPhysicsInfo(this);
		m_vCenterOfMass = m_Physics.GetCenterOfMass();
		
		/*if (RplSession.Mode() == RplMode.Client)
			m_Physics.SetDamping(1, 1);
		else
			CreateJoints();*/
		CreateJoints();
		
		SetEventMask(EntityEvent.SIMULATE | EntityEvent.POSTSIMULATE);
	}
	
	//------------------------------------------------------------------------------------------------
	//Destroys the current physics component.
	private void DestroyPhysics()
	{
		if (m_Physics)
		{
			m_Physics.Destroy();
			m_Physics = null;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//Limits the linear acceleration of this tree part.
	private void LimitAcceleration()
	{
		if (!m_Physics)
			return;
		
		vector velocity = m_Physics.GetVelocity();
		float currentSpeed = velocity.Length();
		
		//Stop if is not moving
		if (currentSpeed != 0)
		{
			//Clamp current speed
			//float targetSpeed = Math.Clamp(currentSpeed, m_fLastSpeedLinear - m_fMaxAccelerationLinear, m_fLastSpeedLinear + m_fMaxAccelerationLinear);
			float targetSpeed = Math.Clamp(currentSpeed, 0, m_fLastSpeedLinear + m_fMaxAccelerationLinear);
			float multiplier = targetSpeed / currentSpeed;
			
			//Apply the limitation
			velocity *= multiplier;
			m_Physics.SetVelocity(velocity);
			m_fLastSpeedLinear = velocity.Length();
		}
		
		vector velocityAngular = m_Physics.GetAngularVelocity();
		vector mat[4];
		GetTransform(mat);
		vector localVelocityAngular = velocityAngular.InvMultiply3(mat);
		for (int i = 0; i < 3; i++)
		{
			float targetVelocityAngular = Math.Clamp(localVelocityAngular[i], 0, m_vLastSpeedAngular[i] + m_vMaxAccelerationAngular[i]);
			localVelocityAngular[i] = targetVelocityAngular;
		}
		
		velocityAngular = localVelocityAngular.Multiply3(mat);
		m_Physics.SetAngularVelocity(velocityAngular);
	}
	
	//------------------------------------------------------------------------------------------------
	//Limit rotation of this tree part around the Y axis.
	private void LimitYRotation()
	{
		if (!m_Physics)
			return;
		if (!m_Physics.IsDynamic())
			return;
		
		vector angularVelocity = m_Physics.GetAngularVelocity();
		//Is the rotation velocity in the world higher than the max velocity along any of the axis?
		if (Math.AbsFloat(angularVelocity[0]) > m_fMaxYRotationVelocity || 
			Math.AbsFloat(angularVelocity[1]) > m_fMaxYRotationVelocity ||
			Math.AbsFloat(angularVelocity[2]) > m_fMaxYRotationVelocity)
		{
			vector mat[4];
			GetTransform(mat);
			float scale = GetScale();
			float matMultiplier = 1;
			//if (scale != 0); //TODO Is this potentially causing a bug?
			matMultiplier /= GetScale();
			
			mat[0] = mat[0] * matMultiplier;
			mat[1] = mat[1] * matMultiplier;
			mat[2] = mat[2] * matMultiplier;
			
			//Move the rotation velocity to local space.
			vector localAngularVelocity = angularVelocity.InvMultiply3(mat);
			float velocity = localAngularVelocity[1];
			
			//Clamp the roration velocity.
			velocity = Math.Clamp(velocity, -m_fMaxYRotationVelocity, m_fMaxYRotationVelocity);
			localAngularVelocity[1] = velocity;
			
			//Move the rotation velocity back to world space and apply it.
			angularVelocity = localAngularVelocity.Multiply3(mat);
		}
		
		//Block the rotation around global Y axis.
		if (!m_bAllowGlobalYRotation)
			angularVelocity[1] = 0;
		
		m_Physics.SetAngularVelocity(angularVelocity);
	}
	
	//------------------------------------------------------------------------------------------------
	//Breaks the given joint, set's all references to this joint to null and destroys it.
	private void BreakJoint(PhysicsJoint joint, SCR_TreePartV2 ent1, SCR_TreePartV2 ent2)
	{
		if (!joint || !ent1 || !ent2)
			return;
		
		ent1.RemoveJoint(joint);
		ent2.RemoveJoint(joint);
		joint.Destroy();
	}
	
	//------------------------------------------------------------------------------------------------
	//Creates all joints for this tree part and all its children.
	private void CreateJoints()
	{
		if (!m_Physics || m_bCreatedJoints)
			return;
		
		//Get the array of children, go through them and create their joints.
		array<IEntity> children = new array<IEntity>();
		GetAllChildren(this, children);
		int count = children.Count();
		
		for (int i = 0; i < count; i++)
		{
			SCR_TreePartV2 treePart = SCR_TreePartV2.Cast(children[i]);
			if (treePart && !treePart.m_ParentJoint)
			{
				PhysicsJoint physicsJoint = treePart.CreateLowerJoint(this);
				if (physicsJoint)
					m_aChildrenJoints.Insert(physicsJoint);
				treePart.CreateJoints();
			}
		}
		m_bCreatedJoints = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//Get all children of the given parent IEntity and store them in array of IEntities allChildren.
	//This only gets children that are directly below this parent in hierarchy, doesn't go any deeper.
	private void GetAllChildren(IEntity parent, notnull inout array<IEntity> allChildren)
	{
		if (!parent)
			return;
		
		IEntity child = parent.GetChildren();
		
		while (child)
		{
			allChildren.Insert(child);
			child = child.GetSibling();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//immediately stops the tree part.
	private void StopSelf()
	{
		if (!m_Physics)
			return;
		
		if (!m_Physics.IsDynamic())
			return;
		
		if (!m_bAllowLinearMovement)
		{
			SetOrigin(m_vLockedOrigin);
			m_Physics.SetVelocity(vector.Zero);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//Checks whether the tree part is faster or slower than the threshold requires.
	//When it is slower, incerement the threshold time.
	private void CheckThreshold(float timeSlice)
	{
		if (GetParent())
			return;
		
		m_fThresholdMaxTime += timeSlice;
		if (m_fThresholdMaxTime > GO_TO_STATIC_TIME_THRESHOLD_MAX)
		{
			if (m_ParentTree && synchManager)
				synchManager.SynchronizeSwitchTreePartToStatic(m_iTreePartIndex, m_ParentTree.GetID());
			m_bSwitchToStatic = true;
		}
		
		if (m_iContactsCount <= 0) //Reset the threshold time if there happend no contacts last simulation step.
		{
			m_bSoundHitGroundPlayed = false;
			m_fThresholdTime = 0;
			return;
		}
		
		if (!m_Physics || m_bSynchronizeHasParent)
			return;
		if (!m_Physics.IsDynamic())
			return;
		
		float velocity = m_Physics.GetVelocity().Length();
		float angularVelocity = m_Physics.GetAngularVelocity().Length();
		if (velocity < GO_TO_STATIC_THRESHOLD && angularVelocity < GO_TO_STATIC_ANGULAR_THRESHOLD)
		{
			m_fThresholdTime += timeSlice;
			if (m_fThresholdTime > GO_TO_STATIC_TIME_THRESHOLD)
			{
				if (m_ParentTree && synchManager)
					synchManager.SynchronizeSwitchTreePartToStatic(m_iTreePartIndex, m_ParentTree.GetID());
				m_bSwitchToStatic = true;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//Creates and returns the parent joint for the tree part.
	PhysicsJoint CreateLowerJoint(SCR_TreePartV2 parent)
	{
		if (!parent || !m_Physics || m_ParentJoint)
			return null;
		
		m_ParentJoint = PhysicsJoint.CreateFixed(parent, this, m_vCenterOfMass * GetScale(), parent.m_vCenterOfMass * parent.GetScale(), true, m_fMaxJointLoad);
		
		return m_ParentJoint;
	}
	
	//------------------------------------------------------------------------------------------------
	void WakeUpParent()
	{
		IEntity parent = GetParent();
		if (!parent)
			WakeUp(false);
		
		SCR_TreePartV2 treePart = SCR_TreePartV2.Cast(parent);
		
		if (treePart)
			treePart.WakeUpParent();
	}
	
	//------------------------------------------------------------------------------------------------
	private bool AddEnt(IEntity ent)
	{
		m_aQuerriedEnts.Insert(ent);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	private bool FilterEnt(IEntity ent)
	{
		if (IsInHierarchy(ent) || ent == this)
			return false;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	private bool IsInHierarchy(IEntity ent)
	{
		if (!ent)
			return false;
		
		array<IEntity> children = new array<IEntity>();
		SCR_Global.GetHierarchyEntityList(this, children);
		int count = children.Count();
		for (int i = 0; i < count; i++)
		{
			if (ent == children[i])
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void WakeUpOthers()
	{
		m_aQuerriedEnts.Clear();
		vector mat[4];
		
		GetTransform(mat);
		GetWorld().QueryEntitiesByOBB(m_vBBOXMin, m_vBBOXMax, mat, AddEnt, FilterEnt);
		
		int count = m_aQuerriedEnts.Count();
		for (int i = 0; i < count; i++)
		{
			if (m_aQuerriedEnts[i])
			{
				SCR_TreePartV2 treePart = SCR_TreePartV2.Cast(m_aQuerriedEnts[i]);//TODO make this for all destructible objects
				if (treePart)
					treePart.WakeUp(true);
			}
			else break;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void WakeUp(bool wakeUpParent)
	{
 		if (!m_Physics)
			return;
		
		if (wakeUpParent)
		{
			WakeUpParent();
			return;
		}
		
		m_bWakeUp = true;
	}
	
	//------------------------------------------------------------------------------------------------
	void WakeUpHierarchy()
	{
		SwitchPhysicsToDynamic();
		
		//Generate all joints for this tree part and it's children.
		CreateJoints();
		
		m_bWakeUp = false;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetTreePartIndex()
	{
		return m_iTreePartIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateTransform(vector position, float quat[4], vector velocityLinear, vector velocityAngular)
	{
		m_bExtrapolate = true;
		
		m_vNetPosition = position;
		m_vNetVelocityLinear = velocityLinear;
		m_vNetVelocityAngular = velocityAngular;
		Math3D.QuatCopy(quat, m_fNetRotation);
		
		if (m_Physics)
		{
			if (m_Physics.IsDynamic())
			{
				m_Physics.SetVelocity(velocityLinear);
				m_Physics.SetAngularVelocity(velocityAngular * Math.DEG2RAD);
			}
			else
			{
				vector mat[4];
				Math3D.QuatToMatrix(quat, mat);
				
//				mat *= GetScale();
				mat[3] = position;
				
				SetTransform(mat);
			}
		}
		
		m_fTimeSinceLastTick = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	void CacheRotation(float quat[4])
	{
//		Print("Caching rotation");
//		Print(quat);
		m_bApplyCachedRotation = true;
		Math3D.QuatCopy(quat, m_fCachedRotation);
	}
	
	//------------------------------------------------------------------------------------------------
	void CachePosition(vector pos)
	{
//		Print("Caching " + pos);
		m_bApplyCachedPosition = true;
		m_vCachedPosition[0] = pos[0];
		m_vCachedPosition[1] = pos[1];
		m_vCachedPosition[2] = pos[2];
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRotation(float quat[4])
	{
		vector mat[4];
		Math3D.QuatToMatrix(quat, mat);
//		mat *= GetScale();
		mat[3] = GetOrigin();
		
		SetTransform(mat);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPosition(vector pos)
	{
//		Print(pos);
		SetOrigin(pos);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetVelocity(vector velocity)
	{
//		Print(velocity);
		if (velocity.Length() < 0.02)
			return;
//		Print(velocity);
		if (m_Physics)
			m_Physics.SetVelocity(velocity);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAngularVelocity(vector angularVelocity)
	{
//		Print(angularVelocity);
		if (angularVelocity.Length() < 0.02)
			return;
//		Print(angularVelocity);
		if (m_Physics)
			m_Physics.SetAngularVelocity(angularVelocity);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPhysics(bool dynamic)
	{
		if (dynamic)
			m_bSwitchToDynamic = true;
		else 
			m_bSwitchToStatic = true;
	}
	
	//------------------------------------------------------------------------------------------------
	void ClientSetToBreakFromParent()
	{
		m_bBreakFromParent = true;
		
		if (m_Physics && !m_Physics.IsDynamic())
		{
			IEntity parent = GetParent();
			if (parent)
			{
				SCR_TreePartV2 treePart = SCR_TreePartV2.Cast(parent);
				
				parent.RemoveChild(this);
			}
			
//			Print(m_vCachedPosition);
//			Print(m_bApplyCachedPosition);
			if (m_vCachedPosition && m_bApplyCachedPosition)
			{
//				Print(m_vCachedPosition);
				SetPosition(m_vCachedPosition);
				m_bApplyCachedPosition = false;
			}
			
//			Print(m_fCachedRotation);
//			Print(m_bApplyCachedRotation);
			if (m_fCachedRotation && m_bApplyCachedRotation)
			{
//				Print(m_fCachedRotation);
				SetRotation(m_fCachedRotation);
				m_bApplyCachedRotation = false;
			}
			
			m_bBreakFromParent = false;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	float GetQuatDiff(float quat1[4], float quat2[4])
	{
		float diff0 = Math.AbsFloat(quat1[0] - quat2[0]);
		float diff1 = Math.AbsFloat(quat1[1] - quat2[1]);
		float diff2 = Math.AbsFloat(quat1[2] - quat2[2]);
		float diff3 = Math.AbsFloat(quat1[3] - quat2[3]);
		float diffSum = diff0 + diff1 + diff2 + diff3;
		
		return diffSum;
	}
	
	//------------------------------------------------------------------------------------------------
	void PrintDamageDebug(float impulse, EDamageType damageType)
	{
		float reducedDamage = -1;
		switch (damageType)
		{
			case EDamageType.MELEE:
				if (m_bDebugDamage)
					Print("Melee");
				if (m_fMeleeResistance != 0)
					reducedDamage = impulse / m_fMeleeResistance;
				break;
			case EDamageType.KINETIC:
				if (m_bDebugDamage)
					Print("Kinetic");
				if (m_fKineticResistance != 0)
					reducedDamage = impulse / m_fKineticResistance;
				break;
			case EDamageType.INCENDIARY:
				if (m_bDebugDamage)
					Print("Incendiary");
				break;
			case EDamageType.EXPLOSIVE:
				if (m_bDebugDamage)
					Print("Explosion");
				if (m_fExplosionResistance != 0)
					reducedDamage = impulse / m_fExplosionResistance;
				break;
			case EDamageType.FIRE:
				if (m_bDebugDamage)
					Print("Fire");
					Print("Damage after reduction: " + "NOT IMPLEMENTED YET");
				break;
			case EDamageType.COLLISION:
				if (m_bDebugDamage)
					Print("Collision");
				if (m_fCollisionResistance != 0)
					reducedDamage = impulse / m_fCollisionResistance;
				break;
		}
		
		if (m_bDebugDamage)
			Print("Damage before reduction: " + impulse);
		if (reducedDamage != -1)
			Print("Damage after reduction: " + reducedDamage);
		else Print("Damage after reduction: NOT IMPLEMENTED YET");
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetThresholdTime()
	{
		m_fThresholdTime = 0;
		m_fThresholdMaxTime = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity FindSuperParent()
	{
		IEntity current = this;
		while (current.GetParent())
		{
			current = current.GetParent();
		}
		
		return current;
	}
	
	//------------------------------------------------------------------------------------------------
	bool WouldBreak(float impulse, EDamageType damageType)
	{
		switch (damageType)
		{
			case EDamageType.MELEE:
				if (m_fMeleeResistance != 0)
					impulse /= m_fMeleeResistance;
				break;
			case EDamageType.KINETIC:
				if (m_fKineticResistance != 0)
					impulse /= m_fKineticResistance;
				break;
			case EDamageType.INCENDIARY:
				break;
			case EDamageType.EXPLOSIVE:
				if (m_fExplosionResistance != 0)
					impulse /= m_fExplosionResistance;
				break;
			case EDamageType.FIRE:
				break;
			case EDamageType.COLLISION:
				if (m_fCollisionResistance != 0)
					impulse /= m_fCollisionResistance;
				break;
		}
		
		if (impulse > (m_fMinImpact * GetScale()))
			return true;
		else 
			return false;
	}
	
	//------------------------------------------------------------------------------------------------
	bool WouldBreak(float impulse, EDamageType damageType, out float reducedImpulse)
	{
		reducedImpulse = impulse;
		
		switch (damageType)
		{
			case EDamageType.MELEE:
				if (m_fMeleeResistance != 0)
					reducedImpulse /= m_fMeleeResistance;
				break;
			case EDamageType.KINETIC:
				if (m_fKineticResistance != 0)
					reducedImpulse /= m_fKineticResistance;
				break;
			case EDamageType.INCENDIARY:
				break;
			case EDamageType.EXPLOSIVE:
				if (m_fExplosionResistance != 0)
					reducedImpulse /= m_fExplosionResistance;
				break;
			case EDamageType.FIRE:
				break;
			case EDamageType.COLLISION:
				if (m_fCollisionResistance != 0)
					reducedImpulse /= m_fCollisionResistance;
				break;
		}
		
		if (reducedImpulse > (m_fMinImpact * GetScale()))
			return true;
		else 
			return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDamage(float damage,
				  EDamageType type,
				  IEntity pHitEntity, 
				  inout vector outMat[3], 
				  IEntity damageSource, 
				  notnull Instigator instigator, 
				  int colliderID, 
				  float speed)
	{
		if (RplSession.Mode() == RplMode.Client)
			return;
		
		//Calculate direction vector based on normal of the hit
		vector directionVector = -outMat[2];
		directionVector.Normalize();
		
		if (m_bDebugDamage)
			PrintDamageDebug(damage, type);
		//If the damage / impulse was big enough to move the tree part
		if (damage > m_fMinImpact)
		{
			//Calculate impulse vector from direction vector and damage
			vector positionVector = outMat[0];
			
			if (type == EDamageType.EXPLOSIVE)
			{
				positionVector = "0 0 0";
				directionVector = (GetOrigin() - outMat[0]);
			}
			
			directionVector.Normalize();
			vector impulseVector = directionVector * (damage / 10); //TODO REMOVE THIS CONSTANT VALUE
			
			if (WouldBreak(damage, type))
				SetToBreak(positionVector: positionVector, impulseVector: impulseVector, damageType: type);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//Handles methods that change stuff in physics world that cannot be done in simulate or contact methods.
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (m_bBreak) //Break this tree part if necessary.
			Break();
		if (m_bWakeUp && !owner.GetParent())
			WakeUpHierarchy();
		if (m_bSwitchToDynamic) //Switch physics to dynamic if necessary.
			SwitchPhysicsToDynamic();
		if (m_bSwitchToStatic) //Switch physics to static if necessary.
		{
			SwitchPhysicsToStatic(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// TODO move extrapolation into generic / global methods
	// GenericEntity entity is the entity you want to be affected by extrapolation.
	// Physics physics is the physics that the extrapolation should calculate with.
	// vector netPosition is the last received position.
	// vector netVelocity is the last received velocity.
	// float netTeleportDistance is the max distance between position and netPosition, anything over this causes the entity to teleport.
	// float netRotation[4] is the last received rotation.
	// vector netVelocityAngular is the last received angular velocity.
	// float netTeleportAng is the max angle between current rotation and replicated rotation, anything over this causes the entity to teleport.
	// float timeSinceLastTick is the time since last synchronization of extrapolation relevant data was received, it should already be incremented by timeSlice by you!
	// float timeSlice is the time since last frame / simulation step.
	static void Extrapolate(GenericEntity entity, Physics physics, vector netPosition, vector netVelocityLinear, float netTeleportDistance, float netRotation[4], vector netVelocityAngular, float netTeleportAng, float timeSinceLastTick, float timeSlice, float tickTime)
	{
		float scale = entity.GetScale();
		vector currentMatrix[4];
		entity.GetWorldTransform(currentMatrix);
		
		// Lerp to positions/rotations received
		vector position = currentMatrix[3];
		float rotation[4];
		Math3D.MatrixToQuat(currentMatrix, rotation);
		
		// Static object, ensure exact rotation/position
		if (!physics || !physics.IsDynamic())
		{
			if (rotation != netRotation)
				Math3D.QuatToMatrix(netRotation, currentMatrix);
			
//			currentMatrix *= entity.GetScale();
			
			currentMatrix[3] = netPosition;
			
			entity.SetWorldTransform(currentMatrix);
			entity.SetScale(scale);
			return;
		}
		
		// Dynamic object, so calculate projected position/rotation based on last tick
		vector projectedPos = netPosition + netVelocityLinear * timeSinceLastTick;
		
		netVelocityAngular = netVelocityAngular * timeSinceLastTick;
		vector netVelocityAngularFlipped = GetFixedAxisVector(netVelocityAngular);
		float projectedRotation[4];
		float netVelocityAngularQuat[4];
		netVelocityAngularFlipped.QuatFromAngles(netVelocityAngularQuat);
		Math3D.QuatMultiply(projectedRotation, netRotation, netVelocityAngularQuat);
		
		// Calculate the position and rotation error
		float posError = vector.Distance(projectedPos, position);
		float rotError = Math3D.QuatAngle(projectedRotation, rotation);
		
		// If too far off position, teleport
		//if (posError > netTeleportDistance && posError > netVelocityLinear.Length() * tickTime * 2)
		if (posError > netTeleportDistance)
		{
			entity.SetOrigin(netPosition);
			posError = 0;
		}
		
		// If too far off rotation, teleport
		//if (rotError > netTeleportAng && rotError > netVelocityAngular.Length() * tickTime * 2)
		if (rotError > netTeleportAng)
		{
			Math3D.QuatToMatrix(netRotation, currentMatrix);
//			currentMatrix *= entity.GetScale();
			currentMatrix[3] = entity.GetOrigin();
			entity.SetWorldTransform(currentMatrix);
			rotError = 0.0;
		}
		
		float timeStep = Math.Clamp(timeSlice * 2, 0, 1);
		float timeStepTick = Math.Clamp(timeSlice / tickTime, 0, 1);
		
		// Adjust to account for errors in position/rotation
		if (posError > 0.01)
		{
			entity.SetOrigin(MoveTowards(position, projectedPos, posError * timeStep));
			physics.SetVelocity(physics.GetVelocity() + (projectedPos - position) * timeStepTick);
		}
		
		if (rotError > 0.01)
		{
			float outRot[4];
			Math3D.QuatRotateTowards(outRot, rotation, projectedRotation, (rotError * timeStep) * Math.RAD2DEG);
			Math3D.QuatToMatrix(outRot, currentMatrix);
			//currentMatrix *= entity.GetScale();
			currentMatrix[3] = entity.GetOrigin();
//			Print(currentMatrix);
			entity.SetWorldTransform(currentMatrix);
			
			float rotDiff[4];
			float rotInv[4];
			Math3D.QuatInverse(rotInv, rotation);
			Math3D.QuatMultiply(rotDiff, projectedRotation, rotInv);
			vector angularVelocity = Math3D.QuatToAngles(rotDiff);
			angularVelocity = FixEulerVector180(angularVelocity) * Math.DEG2RAD * timeStepTick;
			angularVelocity += physics.GetAngularVelocity() * Math.DEG2RAD;
			physics.SetAngularVelocity(angularVelocity);
		}
		
		entity.SetScale(scale);
	}
	
	//------------------------------------------------------------------------------------------------
	// Flips X and Y axis of the vector.
	static vector GetFixedAxisVector(vector toFlip)
	{
		vector flipped;
		flipped[0] = toFlip[1];
		flipped[1] = toFlip[0];
		flipped[2] = toFlip[2];
		return flipped;
	}
	
	//------------------------------------------------------------------------------------------------
	static void RotateTowards(out float result[4], float from[4], float to[4], float maxDegreesDelta)
    {
		float num = Math3D.QuatAngle(from, to);
		if (float.AlmostEqual(num, 0.0))
		{
			Math3D.QuatCopy(to, result);
			return;
		}
		float t = Math.Min(1, maxDegreesDelta / num);
		Math3D.QuatLerp(result, from, to, t);
    }
	
	//------------------------------------------------------------------------------------------------
	// Moves a point start in a straight line towards a target point.
	static vector MoveTowards(vector start, vector target, float maxDistanceDelta)
	{
		vector diff = target - start;
		float magnitude = diff.Length();
		if (magnitude <= maxDistanceDelta || float.AlmostEqual(magnitude, 0.0))
			return target;
		return start + diff / magnitude * maxDistanceDelta;
	}
	
	//----------------------------------------------------------------------------------------------------
	// Ensures the angles are in range <-180; 180>
	static vector FixEulerVector180(vector angles)
	{
		// Goes through each member of the vector and fixes it if necessary
		for (int a = 0; a < 3; a++)
		{
			while (angles[a] < -180)
				angles[a] = angles[a] + 360;
			while (angles[a] > 180)
				angles[a] = angles[a] - 360;
		}
		
		return angles;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnPhysicsActive(IEntity owner, bool activeState)
	{
		if (!activeState)
		{
			SetEventMask(EntityEvent.FRAME);
			m_bSwitchToStatic = true;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (GetParent())
			m_bSynchronizeHasParent = true;
		
		GetBounds(m_vBBOXMin, m_vBBOXMax);
		
		SwitchPhysicsToStatic(false);
		SetEventMask(EntityEvent.FRAME | EntityEvent.PHYSICSACTIVE);
	}
	
	//------------------------------------------------------------------------------------------------
	//Only called when dynamic.
	override void EOnSimulate(IEntity owner, float timeSlice)
	{
		if (!m_Physics)
			return;
		
		//TODO: Move this to method & try to optimize & make sure mat scale is fine
		if (m_bBreakFromParent)
		{
			IEntity parent = GetParent();
			if (parent)
			{
				SCR_TreePartV2 treePart = SCR_TreePartV2.Cast(parent);
				if (treePart && m_ParentJoint)
					BreakJoint(m_ParentJoint, this, treePart);
				
				parent.RemoveChild(this);
			}
			
			if (m_vCachedPosition && m_bApplyCachedPosition)
			{
				SetPosition(m_vCachedPosition);
				m_bApplyCachedPosition = false;
			}
			
			if (m_fCachedRotation && m_bApplyCachedRotation)
			{
				SetRotation(m_fCachedRotation);
				m_bApplyCachedRotation = false;
			}
			
			m_bBreakFromParent = false;
		}
		
		StopSelf();
		LimitYRotation();
		
		if (m_Physics.IsDynamic() && RplSession.Mode() == RplMode.Client)
		{
/*			m_Physics.SetVelocity(vector.Zero);
			m_Physics.SetAngularVelocity(vector.Zero);*/
			return;
		}
		
		CheckThreshold(timeSlice);
		
		m_iContactsCount = 0; //Reset contanct count for next simulation step.
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnPostSimulate(IEntity owner, float timeSlice)
	{
		if (RplSession.Mode() == RplMode.Client)
		{
			if (m_bExtrapolate)
			{
				m_fTimeSinceLastTick += timeSlice;
				Extrapolate(this, m_Physics, m_vNetPosition, m_vNetVelocityLinear, NET_TELEPORT_DISTANCE, m_fNetRotation, m_vNetVelocityAngular, NET_TELEPORT_ANGLE, m_fTimeSinceLastTick, timeSlice, m_fTargetSynchTime);
			}
			return;
		}
		
		/*if (!this.GetParent() && synchManager)
		{
			float q[4];
			vector mat[4];
			
			GetWorldTransform(mat);
			Math3D.MatrixToQuat(mat, q);
			
			if (m_ParentTree)
			{
				vector parentOrigin = m_ParentTree.GetOrigin();
				
				if (m_Physics)
				{
					synchManager.SynchronizeTreePartTransform(m_iTreePartIndex, q, GetOrigin(), m_Physics.GetVelocity(), m_Physics.GetAngularVelocity(), m_ParentTree.GetID());
				}
			}
		}*/
		
		m_fSynchTime += timeSlice;
		if (m_fSynchTime > m_fTargetSynchTime)
		{
			if (!this.GetParent() && synchManager)
			{
				float q[4];
				vector mat[4];
				
				GetWorldTransform(mat);
				Math3D.MatrixToQuat(mat, q);
				
				if (m_ParentTree)
				{
					vector parentOrigin = m_ParentTree.GetOrigin();
					
					if (m_Physics)
					{
						synchManager.SynchronizeTreePartTransform(m_iTreePartIndex, q, GetOrigin(), m_Physics.GetVelocity(), m_Physics.GetAngularVelocity(), m_ParentTree.GetID());
					}
				}
			}
			m_fSynchTime = 0;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		if (RplSession.Mode() == RplMode.Client)
			return;
		
		// SOUND
		if (m_bSoundHitGroundPlayed) // ensure that SOUND_HIT_GROUND only plays once per tree part
			return;
		
		if (GenericTerrainEntity.Cast(other)) // check if contacted entity is ground
		{
			float vNormBefore = contact.GetRelativeNormalVelocityBefore();
			float vNormAfter = contact.GetRelativeNormalVelocityAfter();
			float dV = vNormAfter - vNormBefore;
			float impulse = contact.Impulse;
			
			if (dV > 2.5 && impulse > 100) // check if impact above a certain threshold
			{
				if (m_ParentTree)
				{
					m_ParentTree.OnTreePartHitGround();
				}
				m_bSoundHitGroundPlayed = true;
			}
		}
		
		if (Vehicle.Cast(other))
		{
			auto superParent = SCR_TreePartV2.Cast(FindSuperParent());
			if (superParent)
				superParent.ResetThresholdTime();
		}
		
		m_iContactsCount++;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_TreePartV2(IEntitySource src, IEntity parent)
	{
		if (TARGET_RPC_COUNT > 0 && m_fTargetSynchTime == -1)
		{
			m_fTargetSynchTime = 1 / TARGET_RPC_COUNT;
		}
		SetEventMask(EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_TreePartV2()
	{
		m_Physics = null;
		m_ParentJoint = null;
		m_HybridPhysicsInfo = null;
		m_aChildrenJoints = null;
		m_aQuerriedEnts = null;
	}
#endif
};