class SCR_CharacterUIComponent : ScriptedWidgetComponent
{
	[Attribute("0", uiwidget: UIWidgets.EditBox)]
	protected float m_fArrowRotation;

	[Attribute("0", uiwidget: UIWidgets.SearchComboBox, enumType: SCR_ECharacterTypeUI)]
	protected SCR_ECharacterTypeUI m_eCharacterTypes;

	[Attribute("1.0 1.0 1.0 1.0", uiwidget: UIWidgets.ColorPicker)]
	protected ref Color m_CharacterColor;

	[Attribute("IconCharType", uiwidget: UIWidgets.EditBox, "Name of the image widget, that holds character type icon")]
	protected string m_sIconCharType;

	[Attribute("Arrow-Texture", uiwidget: UIWidgets.EditBox, "Name of the image widget, that holds arrow texture")]
	protected string m_sIconArrowTexture;

	[Attribute("CharacterOverlay", uiwidget: UIWidgets.EditBox, "Name of the Overlay widget that holds color value")]
	protected string m_sCharacterOverlay;

	[Attribute("{789DC92AF28E0AB1}UI/Imagesets/Characters/CharacterWrapperUI.imageset", "Imageset source for Character Type")]
	protected ResourceName m_sCharTypeImageset;

	protected ref array<string> m_aTypes = {"AntiTank","CrewCommander","CrewMan","Driver","Engineer","Grenadier","HeliCrew","HeliPilot","Machinegunner","Medic","MortarCrew","Officer","Outline","PlatoonLeader","PlattonSergeant","RadioOperator","Rifleman","Sapper","Scout","ScoutRadioOperator","Sharpshooter","SpecialForces"};

	override void HandlerAttached(Widget w)
	{
		//Casting Widget types
		ImageWidget m_wIconCharType = ImageWidget.Cast(w.FindAnyWidget(m_sIconCharType));
		ImageWidget m_wArrowWidget = ImageWidget.Cast(w.FindAnyWidget(m_sIconArrowTexture));
		OverlayWidget m_wCharacterOverlay = OverlayWidget.Cast(w.FindAnyWidget(m_sCharacterOverlay));

		SetCharTypeTexture(m_wIconCharType, m_aTypes);
		SetArrowRotation(m_wArrowWidget);
		SetCharacterColor(m_wCharacterOverlay);
	}

	/*!
	Get character types enum.
	\return Characters
	*/
	//------------------------------------------------------------------------------------------------
	SCR_ECharacterTypeUI GetCharacterTypes()
	{
		return m_eCharacterTypes;
	}

	/*!
	Set character type.
	\param characterTypes
	*/
	//------------------------------------------------------------------------------------------------
	void SetIdentity(SCR_ECharacterTypeUI characterTypes)
	{
		m_eCharacterTypes = characterTypes;
	}

	/*!
	Get rotation Float.
	\return m_fArrowRotation
	*/
	//------------------------------------------------------------------------------------------------
	float GetRotationFloat()
	{
		return m_fArrowRotation;
	}

	/*!
	Set character type.
	\param arrowRotation
	*/
	//------------------------------------------------------------------------------------------------
	void SetRotation(float arrowRotation)
	{
		m_fArrowRotation = arrowRotation;
	}

	/*!
	Get Character Color
	\return m_CharacterColor
	*/
	//------------------------------------------------------------------------------------------------
	Color GetColor()
	{
		return m_CharacterColor;
	}

	/*!
	Set Color.
	\param characterColor
	*/
	//------------------------------------------------------------------------------------------------
	void SetColor(Color characterColor)
	{
		m_CharacterColor = characterColor;
	}

	/*!
	Set character type texture
	\param characterTypeIcon
	\param typeArray
	*/
	//------------------------------------------------------------------------------------------------
	void SetCharTypeTexture(ImageWidget characterTypeIcon, array<string> typeArray)
	{
		characterTypeIcon.LoadImageFromSet(0, m_sCharTypeImageset, typeArray[m_eCharacterTypes]);
	}

	/*!
	Set Arrow Rotation
	\param arrowWidget
	*/
	//------------------------------------------------------------------------------------------------
	void SetArrowRotation(ImageWidget arrowWidget)
	{
		arrowWidget.SetRotation(m_fArrowRotation);
	}

	/*!
	Set character widget color
	\param characterOverlay
	*/
	//------------------------------------------------------------------------------------------------
	void SetCharacterColor(OverlayWidget characterOverlay)
	{
		characterOverlay.SetColor(m_CharacterColor);
	}
}
