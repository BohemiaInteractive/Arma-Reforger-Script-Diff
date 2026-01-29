/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Visual
\{
*/

class Color: Managed
{
	//! color constants - hex codes
	static const int BLACK = 0xff000000; // black
	static const int GRAY_25 = 0xff404040; // charcoal
	static const int GRAY = 0xff808080; // gray
	static const int GRAY_75 = 0xffc0c0c0; // silver
	static const int WHITE = 0xffffffff; // white
	static const int RED = 0xffff0000; // red
	static const int GREEN = 0xff00ff00; // lime
	static const int BLUE = 0xff0000ff; // blue
	static const int DARK_RED = 0xff800000; // maroon
	static const int DARK_GREEN = 0xff008000; // green
	static const int DARK_BLUE = 0xff000080; // navy
	static const int CYAN = 0xff00ffff; // cyan
	static const int MAGENTA = 0xffff00ff; // magenta
	static const int YELLOW = 0xffffff00; // yellow
	static const int DARK_CYAN = 0xff008080; // teal
	static const int DARK_MAGENTA = 0xff800080; // purple
	static const int DARK_YELLOW = 0xff808000; // olive
	static const int ORANGE = 0xffff8000; // dark orange
	static const int CHARTREUSE = 0xff80ff00; // chartreuse
	static const int SPRING_GREEN = 0xff00ff80; // spring green
	static const int DODGER_BLUE = 0xff0080ff; // dodger blue
	static const int VIOLET = 0xff8000ff; // electric indigo
	static const int PINK = 0xffff0080; // deep pink

	//! color constants - color objects
	static const ref Color Black = new Color(0.0, 0.0, 0.0, 1.0); // black
	static const ref Color Gray25 = new Color(0.25, 0.25, 0.25, 1.0); // charcoal
	static const ref Color Gray = new Color(0.5, 0.5, 0.5, 1.0); // gray
	static const ref Color Gray75 = new Color(0.75, 0.75, 0.75, 1.0); // silver
	static const ref Color White = new Color(1.0, 1.0, 1.0, 1.0); // white
	static const ref Color Red = new Color(1.0, 0.0, 0.0, 1.0); // red
	static const ref Color Green = new Color(0.0, 1.0, 0.0, 1.0); // lime
	static const ref Color Blue = new Color(0.0, 0.0, 1.0, 1.0); // blue
	static const ref Color DarkRed = new Color(0.5, 0.0, 0.0, 1.0); // maroon
	static const ref Color DarkGreen = new Color(0.0, 0.5, 0.0, 1.0); // green
	static const ref Color DarkBlue = new Color(0.0, 0.0, 0.5, 1.0); // navy
	static const ref Color Cyan = new Color(0.0, 1.0, 1.0, 1.0); // cyan
	static const ref Color Magenta = new Color(1.0, 0.0, 1.0, 1.0); // magenta
	static const ref Color Yellow = new Color(1.0, 1.0, 0.0, 1.0); // yellow
	static const ref Color DarkCyan = new Color(0.0, 0.5, 0.5, 1.0); // teal
	static const ref Color DarkMagenta = new Color(0.5, 0.0, 0.5, 1.0); // purple
	static const ref Color DarkYellow = new Color(0.5, 0.5, 0.0, 1.0); // olive
	static const ref Color Orange = new Color(1.0, 0.5, 0.0, 1.0); // dark orange
	static const ref Color Chartreuse = new Color(0.5, 1.0, 0.0, 1.0); // chartreuse
	static const ref Color SpringGreen = new Color(0.0, 1.0, 0.5, 1.0); // spring green
	static const ref Color DodgerBlue = new Color(0.0, 0.5, 1.0, 1.0); // dodger blue
	static const ref Color Violet = new Color(0.5, 0.0, 1.0, 1.0); // electric indigo
	static const ref Color Pink = new Color(1.0, 0.0, 0.5, 1.0); // deep pink
	// --------------------------------------------------------------------
	void Color(float red = 0.0, float green = 0.0, float blue = 0.0, float alpha = 0.0);

	//! Creates a new color instance from ARGB color packed in int
	static proto ref Color FromInt(int color);
	//! Creates a new color instance from RGB vector
	static proto ref Color FromVector(vector color);
	//! Creates a new color instance from RGBA integers in range 0-255
	static proto ref Color FromRGBA(int r, int g, int b, int a);
	//! Creates a new color instance in linear color space from ARGB color packed in int which is in sRGB color space
	static proto ref Color FromIntSRGB(int color);
	//! Creates a new color instance in linear color space from RGBA integers in range 0-255 which are in sRGB color space
	static proto ref Color FromSRGBA(int r, int g, int b, int a);
	//! Returns Red component
	proto external float R();
	//! Returns Green component
	proto external float G();
	//! Returns Blue component
	proto external float B();
	//! Returns Alpha component
	proto external float A();
	//! Sets the Red component
	proto external void SetR(float red);
	//! Sets the Green component
	proto external void SetG(float green);
	//! Sets the Blue component
	proto external void SetB(float blue);
	//! Sets the Alpha component
	proto external void SetA(float alpha);
	//! Sets the color to color given in packed ARGB integer
	proto external void SetFromInt(int color);
	//! Component-wise addition-assignment
	proto external void Add(Color other);
	//! Component-wise addition returns result as a new color
	proto ref Color AddNew(Color other);
	//! Component-wise subtraction-assignment
	proto external void Sub(Color other);
	//! Component-wise subtraction returns result as a new color
	proto ref Color SubNew(Color other);
	//! Component-wise multiplication-assignment
	proto external void Mul(Color other);
	//! Component-wise multiplication returns result as a new color
	proto ref Color MulNew(Color other);
	//! Component-wise division-assignment
	proto external void Div(Color other);
	//! Component-wise division returns result as a new color
	proto ref Color DivNew(Color other);
	//! Multiplies each component by `scale`
	proto external void Scale(float scale);
	//! Returns perceptual brightness
	proto external float GetLuminance();
	//! Clamps each component to [0, 1] range
	proto external void Saturate();
	//! Clamps each component to [min, max] range
	proto external void Clamp(float min, float max);
	/*!
	Linear interpolation with `other` based on `t`.
	Returns `this + (other - this) * t`.
	*/
	proto external void Lerp(Color other, float t);
	/*!
	\see Lerp
	\return result as a new color
	*/
	proto ref Color LerpNew(Color other, float t);
	//! Returs this color as integer with 8 bits per channel
	proto external int PackToInt();
	/*!
	Unpack color data from packed integer, see PackToInt().
	\param argb input packed argb int
	\param a out alpha channel <0..255>
	\param r out red channel <0..255>
	\param g out green channel <0..255>
	\param b out blue channel <0..255>
	*/
	static proto void UnpackInt(int argb, out int a, out int r, out int g, out int b);
	//! Converts this color from sRGB color space to linear RGB color space
	proto external void SRGBToLinear();
	//! Converts this color from linear RGB color space to sRGB color space
	proto external void LinearToSRGB();
	//! Returns true if all the components are zero
	proto external bool IsZero();
	//! Returns true if the colors differ by at most `epsilon` in each component
	proto external bool IsEqual(Color other, float epsilon = 0.0);
}

/*!
\}
*/
