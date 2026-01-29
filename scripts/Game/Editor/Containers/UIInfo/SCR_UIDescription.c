//! Class used to hold UI name and description.
//! Data are intentionally *READ ONLY*, because the class is often used on prefabs, not instances.
//! Instead of adding SetXXX() functions here, consider using specialized inherited class.
[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_UIDescription : SCR_UIName
{
	[Attribute(uiwidget: UIWidgets.EditBoxMultiline)]
	protected LocalizedString Description;
	
	//------------------------------------------------------------------------------------------------
	//! Get the unformatted description from SCR_HintUIInfo instance.
	//! \return LocalizedString description set in SCR_HintUIInfo instance.
	LocalizedString GetUnformattedDescription()
	{
		return Description;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Change the description from SCR_HintUIInfo instance.
	//! Call SCR_HintManagerComponent.Refresh() to update shown hint to display changed text.
	//! \param LocalizedString text to which discription should be updated to.
	void SetDescription(LocalizedString description)
	{
		Description = description;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get description.
	//! When using it to fill TextWidget, use SetDescriptionTo() if possible.
	//! \return Description text
	LocalizedString GetDescription()
	{
		return Description;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if the info has a description defined.
	//! \return True when the description is defined
	bool HasDescription()
	{
		return !GetDescription().IsEmpty();
	}
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----	
// This class is meant to hold data, not manipulate widgets! Such methods should be in SCR_WidgetTools or SCR_WidgetHelper. In this case, I would expect whatever component is handling the layout to make sure the text widget exists, so this method is not needed
	
	//------------------------------------------------------------------------------------------------
	//! Set description to given text widget.
	//! When possible, use this function instead of retrieving the description using GetDescription() and setting it manually.
	//! Custom UI info classes may be using parameters which would not be applied otherwise.
	//! \param textWidget Target text widget
	//! \return True when the description was set
	bool SetDescriptionTo(TextWidget textWidget)
	{
		if (!textWidget)
			return false;
		
		textWidget.SetText(GetDescription());
		return true;
	}
	
//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param prefix
	//! \param logLevel
	override void Log(string prefix = string.Empty, LogLevel logLevel = LogLevel.VERBOSE)
	{
		Print(string.Format(prefix + "%1: \"%2\", \"%3\"", Type(), Name, Description), logLevel);
	}
	
	//------------------------------------------------------------------------------------------------
	//--- Protected, to be overridden and/or made public by inherited classes
	override protected void CopyFrom(SCR_UIName source)
	{
		SCR_UIDescription sourceDescription = SCR_UIDescription.Cast(source);
		if (sourceDescription)
		{
			Description = sourceDescription.Description;
		}
		
		super.CopyFrom(source);
	}
}
