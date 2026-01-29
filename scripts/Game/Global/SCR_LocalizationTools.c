class SCR_LocalizationTools
{
	static const string FORMAT_TAG = "##";
	
	protected static bool AppendParam(out string text, string p)
	{
		text += FORMAT_TAG + p;
		return p != string.Empty;
	}
	/*!
	Encode text with params to single-line text.
	\param text Text body
	\param p1-p9 Params
	\return Encoded text. Remains unchanged if there were no params provided.
	*/
	static string EncodeFormat(string text, string p1 = string.Empty, string p2 = string.Empty, string p3 = string.Empty, string p4 = string.Empty, string p5 = string.Empty, string p6 = string.Empty, string p7 = string.Empty, string p8 = string.Empty, string p9 = string.Empty)
	{
		string params;
		bool isParam = AppendParam(params, p1)
					|| AppendParam(params, p2)
					|| AppendParam(params, p3)
					|| AppendParam(params, p4)
					|| AppendParam(params, p5)
					|| AppendParam(params, p6)
					|| AppendParam(params, p7)
					|| AppendParam(params, p8)
					|| AppendParam(params, p9);
		
		if (isParam)
			return FORMAT_TAG + text + params;
		else
			return text;
	}
	/*!
	Decode single-line text with params to individual strings.
	\param text Text with expected params
	\param[out] p1-p9 Params
	\return Decoded text body
	*/
	static string DecodeFormat(string text, out string p1, out string p2, out string p3, out string p4, out string p5, out string p6, out string p7, out string p8, out string p9)
	{
		if (!text.StartsWith(FORMAT_TAG))
			return text;
		
		array<string> segments = {};
		text.Split(FORMAT_TAG, segments, false);
		segments.Resize(11);
		
		p1 = segments[2];
		p2 = segments[3];
		p3 = segments[4];
		p4 = segments[5];
		p5 = segments[6];
		p6 = segments[7];
		p7 = segments[8];
		p8 = segments[9];
		p9 = segments[10];
		
		return segments[1];
	}
	/*!
	Apply text with expected single-line params to text widget.
	\param w Text widget
	\param text Text with expected params
	*/
	static void SetTextFormat(TextWidget w, string text)
	{
		if (text.StartsWith(FORMAT_TAG))
		{
			string p1, p2, p3, p4, p5, p6, p7, p8, p9;
			text = DecodeFormat(text, p1, p2, p3, p4, p5, p6, p7, p8, p9);
			w.SetTextFormat(text, p1, p2, p3, p4, p5, p6, p7, p8, p9);
		}
		else
		{
			w.SetText(text);
		}
	}
};