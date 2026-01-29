/*!
	Interface for simulating individual camera shake(s).
*/
class SCR_BaseCameraShakeProgress
{
	/*!
		Is this shake 
	*/
	bool IsRunning();

	/*!
		Is this shake finished updating, should it be removed?
	*/
	sealed bool IsFinished()
	{
		return !IsRunning();
	}
	
	/*!
		First tick of the shake update.
	*/
	void Start();

	/*!
		Update the shake progress by a single step.
	*/
	void Update(IEntity owner, float timeSlice);

	/*!
		Apply the shake to camera matrix.
		\param transformMatrix Input/output camera matrix to apply transform to
		\param fieldOfView Input/otuput camera field of view in degrees
	*/
	void Apply(inout vector transformMatrix[4], inout float fieldOfView);

	/*!
		Clear progress of this shake.
	*/
	void Clear();
};
