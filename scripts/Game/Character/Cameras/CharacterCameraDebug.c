// *************************************************************************************
// ! CharacterCameraDebug - debug views
// *************************************************************************************
class CharacterCameraDebug : CharacterCameraBase
{
	//------------------------------------------------------------------------------------------------
	// constructor
	void CharacterCameraDebug(CameraHandlerComponent pCameraHandler)
	{
		m_iHeadBoneIndex = m_OwnerCharacter.GetAnimation().GetBoneIndex("Head");
		
		m_WeaponManager = BaseWeaponManagerComponent.Cast(m_OwnerCharacter.FindComponent(BaseWeaponManagerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate (ScriptedCameraItem pPrevCamera, ScriptedCameraItemResult pPrevCameraResult)
	{
		super.OnActivate(pPrevCamera, pPrevCameraResult);
	}

	//------------------------------------------------------------------------------------------------
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		pOutResult.m_bUpdateWhenBlendOut	= false;
		pOutResult.m_bWSCameraSmoothing 	= false;
		pOutResult.m_pOwner 				= m_OwnerCharacter;

		int debugView = DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_CHARACTER_DEBUG_VIEW);
		if( debugView == CharacterCameraSet.DEBUGVIEW_FACE )
		{
			Math3D.MatrixIdentity4(pOutResult.m_CameraTM);

			// position
			pOutResult.m_CameraTM[3] = "0.0 0.03 -0.07";
			pOutResult.m_CameraTM[3] = -vector.Forward * 0.6 + pOutResult.m_CameraTM[3];

			pOutResult.m_iDirectBone 		= m_iHeadBoneIndex;
			pOutResult.m_iDirectBoneMode 	= EDirectBoneMode.RelativeDirection;
			pOutResult.m_fUseHeading 		= 0.0;
			pOutResult.m_fFOV				= 30;
			
			super.OnUpdate(pDt, pOutResult);
		}
		else if( debugView == CharacterCameraSet.DEBUGVIEW_WEAPON )
		{
			vector charMat[4];
			m_OwnerCharacter.GetTransform(charMat);

			vector baseMat[4];
			if (m_WeaponManager)
				m_WeaponManager.GetCurrentMuzzleTransform(baseMat);
			else
				m_OwnerCharacter.GetTransform(baseMat);
			
			vector rotMat[3];
			Math3D.AnglesToMatrix("180 0 0", rotMat);
			Math3D.MatrixMultiply3(baseMat, rotMat, pOutResult.m_CameraTM);
			pOutResult.m_CameraTM[3] = baseMat[2] * 1.5 + baseMat[3];
			
			Math3D.MatrixInvMultiply4(charMat, pOutResult.m_CameraTM, pOutResult.m_CameraTM);
			
			pOutResult.m_fFOV = 40;
		}
		else
		{
			//vector rotMat[3];
			if (debugView == CharacterCameraSet.DEBUGVIEW_LEFT)
				Math3D.AnglesToMatrix("90 0 0", pOutResult.m_CameraTM);
			else if (debugView == CharacterCameraSet.DEBUGVIEW_RIGHT)
				Math3D.AnglesToMatrix("-90 0 0", pOutResult.m_CameraTM);
			else if (debugView == CharacterCameraSet.DEBUGVIEW_FRONT)
				Math3D.AnglesToMatrix("180 0 0", pOutResult.m_CameraTM);
			else if (debugView == CharacterCameraSet.DEBUGVIEW_REAR)
				Math3D.AnglesToMatrix("0 0 0", pOutResult.m_CameraTM);
			else if (debugView == CharacterCameraSet.DEBUGVIEW_TOP)
				Math3D.AnglesToMatrix("0 -90 0", pOutResult.m_CameraTM);
			else if (debugView == CharacterCameraSet.DEBUGVIEW_BOTTOM)
				Math3D.AnglesToMatrix("0 90 0", pOutResult.m_CameraTM);

			pOutResult.m_fUseHeading = 0.0;
			pOutResult.m_CameraTM[3] = pOutResult.m_CameraTM[2] * -2.5 + vector.Up * 1;
			pOutResult.m_fFOV = 50;
		}
	}
	
	protected int m_iHeadBoneIndex;
	protected BaseWeaponManagerComponent m_WeaponManager = null;
}
