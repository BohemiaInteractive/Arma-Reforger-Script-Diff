/*!
Widget list entry specific for publishing dialog image settings 
Has a list of images for thumbnail and image gallery. Player can fill it with custom images
Returns paths to images 
*/

[BaseContainerProps(configRoot: true), SCR_WidgetListEntryCustomTitle()]
class SCR_WidgetListEntryPublishImages : SCR_WidgetListEntry
{
	protected SCR_SaveImageGalleryPickerComponent m_SaveImagesGallery;
	
	//-------------------------------------------------------------------------------------------
	//! Removed setting up of m_ChangeableComponent as it's not used for this case 
	override void CreateWidget(Widget parent)
	{
		// Create widget 
		m_EntryRoot = GetGame().GetWorkspace().CreateWidgets(m_sEntryLayout, parent);
	
		// Show 
		m_EntryRoot.SetVisible(m_bShow);
		if (!m_bShow)
			return;
		
		SetupHandlers();
		SetInteractive(m_bInteractive);
	}
	
	//-------------------------------------------------------------------------------------------
	override protected void SetupHandlers()
	{
		m_SaveImagesGallery = SCR_SaveImageGalleryPickerComponent.Cast(m_EntryRoot.FindHandler(SCR_SaveImageGalleryPickerComponent));
	}
	
	//-------------------------------------------------------------------------------------------
	SCR_SaveImageGalleryPickerComponent GetSaveImagesGallery()
	{
		return m_SaveImagesGallery;
	}
}