class SCR_ColorAHSV
{
	float m_fAlpha;
	float m_fHue;
	float m_fSaturation;
	float m_fValue;

	//------------------------------------------------------------------------------------------------
	//! Constructs SCR_ColorAHSV from alpha, hue, saturation, and value.
	//! \param[in] alpha Alpha is the transparency level of the color.
	//! \param[in] hue Hue represents color's position on the color wheel, from 0 to 1.
	//! \param[in] saturation Saturates the color intensity.
	//! \param[in] value Value represents brightness.
	void SCR_ColorAHSV(float alpha, float hue, float saturation, float value)
	{
		m_fAlpha = alpha;
		m_fHue = hue;
		m_fSaturation = saturation;
		m_fValue = value;
	}

	//------------------------------------------------------------------------------------------------
	//! Converts an packed color value into an AHSV color object.
	//! \param[in] color Packed color.
	//! \return an SCR_ColorAHSV.
	static SCR_ColorAHSV FromColorInt(int color)
	{
		int a, r, g, b;
		Color.UnpackInt(color, a, r, g, b);
		return FromARGB(a / 255.0, r / 255.0, g / 255.0, b / 255.0);
	}

	//------------------------------------------------------------------------------------------------
	//! Converts color to AHSV format.
	//! \param[in] color Color.
	//! \return an AHSV color.
	static SCR_ColorAHSV FromColor(Color color)
	{
		return FromARGB(color.A(), color.R(), color.G(), color.B());
	}

	//------------------------------------------------------------------------------------------------
	//! Converts RGB color to AHSV format.
	//! \param[in] a Alpha component.
	//! \param[in] r Red component.
	//! \param[in] g Green component.
	//! \param[in] b Blue component.
	//! \return an AHSV color format.
	static SCR_ColorAHSV FromARGB(float a, float r, float g, float b)
	{
		float minValue = Math.Min(r, Math.Min(g, b));
		float maxValue = Math.Max(r, Math.Max(g, b));
		float valueDelta = maxValue - minValue;

		if (float.AlmostEqual(maxValue, 0) || float.AlmostEqual(valueDelta, 0))
		{
			return new SCR_ColorAHSV(a, 0, 0, maxValue);
		}

		float saturation = valueDelta / maxValue;
		float hue;
		if (float.AlmostEqual(maxValue, r))
			hue = (g - b) / valueDelta;
		else if (float.AlmostEqual(maxValue, g))
			hue = 2 + (b - r) / valueDelta;
		else
			hue = 4 + (r - g) / valueDelta;

		hue /= 6;
		if (hue < 1)
			hue += 1;

		return new SCR_ColorAHSV(a, hue, saturation, maxValue);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns a Color representation of this AHSV object.
	//! \return Engine color.
	Color ToColor()
	{
		if (float.AlmostEqual(m_fSaturation, 0))
			return new Color(m_fValue, m_fValue, m_fValue, m_fAlpha);

		m_fHue = Math.Mod(m_fHue, 1);
		float hueSextantF = m_fHue * 6;
		int hueSextant = Math.Floor(hueSextantF);
		float hueRemainder = hueSextantF - hueSextant;

		float channel1 = m_fValue * (1 - m_fSaturation);
		float channel2 = m_fValue * (1 - (m_fSaturation * hueRemainder));
		float channel3 = m_fValue * (1 - (m_fSaturation * (1 - hueRemainder)));

		switch (hueSextant)
		{
			case 0:
				return new Color(m_fValue, channel3, channel1, m_fAlpha);
			case 1:
				return new Color(channel2, m_fValue, channel1, m_fAlpha);
			case 2:
				return new Color(channel1, m_fValue, channel3, m_fAlpha);
			case 3:
				return new Color(channel1, channel2, m_fValue, m_fAlpha);
			case 4:
				return new Color(channel3, channel1, m_fValue, m_fAlpha);
			case 5:
			default:
				return new Color(m_fValue, channel1, channel2, m_fAlpha);
		}
		// Compiler ignores defaut switch case.
		return new Color(0, 0, 0, m_fAlpha);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns a copy with a shifted hue
	//! HueShift is added to the Hue and then moduloed.
	//! \param[in] hueShift HueShift should not be greater than 1 or less than -1;
	//! \return New SCR_ColorAHSV of rotated hue color with alpha, saturation, and value preserved.
	SCR_ColorAHSV WithHueShift(float hueShift)
	{
		SCR_ColorAHSV color = new SCR_ColorAHSV(m_fAlpha, m_fHue, m_fSaturation, m_fValue);
		color.m_fHue = Math.Mod(1 + color.m_fHue + hueShift, 1);
		return color;
	}
}
