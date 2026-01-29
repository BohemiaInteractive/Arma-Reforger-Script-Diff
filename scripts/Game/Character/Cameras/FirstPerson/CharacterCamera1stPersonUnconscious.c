// *************************************************************************************
// ! CharacterCamera1stPersonUnconscious - first person only unconscious
// *************************************************************************************
class CharacterCamera1stPersonUnconscious extends CharacterCamera1stPerson
{
	void CharacterCamera1stPersonUnconscious(CameraHandlerComponent pCameraHandler)
	{
	}
	
	void PrintMatrix(string Name, const vector InMatrix[4])
	{

		Print(string.Format("%1 %2 %3 %4 %5", Name, InMatrix[0], InMatrix[1], InMatrix[2], InMatrix[3]), LogLevel.ERROR);
	}

	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		// Note: The transformation is discarded (overwritten) later on
		super.OnUpdate(pDt, pOutResult);

		pOutResult.m_vBaseAngles 		= "0 0 0";
		pOutResult.m_fUseHeading 		= 0.0;
		pOutResult.m_iDirectBoneMode 	= EDirectBoneMode.RelativeTransform;
		
		// TODO@AS: For now always use the camera bone. If transition is animated,
		// it is safe to revert to using GetCameraBoneIndex(), assuming sm_TagFPCamera is set.
		pOutResult.m_iDirectBone 		= sm_iCameraBoneIndex;
		
		vector animationBoneWorldTM [4];
		int headBoneIndex = m_OwnerCharacter.GetAnimation().GetBoneIndex("Head");
		m_OwnerCharacter.GetAnimation().GetBoneMatrix(headBoneIndex, animationBoneWorldTM);

		vector charMat[4];
		m_OwnerCharacter.GetWorldTransform(charMat);

		Math3D.MatrixMultiply4(charMat, animationBoneWorldTM, animationBoneWorldTM);

		// Head bone in model space is rotated 180 around its yaw axis,
		// so we simply undo this rotation
		vector lookAngles = m_CharacterHeadAimingComponent.GetLookAngles();
		float y = lookAngles[0] - 180.0;

		while (y < -180)
			y += 360;
		while (y > 180)
			y -= 360;

		lookAngles[0] = y;

		if (m_OwnerCharacter.GetAnimationComponent().IsRagdollActive())
		{
			// Apply lookat angles.
			Math3D.AnglesToMatrix(lookAngles, pOutResult.m_CameraTM);

			//NOTE: 2 is head bone
			m_OwnerCharacter.GetBoneMatrix(headBoneIndex, m_ragdollHeadWorldTR);

			Math3D.MatrixMultiply3(m_ragdollHeadWorldTR, pOutResult.m_CameraTM, m_ragdollHeadWorldTR);

			//apply offset translation
			pOutResult.m_CameraTM[3] = "0 0 0";

			Math3D.MatrixMultiply4(charMat, m_ragdollHeadWorldTR, m_ragdollHeadWorldTR);
			Math3D.MatrixToQuat(m_ragdollHeadWorldTR, m_ragdollHeadQuatWS);

			m_fCurrentEaseOutRagdollSmoothTime = m_fEaseOutRagdollSmoothTime;
		}
		else if (m_fCurrentEaseOutRagdollSmoothTime > 0.0 && m_ragdollHeadWorldTR[0] != vector.Zero && m_ragdollHeadWorldTR[1] != vector.Zero && m_ragdollHeadWorldTR[2] != vector.Zero)
		{
			Math3D.AnglesToMatrix(lookAngles, pOutResult.m_CameraTM);

			//calculate the lerp step
			m_fCurrentEaseOutRagdollSmoothTime -= pDt;
			float step = 1.0 - (m_fCurrentEaseOutRagdollSmoothTime / m_fEaseOutRagdollSmoothTime);
			step = Math.Clamp(step, 0.0, 1.0);

			//transform pOutResult.m_CameraTM to world space
			vector cameraWorldTM[4];
			Math3D.MatrixMultiply4(animationBoneWorldTM, pOutResult.m_CameraTM, cameraWorldTM);

			//to quat, lerp, to TM
			float targetQuatWS[4];
			Math3D.MatrixToQuat(cameraWorldTM, targetQuatWS);
			Math3D.QuatLerp(targetQuatWS, m_ragdollHeadQuatWS, targetQuatWS, step);
			Math3D.QuatNorm(targetQuatWS);
			Math3D.QuatToMatrix(targetQuatWS, pOutResult.m_CameraTM);
			pOutResult.m_CameraTM[3] = cameraWorldTM[3];

			//go back to bone space
			vector invertedAnimationBoneWorldTM[4];
			Math3D.MatrixGetInverse4(animationBoneWorldTM, invertedAnimationBoneWorldTM);
			Math3D.MatrixMultiply4(invertedAnimationBoneWorldTM, pOutResult.m_CameraTM, pOutResult.m_CameraTM);

			//apply offset translation
			pOutResult.m_CameraTM[3] = m_OffsetLS;
		}
		else
		{
			Math3D.AnglesToMatrix(lookAngles, pOutResult.m_CameraTM);
			pOutResult.m_CameraTM[3] = m_OffsetLS;
		}

		// Apply camera shake if there is shake to be applied
		if (m_CharacterCameraHandler)
			m_CharacterCameraHandler.AddShakeToToTransform(pOutResult.m_CameraTM, pOutResult.m_fFOV);
	}

	protected float m_fEaseOutRagdollSmoothTime = 0.5;
	protected float m_fCurrentEaseOutRagdollSmoothTime = 0.0;
	protected float m_ragdollHeadQuatWS[4] = {0, 0, 0, 1};
	protected vector m_ragdollHeadWorldTR[4];
};
