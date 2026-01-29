class SCR_ManualCameraSettings: ModuleGameSettings
{
	[Attribute(defvalue: "1", desc: "When enabled, camera will move horizontally instead of flying directly forward.")]
	bool m_bCameraMoveATL;
	
	[Attribute(defvalue: "1", desc: "When enabled, camera will move faster when higher above terrain.\nThis setting has effect only if 'Camera Move ATL' is enabled as well.")]
	bool m_bCameraSpeedATL;
	
	[Attribute(defvalue: "1", desc: "When enabled, camera cannot move below terrain.")]
	bool m_bCameraAboveTerrain;
	
	[Attribute(defvalue: "1", desc: "When enabled, camera rotates only when modifier button (RMB by default) is pressed.")]
	bool m_bCameraRotateWithModifier;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, params: "0.1 10 0.1", desc: "General camera speed coefficient.")]
	float m_fCameraSpeedCoef;
	
	[Attribute(desc: "Saved camera positions per terrain")]
	ref array<ref SCR_ManualCameraSave> m_aSavedData;
};