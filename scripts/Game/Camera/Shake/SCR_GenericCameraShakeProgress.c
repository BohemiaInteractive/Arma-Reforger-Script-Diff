/*!
	This object implements generic way of applying camera shake,
	but does not implement any shaking by itself.
*/
class SCR_GenericCameraShakeProgress : SCR_BaseCameraShakeProgress
{
	protected vector m_vTranslation;
	protected vector m_vRotation;
	protected float m_fFovScale = 1.0;
	protected bool m_bIsRunning;
	
	override bool IsRunning()
	{
		return m_bIsRunning;
	}

	override void Apply(inout vector transformMatrix[4], inout float fieldOfView)
	{
		vector shakeMatrix[4];
		Math3D.AnglesToMatrix(m_vRotation, shakeMatrix);
		shakeMatrix[3] = m_vTranslation;

		vector modifiedMatrix[4];
		Math3D.MatrixMultiply4(transformMatrix, shakeMatrix, modifiedMatrix);
		transformMatrix[0] = modifiedMatrix[0]; // Math3D.MatrixCopy does not seem to work with inout params?
		transformMatrix[1] = modifiedMatrix[1];
		transformMatrix[2] = modifiedMatrix[2];
		transformMatrix[3] = modifiedMatrix[3];

		fieldOfView = m_fFovScale * fieldOfView;
	}

	override void Clear()
	{
		m_vTranslation = vector.Zero;
		m_vRotation = vector.Zero;
		m_fFovScale = 1.0;
	}
};
