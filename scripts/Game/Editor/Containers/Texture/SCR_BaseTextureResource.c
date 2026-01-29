[BaseContainerProps()]
class SCR_BaseTextureResource
{
	//------------------------------------------------------------------------------------------------
	//! Apply the placeholder texture to the provided widget
	//! \param widget if null, nothing is done
	void ApplyTo(ImageWidget widget)
	{
		if (widget)
			widget.LoadImageTexture(0, "{AC7E384FF9D8016A}Common/Textures/placeholder_BCR.edds"); //--- Hard-coded placeholder. Ugly, but necessary.
	}
}
