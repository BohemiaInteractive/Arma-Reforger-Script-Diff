#define ENABLE_BASE_DESTRUCTION

[ComponentEditorProps(category: "GameScripted")]
class SCR_DestructionTireInitialDamageDataComponentClass: ScriptComponentClass
{
	[Attribute("", UIWidgets.Object, "Initial damage phase data", category: "Destruction Multi-Phase")]
	ref SCR_DamagePhaseData InitialData;	
}

[ComponentEditorProps(category: "GameScripted/Destruction", description: "Tire destruction component, for destruction and deformation of tires")]
class SCR_DestructionTireComponentClass: SCR_DestructionMultiPhaseComponentClass
{
}

//! Tire destruction component, for destruction and deformation of tires
class SCR_DestructionTireComponent : SCR_DestructionMultiPhaseComponent
{
	protected static const int MAX_BONES = 36;

	//
	// Category: Tire Destruction
	//

	[Attribute("", UIWidgets.EditBox, "List of phases in which the tire should handle deformation (this also means deflating)", category: "Tire Destruction")]
	protected ref array<int> m_DeformPhaseIndexes;

	[Attribute("0.15", UIWidgets.Slider, "Rate at which the tire deflates when in a tire deformation damage phase (in %/s)", "0.01 1000 0.01", category: "Tire Destruction")]
	protected float m_fDeflationRate;

	[Attribute("Tyre_", UIWidgets.EditBox, "Name prefix (followed then by index) of the tire deformation joints", category: "Tire Destruction")]
	protected string m_sTireJointNamePrefix;

	[Attribute("1", UIWidgets.CheckBox, "If true, tire damage and destruction will affect vehicle physics (if attached to such)", category: "Tire Destruction")]
	protected bool m_bAffectPhysics;
	
	//
	// Category: Tire Deformation Simple
	//

	[Attribute("1", UIWidgets.Slider, "Ground deformation of the tire (in %)", "0 1 0.01", category: "Tire Deformation Simple")]
	protected float m_fGroundSlumpScale;

	[Attribute("1", UIWidgets.Slider, "Radial deformation of the tire (in %)", "0 1 0.01", category: "Tire Deformation Simple")]
	protected float m_fRadiusScale;

	[Attribute("1", UIWidgets.Slider, "Width deformation of the tire (in %)", "0 1 0.01", category: "Tire Deformation Simple")]
	protected float m_fTireWidthScale;

	[Attribute("1", UIWidgets.Slider, "Sideward skew deformation of the tire (in %)", "0 1 0.01", category: "Tire Deformation Simple")]
	protected float m_fTireSideSkewScale;
	
	//
	// Category: Tire Deformation Advanced
	//

	[Attribute("0.75", UIWidgets.Slider, "Ground contact width scale addition (in %) - [Scaled by m_fGroundSlumpScale]", "-2 2 0.01", category: "Tire Deformation Advanced")]
	protected float m_fDeformParam_ScaleAdd_SquishWidthGround;

	[Attribute("-0.3", UIWidgets.Slider, "Opposite to ground width scale addition (in %) - [Scaled by m_fTireWidthScale]", "-2 2 0.01", category: "Tire Deformation Advanced")]
	protected float m_fDeformParam_ScaleAdd_SquishWidthTop;

	[Attribute("0.25", UIWidgets.Slider, "Flatten tire top offset (in m) - [Scaled by m_fRadiusScale]", "-10 10 0.01", category: "Tire Deformation Advanced")]
	protected float m_fDeformParam_Offset_FlattenTop;

	[Attribute("0.25", UIWidgets.Slider, "Outward slump of tire on front/back of ground contact point (in m) - [Scaled by m_fGroundSlumpScale]", "-10 10 0.01", category: "Tire Deformation Advanced")]
	protected float m_fDeformParam_Offset_GroundSlump;

	[Attribute("0.1", UIWidgets.Slider, "Outward extrusion near ground contact point (in m)", "-10 10 0.01", category: "Tire Deformation Advanced")]
	protected float m_fDeformParam_Offset_ExtrudeBottomOutward;

	[Attribute("0.35", UIWidgets.Slider, "Upward extrusion near ground contact point (in m)", "-10 10 0.01", category: "Tire Deformation Advanced")]
	protected float m_fDeformParam_Offset_ExtrudeBottomUpward;

	[Attribute("0.1", UIWidgets.Slider, "Downward extrusion of top opposite ground contact point (in m)", "-10 10 0.01", category: "Tire Deformation Advanced")]
	protected float m_fDeformParam_Offset_ExtrudeTopDownward;

