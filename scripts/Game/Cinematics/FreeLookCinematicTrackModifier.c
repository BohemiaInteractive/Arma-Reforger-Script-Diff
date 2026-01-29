class FreeLookCinematicTrackModifier : CinematicTrackModifier
{
	float lastX;
	float lastY;
	float deltaX;
	float deltaY;
	float inactiveX;
	float inactiveY;
	float deltaCumulX;
	float deltaCumulY;
	bool firstFrame = true;
	
	bool invertMouse = false;
	int signMouse = 1;
	
	[Attribute(defvalue:"45", desc:"MaxAngleX")]
	int maxAngleX;
	
	[Attribute(defvalue:"30", desc:"MaxAngleX")]
	int maxAngleY;
	
	[Attribute(defvalue:"true")]
	bool returnCameraAfterInactivity;
	
	override vector OnApplyModifierVector(float time, vector originalValue)
	{
		
		InputManager inputManager = GetGame().GetInputManager();
		inputManager.ActivateContext("CharacterFreelookMouseContext");
		inputManager.ActivateContext("CharacterFreelookGamepadContext");

        // Get Mouse Movement
        float positionX = inputManager.GetActionValue("MouseX");
        float positionY = inputManager.GetActionValue("MouseY");
		
		BaseContainer inputDeviceSettings = GetGame().GetEngineUserSettings().GetModule("InputDeviceUserSettings");
		
		if (inputDeviceSettings)
			inputDeviceSettings.Get("MouseInvert", invertMouse);
		
		if (invertMouse)
			 signMouse = -1;
		else
			signMouse = 1;
		
		if (firstFrame)
		{
			firstFrame = false;
			lastX = positionX;
			lastY = positionY;
			return originalValue;	
		}
		
		deltaX = (positionX - lastX) / 15 - inputManager.GetActionValue("FreelookGamepadLeft") + inputManager.GetActionValue("FreelookGamepadRight");
		deltaY = (-signMouse * positionY + signMouse * lastY) / 15 + inputManager.GetActionValue("FreelookGamepadUp") - inputManager.GetActionValue("FreelookGamepadDown");;
		
		lastX = positionX;
		lastY = positionY;
		
		if (deltaX == 0 && returnCameraAfterInactivity) //no x input move
		{
			inactiveX += 0.001;
			
			if (inactiveX > 0.06) //for more than second
				deltaCumulX = deltaCumulX / (1 + inactiveX - 0.06); //slowly get back to original camera
		}
		else //if there is input - full impact on camera
		{
			inactiveX = 0;
			deltaCumulX += deltaX;
		}
		
		if (deltaY == 0 && returnCameraAfterInactivity) //no y input move
		{
			inactiveY += 0.001;
			
			if (inactiveY > 0.06) //for more than second
				deltaCumulY = deltaCumulY / (1 + inactiveY - 0.06); //slowly get back to original camera
		}
		else //if there is input - full impact on camera
		{
			inactiveY = 0;
			deltaCumulY += deltaY;
		}
		
		//Print(deltaCumulY);
		deltaCumulX = Math.Clamp(deltaCumulX, -maxAngleX, maxAngleX);
		deltaCumulY = Math.Clamp(deltaCumulY, -maxAngleY, maxAngleY);
		
		vector newValue;	
		newValue[0] = originalValue[0] + deltaCumulY;
		newValue[1] = originalValue[1] + deltaCumulX;
		newValue[2] = originalValue[2];
		
		return newValue;
	}
};
