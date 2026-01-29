/*
Collection of functions to work with rich text tags.
*/

class SCR_RichTextTags
{
	//------------------------------------------------------------------------------------
	//! content - the content which will be wrapped into the tag.
	//! color - in linear space.
	static string TagColor(string content, Color color)
	{
		Color colorCopy = Color.FromInt(Color.WHITE);
		colorCopy = Color.FromInt(color.PackToInt());
		colorCopy.LinearToSRGB();
		
		return string.Format("<color rgba=\"%1,%2,%3,%4\">%5</color>",
			Math.Floor(255.0*colorCopy.R()),
			Math.Floor(255.0*colorCopy.G()),
			Math.Floor(255.0*colorCopy.B()),
			Math.Floor(255.0*colorCopy.A()),
			content);
	}
};