	[Attribute("0.01", UIWidgets.Slider, "Sideward skew extrusion all around the tire (in m) - [Scaled by m_fTireSideSkewScale]", "-10 10 0.01", category: "Tire Deformation Advanced")]
	protected float m_fDeformParam_Offset_ExtrudeSidewardSkew;
	
#ifdef ENABLE_BASE_DESTRUCTION
	protected bool m_bHandleDeformation;
	protected float m_fInflation = 1;
	protected float m_fWheelRadius = -1;
	protected ref array<int> m_WheelBones = {};
	protected ref SCR_DamagePhaseData m_InitialData;
	
	protected VehicleWheeledSimulation m_VehicleWheeledSimulation;

	protected int m_iWheelIndex = -1;
	
	//------------------------------------------------------------------------------------------------
	//! Sets the wheel index for the tire and updates it's radius
	//! \param[in] wheelIndex
	void SetWheelIndex(int wheelIndex)
	{
		m_iWheelIndex = wheelIndex;
		
		if (m_iWheelIndex > -1 && m_VehicleWheeledSimulation && m_iWheelIndex < m_VehicleWheeledSimulation.WheelCount())
			m_fWheelRadius = m_VehicleWheeledSimulation.WheelGetRadius(m_iWheelIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] damagePhase
	//! \return whether deformation should be handled in the input damage
	bool GetShouldHandleDeformation(int damagePhase)
	{
		return m_DeformPhaseIndexes && m_DeformPhaseIndexes.Find(damagePhase) != -1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Enables/disables handling of tire deformation and deflation
	//! \param[in] enable
	void EnableTireDeformation(bool enable)
	{
#ifdef ENABLE_DEFORMATION
		m_bHandleDeformation = enable;
#endif
		
		m_WheelBones.Clear();
		if (!m_bHandleDeformation)
			return;
		
		for (int i = 0; i < MAX_BONES; i++)
		{
			int boneIndex = GetOwner().GetAnimation().GetBoneIndex(m_sTireJointNamePrefix + i.ToString());
			if (boneIndex > -1)
				m_WheelBones.Insert(boneIndex);
		}
	}
	
	protected float groundContactPct1 = 1;

	//------------------------------------------------------------------------------------------------
	override void OnFrame(IEntity owner, float timeSlice)
	{
		super.OnFrame(owner, timeSlice);
		
		if (!m_bHandleDeformation)
			return;
		
		// Update the physics wheel radius
		if (m_VehicleWheeledSimulation && m_fWheelRadius > 0)
		{
			float targetWheelRadius = (0.5 + m_fInflation * 0.5) * m_fWheelRadius;
			if (targetWheelRadius != m_VehicleWheeledSimulation.WheelGetRadius(m_iWheelIndex))
			{
				m_VehicleWheeledSimulation.WheelSetRadiusState(m_iWheelIndex, targetWheelRadius);
				
				Physics physics = null;
				IEntity parent = owner.GetParent();
				if (parent)
				{
					physics = parent.GetPhysics();

					if (physics && !physics.IsActive())
						physics.SetActive(ActiveState.ACTIVE);
				}
			}
		}
		
		// Handle deformation and deflation of the tire
		vector floorNorm = vector.Up;
		vector wheelMat[4];
		owner.GetTransform(wheelMat);
		
		m_fInflation = Math.Clamp(timeSlice * -m_fDeflationRate + m_fInflation, 0, 1);
		
		float deflationPct = 1 - Math.Pow(1 - Math.Clamp((1 - m_fInflation) * 2, 0, 1), 1.5);
		float deflationExpPct = (1 - Math.Clamp(m_fInflation * 1.7, 0, 1));
		deflationExpPct += (1 - deflationExpPct) * deflationExpPct;
		deflationExpPct *= deflationExpPct;
		float deflationExpPct2 = (1 - Math.Clamp(m_fInflation * 1.1, 0, 1));
		deflationExpPct2 += (1 - deflationExpPct2) * deflationExpPct2;
		deflationExpPct2 *= deflationExpPct2;
		groundContactPct1 = Math.Clamp(groundContactPct1 + (Debug.KeyState(KeyCode.KC_RIGHT) - Debug.KeyState(KeyCode.KC_LEFT)) * timeSlice, 0, 1);
		float groundContactPct = groundContactPct1;
		
		// Apply deformation
		vector boneMatLocal[4];
		int numBones = m_WheelBones.Count();
		for (int i = 0; i < numBones; i++)
		{
			int boneIndex = m_WheelBones.Get(i);
			owner.GetAnimation().GetBoneMatrix(boneIndex, boneMatLocal);
			vector boneUp = boneMatLocal[2].Normalized();
			boneUp = boneUp.Multiply3(wheelMat);
			float boneFloorDot = Math.Clamp(floorNorm * boneUp, 0, 1);
			float boneTopDot = 1 - boneFloorDot;
			float tyreBottom = Math.Pow(boneFloorDot, 2);
			float tyreTop = Math.Pow(boneTopDot, 2);
			
			float widthScale = 1;
			widthScale += tyreBottom * deflationPct * m_fDeformParam_ScaleAdd_SquishWidthGround * m_fGroundSlumpScale * groundContactPct; // Squish out the tyre at the floor
			widthScale += tyreTop * deflationExpPct * m_fDeformParam_ScaleAdd_SquishWidthTop * m_fTireWidthScale * groundContactPct; // Thin out the tyre above
			widthScale += deflationExpPct * m_fDeformParam_ScaleAdd_SquishWidthTop * m_fTireWidthScale * (1 - groundContactPct); // Thin out the tyre above
			
			float heightOffset = 0;
			heightOffset += boneTopDot * deflationExpPct2 * m_fDeformParam_Offset_FlattenTop * m_fRadiusScale * tyreTop; // Flatten top of the tyre
			heightOffset += boneTopDot * deflationExpPct * m_fDeformParam_Offset_GroundSlump * m_fGroundSlumpScale * (1 - tyreTop); // Flatten all but bottom and top of the tyre
			heightOffset += Math.Sin(tyreBottom * 180 * Math.DEG2RAD) * deflationPct * -m_fDeformParam_Offset_ExtrudeBottomOutward; // Offset bottom-sides outward
			
			float squishHeightOffset = 0;
			squishHeightOffset += tyreBottom * deflationPct * m_fDeformParam_Offset_ExtrudeBottomUpward * groundContactPct; // Offset bottom upward
			squishHeightOffset += tyreTop * deflationPct * m_fDeformParam_Offset_ExtrudeTopDownward * (1 - groundContactPct); // Offset top downward
			
			int sidePct = (i % 3);
			float sideOffset = (-1 + (float)sidePct * 2) * deflationExpPct * m_fDeformParam_Offset_ExtrudeSidewardSkew * m_fTireSideSkewScale; // Offset based on joint num
			
			vector localBoneMat[4];
			Math3D.MatrixIdentity4(localBoneMat);
			
			localBoneMat[0] = vector.Right * widthScale;
			localBoneMat[3] = vector.Forward * (heightOffset + squishHeightOffset) + vector.Right * sideOffset;
			
			owner.GetAnimation().SetBoneMatrix(owner, boneIndex, localBoneMat);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] pData
	void ReturnToInitialDamagePhase(SCR_DamagePhaseData pData)
	{
		if (GetDamagePhase() != 0)
			SetDamagePhase(0);
		
		if (GetTargetDamagePhase() != 0)
			SetTargetDamagePhase(0);
	
		ScriptCallQueue callQueue = GetGame().GetCallqueue();
		if (!callQueue || callQueue.GetRemainingTime(ChangeModel) > -1)
			return;

		callQueue.CallLater(ChangeModel, param1: pData.m_PhaseModel);
	}

	//------------------------------------------------------------------------------------------------
	//! Switches to the input damage phase (or deletes if past last phase)
	//! \param[in] damagePhase
	//! \param[in] delayMeshChange
	override void GoToDamagePhase(int damagePhase, bool delayMeshChange)
	{
		if (damagePhase == 0)
			ReturnToInitialDamagePhase(m_InitialData);
		else
			super.GoToDamagePhase(damagePhase, delayMeshChange);
		
		EnableTireDeformation(GetShouldHandleDeformation(GetDamagePhase()));
//		if (m_bHandleDeformation)
//			EnableOnFrame(true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (!owner)
			return;
		
		EntityComponentPrefabData prefabData = owner.FindComponentData(SCR_DestructionTireInitialDamageDataComponentClass);
		if (prefabData)
		{
			SCR_DestructionTireInitialDamageDataComponentClass data = SCR_DestructionTireInitialDamageDataComponentClass.Cast(prefabData);
			if (data)
				m_InitialData = data.InitialData;
		}
		
		EnableTireDeformation(GetShouldHandleDeformation(GetDamagePhase()));
		
		IEntity parent = owner.GetParent();
		
		if (parent)
		{
			m_VehicleWheeledSimulation = VehicleWheeledSimulation.Cast(parent.FindComponent(VehicleWheeledSimulation));
		}
	}
#endif
}
