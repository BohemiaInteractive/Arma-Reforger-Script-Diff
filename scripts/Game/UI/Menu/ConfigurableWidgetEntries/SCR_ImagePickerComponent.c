void ScriptInvokerImagePickerComponentMethod(SCR_ImagePickerComponent picker);
typedef func ScriptInvokerImagePickerComponentMethod;
typedef ScriptInvokerBase<ScriptInvokerImagePickerComponentMethod> ScriptInvokerImagePickerComponent;

/*
void ScriptInvokerImagePicker(vector v);
typedef func ScriptInvokerVectorMethod;
typedef ScriptInvokerBase<ScriptInvokerVectorMethod> ScriptInvokerVector;
*/

/*!
Allows image to be set and removed directly on widget
*/
class SCR_ImagePickerComponent: SCR_ChangeableComponentBase
{
	protected const string TESTING_IMAGE = "{A6D83FD56C68EB77}UI/Textures/MissionLoadingScreens/campaignMP_UI.edds";
	
	[Attribute("Image", desc: "Name of widget for displaying picked")]
	protected string m_sImageName;
	
	protected ResourceName m_sResourcePath;
	
	protected TextWidget m_wImageSourceText;
	protected SCR_ModularButtonComponent m_PickerButton;
	
	protected ref ScriptInvokerImagePickerComponent m_OnPickerButtonClicked;
	protected ref ScriptInvokerBase<ScriptInvokerStringMethod> m_OnResourcePicked;
	
	//-------------------------------------------------------------------------------------------
	// Override
	//-------------------------------------------------------------------------------------------
	
	//-------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		// Setup 
		m_wImageSourceText = TextWidget.Cast(w.FindAnyWidget("ImageText"));
		Widget pickerButton = w.FindAnyWidget("PickerButton");
		m_PickerButton = SCR_ModularButtonComponent.Cast(pickerButton.FindHandler(SCR_ModularButtonComponent));
		
		m_PickerButton.m_OnClicked.Insert(OnPickerButtonClicked);
	}
	
	//-------------------------------------------------------------------------------------------
	//! Solve interaction: Open image capture or delete current  
	protected void OnPickerButtonClicked(SCR_ModularButtonComponent button)
	{
		if (m_sResourcePath.IsEmpty())
			CaptureImage();
		else
			RemoveImage();
		
		if (m_OnPickerButtonClicked)
			m_OnPickerButtonClicked.Invoke(this);
	}
	
	//-------------------------------------------------------------------------------------------
	void CaptureImage()
	{
		SCR_EditorManagerEntity.GetInstance().SetCurrentMode(EEditorMode.PHOTO_SAVE);
		
		// TODO: Send out image capture to hide dialog
	}

	//-------------------------------------------------------------------------------------------
	void SetImage(string image)
	{
		m_sResourcePath = image;
		m_wImageSourceText.SetText(image);
		
		if (m_OnResourcePicked)
			m_OnResourcePicked.Invoke(image);
	}
	
	//-------------------------------------------------------------------------------------------
	void RemoveImage()
	{
		m_sResourcePath = "";
		m_wImageSourceText.SetText("");
		
		if (m_OnResourcePicked)
			m_OnResourcePicked.Invoke("");
	}
	
	//-------------------------------------------------------------------------------------------
	ResourceName GetResourcePath()
	{
		return m_sResourcePath;
	}
	
	//-------------------------------------------------------------------------------------------
	ScriptInvokerImagePickerComponent GetOnPickerButtonClick()
	{
		if (!m_OnPickerButtonClicked)
			m_OnPickerButtonClicked = new ScriptInvokerImagePickerComponent();
		
		return m_OnPickerButtonClicked;
	}
	
	//-------------------------------------------------------------------------------------------
	ScriptInvokerBase<ScriptInvokerStringMethod> GetOnResourcePicked()
	{
		if (!m_OnResourcePicked)
			m_OnResourcePicked = new ScriptInvokerBase<ScriptInvokerStringMethod>();
		
		return m_OnResourcePicked;
	}
}