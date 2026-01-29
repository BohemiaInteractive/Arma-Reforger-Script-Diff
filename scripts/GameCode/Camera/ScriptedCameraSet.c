// *************************************************************************************
// ! ScriptedCameraItem - Camera instance
// *************************************************************************************
class ScriptedCameraItem : BaseScriptedCameraItem 
{
	void ScriptedCameraItem(CameraHandlerComponent pCameraHandler)
	{
		m_CameraHandler = pCameraHandler;
	}

	//! this overrides freelook for cameras
	/*bool		CanFreeLook()
	{
		return true;
	}*/

	//! virtual callback - called when camera is created
	override void OnActivate (ScriptedCameraItem pPrevCamera, ScriptedCameraItemResult pPrevCameraResult);

	override void OnBlendIn();
	override void OnBlendOut();
	
	//!	virtual callback - called each frame
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult);
	void OnAfterCameraUpdate(float pDt, bool pIsKeyframe, inout vector transformMS[4]);

	//!	helper to blend between cameras 
	//! ret[0] - up down angle
	//! ret[1] - left right angle
	//! ret[2] - roll
	vector GetBaseAngles()
	{
		return	"0 0 0";
	}
	
	override void SetBaseAngles(out vector angles)
	{
		return;
	}
	
	float GetBaseFOV()
	{
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return 0;

		return cameraManager.GetFirstPersonFOV();
	}
	
	void OverrideDirectBoneMode(EDirectBoneMode boneMode)
	{
		m_OverrideDirectBoneMode = boneMode;
	}
	
	EDirectBoneMode GetOverrideDirectBoneMode()
	{
		return m_OverrideDirectBoneMode;
	}
			
	//! data 
	protected CameraHandlerComponent m_CameraHandler;	//!< component owner
	
	protected EDirectBoneMode m_OverrideDirectBoneMode = 0;
};

// *************************************************************************************
// ! ScriptedCameraSet
// *************************************************************************************
class ScriptedCameraSet : BaseScriptedCameraSet
{
	//! called when camera set is created - used for adding camera items and for transition between cameras settings
	override void Init();
	
	
	//! static function to set time to table
	void RegisterTransitionTime(int pFrom, int pTo, float pTime, bool pTwoWay)
	{
		int 	key = (pFrom * 65536) + pTo;
		m_TransitionTimes.Set(key, pTime);
		if (pTwoWay)
		{
			RegisterTransitionTime(pTo, pFrom, pTime, false);	
		}
	}
	
	//! static function to get time from table
	float GetTransitionTime(int pFrom, int pTo)
	{
		int 	key = (pFrom * 65536) + pTo;
		float 	val;

		if (m_TransitionTimes.Find(key, val))
		{
			return val;
		}
		else 
		{
			return 0.3;
		}
	}


	//! table of transition times
	private ref map<int,float> m_TransitionTimes = new map<int,float>;
};

enum EDirectBoneMode
{
	None = 0,// not used
	RelativePosition = 1,
	RelativeOrientation = 2,
	RelativeTransform = 3, // pos+rot applied as a parent to m_CameraTM
	RelativeDirection = 4 // as 3 but cam aligned with global Y
};

// *************************************************************************************
// ! ScriptedCameraItemResult - Camera result after update
// *************************************************************************************
class ScriptedCameraItemResult
{
	vector		m_CameraTM[4];			//!< transformation matrix - pos + orient of the camera
	float 		m_fFOV;					//!< fov absolute value override
	float		m_fNearPlane;			//!< nearplane distance
	float 		m_fPositionModelSpace;  //!< 0.0 position is in heading space, 1.0 position is in model space, 2.0 position is in world space
	float 		m_fDistance;			//!< camera distance (external cameras only)
	float 		m_fUseHeading;			//!< 0..1 (0 uses direct dir returned, 1 uses heading from player)
	float		m_fShoulderDist;		//!< -1..1 (-1 or 1 when shoulder transition is finished, inbetween when transitioning)
	
	int 		m_iDirectBone;			//!< -1 no bone, >= 0 - bone index camera is bound to, m_CameraTM is offset to the bone 
	EDirectBoneMode m_iDirectBoneMode;
	bool		m_bUpdateWhenBlendOut;	//!< true - camera is updated when blending to new camera (Ironsights == false)
	bool		m_bAllowInterpolation;	//!< true - camera interpolation is allowed (switching to/from head bone == false)
	bool		m_bAllowCollisionSolver; //!< true - camera collision solver is called
	vector		m_vBaseAngles;			//!< Freelook angles
	float		m_fHeading;				//!< main person heading (used to check for collision - don't change in script)
	bool		m_bNoParent = false;	//!< when enabled, camera hierarchy autoTransform is disabled for more freedom
	bool 		m_bBlendFOV = true;
	bool 		m_bWSCameraSmoothing = true;
	float 		m_fSmoothingTranslationSpeed = 5.0; //!< max camera smoothing translation speed m/s
	float 		m_fSmoothingRotationSpeed = Math.PI; //!< max camera smoothing rotation speed rad/s
	IEntity 	m_pOwner = null;
	ref PointInfo m_pWSAttachmentReference = null; //!< used with WSAttachment property it is either reference point info... It takes priority over m_pOwner. The owner of the PointInfo will be the new owner of the camera
	
	vector		m_vBacktraceDir;			//!< local direction used for backtrace if m_fUseBacktraceDir > 0. Used when m_bAllowCollisionSolver is true
	float		m_fUseBacktraceDir = 0.0;	//!< [0-1] Blend b/w m_vBacktraceDir and m_CameraTM[2]. default 0.0
	
	bool		m_bInterpolateOrientation = true;

	//! cannot be instanced from script (always created from C++)`
	private void ScriptedCameraItemResult()
	{
	}

	//! cannot be instanced from script (always created from C++)
	private void ~ScriptedCameraItemResult()
	{
	}
};
