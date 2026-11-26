class SCR_InputHelper
{
	static const string INPUT_CONTAINER_SOURCE_COMBO_TYPE = "InputSourceCombo";
	static const string INPUT_CONTAINER_FILTER_NAME = "Filter";
	static const string INPUT_CONTAINER_MULTIPLIER_NAME = "Multiplier";
	static const string INPUT_CONTAINER_OFFSET_NAME = "Offset";
	static const string INPUT_CONTAINER_DEADZONE_NAME = "Deadzone";
	static const string INPUT_CONTAINER_TRANSITION_CURVE_NAME = "Transition";
	static const string INPUT_CONTAINER_FILTER_CURVE_VALUES_NAME = "Values";
	static const string INPUT_CONTAINER_FILTER_CURVE_KNOTS_NAME = "Knots";
	static const string INPUT_CONTAINER_FILTER_SPLINE_TYPE_NAME = "SplineType";
	static const string INPUT_CONTAINER_INVERT_NAME = "UseInvert";
	static const string INPUT_CONTAINER_INPUT_SOURCE_NAME = "InputSource";
	static const string INPUT_CONTAINER_SOURCES_NAME = "Sources";
	static const string INPUT_CONTAINER_FILTER_PRESET_NAME = "FilterPreset";
	static const string INPUT_CONTAINER_INPUT_NAME = "Input";
	static const string INPUT_CONTAINER_THRESHOLD_NAME = "Threshold";
	static const string INPUT_CONTAINER_HOLD_DURATION_NAME = "HoldDuration";
	static const string INPUT_CONTAINER_CLICK_DURATION_NAME = "ClickDuration";
	static const string INPUT_DEVICE_CONTAINER_DEVICE_NAME = "DeviceNameContains";
	static const string INPUT_DEVICE_CONTAINER_DEVICE_SLOT_NAME = "DeviceSlotNumber";
	static const string INPUT_FILTER_VALUE_TYPE_NAME = "InputFilterValue";
	static const string INPUT_CURVE_CONTAINER_TYPE = "CurveCubicSplineFloat";
	static const string DEVICE_ID_DISPLAY_FORMAT = "%1 %2";
	static const string ENGINE_INPUT_DEVICE_USER_SETTINGS_NAME = "InputDeviceUserSettings";
	static const string ENGINE_SETTINGS_INPUT_GAMEPAD_PROFILE_NAME = "GamepadProfile";
	// engine setttings use 'DeadZone', while input filters use 'Deadzone' :/
	static const string ENGINE_SETTINGS_INPUT_DEAD_ZONE_NAME = "DeadZone";

	static const LocalizedString DEVICE_ID_DISPLAY_TEXT = "#AR-Settings_Joysticks_DeviceId";

	static const int INPUT_CURVE_KNOTS_COUNT = 11;

	protected const int BACKGROUND_VERTICAL_LINE_WIDTH = 2;
	protected const int BACKGROUND_HORIZONTAL_LINE_WIDTH = 1;
	protected const int FOREGROUND_LINE_WIDTH = 7;
	protected const int FOREGROUND_OUTLINE_WIDTH = 1;

	//------------------------------------------------------------------------------------------------
	//! Finds and returns input binding container
	//! \param[in] actionName for which input will be fetched
	//! \param[in] actionPreset filter preset which is used by the searched binding
	//! \param[in] inputId id of the binding which should be fetched, if inputId is below 0, then entire InputSource container is returned
	//! \param[in] binding cached version of user bindings handler
	//! \return container with matching binding, and if such is not found then null. In case of combo inputs it will return first valid InputSourceValue container
	static BaseContainer GetInputBinding(string actionName, string actionPreset, int inputId, InputBinding binding = null)
	{
		ref InputBinding bindingInstance = binding; // use hard ref or else if it wasnt provided, then it wont be created
		if (!bindingInstance)
		{
			bindingInstance = GetGame().GetInputManager().CreateUserBinding();
			if (!bindingInstance)
				return null;
		}

		BaseContainer actionContaier = bindingInstance.FindAction(actionName);
		if (!actionContaier)
			return null;

		BaseContainer inputSrc = actionContaier.GetObject(INPUT_CONTAINER_INPUT_SOURCE_NAME);
		if (!inputSrc)
			return null;

		if (inputId < 0)
			return inputSrc;

		BaseContainerList sources = inputSrc.GetObjectArray(INPUT_CONTAINER_SOURCES_NAME);
		if (!sources)
			return null;

		int count = sources.Count();
		if (count < 1)
			return null;

		string filter, input;
		BaseContainer source;
		BaseContainerList comboSources;
		BaseContainer comboSource;
		int matchingInputs = -1;
		for (int i; i < count; i++)
		{
			source = sources.Get(i);
			source.Get(INPUT_CONTAINER_FILTER_PRESET_NAME, filter);
			if (filter != actionPreset)
				continue;

			if (source.GetClassName() == INPUT_CONTAINER_SOURCE_COMBO_TYPE)
			{
				comboSources = source.GetObjectArray(INPUT_CONTAINER_SOURCES_NAME);
				if (!comboSources)
					continue;

				for (int j, comboCount = comboSources.Count(); j < comboCount; j++)
				{
					comboSource = comboSources.Get(j);
					if (!comboSource.Get(INPUT_CONTAINER_INPUT_NAME, input))
						continue;

					if (input.IsEmpty())
						continue;

					source = comboSource;
					break; // we dont support combos between different devices, thus first valid input binding is fine
				}
			}
			else
			{
				source.Get(INPUT_CONTAINER_INPUT_NAME, input);
				if (input.IsEmpty())
					continue;
			}

			matchingInputs++;
			if (matchingInputs == inputId)
				return source;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Translates device type to its prefix used for input bindings
	//! \param[in] device for which prefixes will be fetched
	//! \return array of strings that used by bindings of such devices
	static array<string> GetDeviceString(EInputDeviceType device)
	{
		switch (device)
		{
			case EInputDeviceType.GAMEPAD:
				return {UIConstants.DEVICE_GAMEPAD};

			case EInputDeviceType.JOYSTICK:
				return {UIConstants.DEVICE_JOYSTICK};

			case EInputDeviceType.TRACK_IR:
				return {UIConstants.DEVICE_TRACK_IR};
		}

		return {UIConstants.DEVICE_MOUSE, UIConstants.DEVICE_KEYBOARD};
	}

	//------------------------------------------------------------------------------------------------
	//! Draws a graph which visually represents in provided canvas an input curve
	//! \param[in] curveParentContainer container which will contain curve container and its dead zone value
	//! \param[in] canvas in which graph will be drawn
	//! \param[inout] drawCommands reference to the member array which will store draw calls for future frames
	//! \param[in] isHalfAxis determines if graph should be drawn only for the values from 0 to 1 if it is a half axis, or from -1, through 0, to 1 for full axis
	//! \param[in] innerDeadZoneValueName name of the container which will container the inner dead zone value
	static void DrawInputCurve(notnull BaseContainer curveParentContainer, notnull CanvasWidget canvas, notnull inout array<ref CanvasWidgetCommand> drawCommands, bool isHalfAxis, string innerDeadZoneValueName = INPUT_CONTAINER_DEADZONE_NAME)
	{
		float maxWidth;
		float maxHeight;
		canvas.GetScreenSize(maxWidth, maxHeight);
		if (maxWidth == 0 && maxHeight == 0)
			return; // postpone drawing until the widget is fully initialized and has its proper height/width

		maxHeight -= maxHeight * 0.01; // reduce by 1% as otherwise last row wont be visible
		const float minHeight = maxHeight * 0.01; // define minY for the top, so thicker lines are not cut when drawn in rowY = 0

		drawCommands.Clear();

		float innerDeadZone;
		curveParentContainer.Get(innerDeadZoneValueName, innerDeadZone);

		TVectorArray curve = GetInputCurve(curveParentContainer);
		if (!curve)
		{ // create empty curve if this binding doesnt have one defined
			curve = {};
		}
		else
		{ // strip edge values which are at begining and end of the graph, as these are added by default
			curve.RemoveItemOrdered(vector.Zero);
			curve.RemoveItemOrdered({1, 1, 0});
		}

		// draw the background grid
		LineDrawCommand backgroundLine;
		for (int i; i <= 100; i += 25)
		{
			// vertical
			backgroundLine = new LineDrawCommand();
			backgroundLine.m_iColor = Color.WHITE;
			backgroundLine.m_fWidth = BACKGROUND_VERTICAL_LINE_WIDTH;
			float x = i * 0.01 * maxWidth;
			backgroundLine.m_Vertices = {x, minHeight, x, maxHeight};
			drawCommands.Insert(backgroundLine);

			// horizontal
			backgroundLine = new LineDrawCommand();
			backgroundLine.m_iColor = Color.WHITE;
			backgroundLine.m_fWidth = BACKGROUND_HORIZONTAL_LINE_WIDTH;
			float y = i * 0.01 * (maxHeight - minHeight) + minHeight;
			backgroundLine.m_Vertices = {0, y, maxWidth, y};
			drawCommands.Insert(backgroundLine);
		}

		// create the actual graph line
		LineDrawCommand graph = new LineDrawCommand();
		graph.m_iColor = UIColors.CONTRAST_COLOR.PackToInt();
		graph.m_fWidth = FOREGROUND_LINE_WIDTH;
		graph.m_fOutlineWidth = FOREGROUND_OUTLINE_WIDTH;
		graph.m_iOutlineColor = Color.BLACK;
		graph.m_Vertices = {0, maxHeight}; // beginning of the graph at the bottom left corner of the canvas - {0, 0} is top left

		float innerDeadZoneWidth = Math.Floor(innerDeadZone * maxWidth); // round down to leave a bit more space for the edges
		float leftoverWidth = maxWidth - innerDeadZoneWidth;
		if (isHalfAxis)
		{
			if (innerDeadZone > 0)
			{ // the end of the input dead zone
				graph.m_Vertices.Insert(innerDeadZoneWidth);
				graph.m_Vertices.Insert(maxHeight);
			}

			foreach (vector coords : curve)
			{
				float x = coords[0] * leftoverWidth + innerDeadZoneWidth;

				graph.m_Vertices.Insert(x);
				graph.m_Vertices.Insert(((1 - coords[1]) * (maxHeight - minHeight) + minHeight));
			}
		}
		else
		{
			float innerDeadZoneStartX = leftoverWidth * 0.5;
			// left side of the graph
			for (int id = curve.Count() - 1; id >= 0; id--)
			{
				vector coords = curve[id];
				float x = (1 - coords[0]) * leftoverWidth * 0.5;

				graph.m_Vertices.Insert(x);
				graph.m_Vertices.Insert(coords[1] * maxHeight * 0.5 + maxHeight * 0.5);
			}

			float innerDeadZoneEndX = (maxWidth + innerDeadZoneWidth) * 0.5;
			if (innerDeadZone > 0)
			{ // the inner dead zone island
				float innerDeadZoneY = maxHeight * 0.5;
				graph.m_Vertices.Insert(innerDeadZoneStartX);
				graph.m_Vertices.Insert(innerDeadZoneY);

				graph.m_Vertices.Insert(innerDeadZoneEndX);
				graph.m_Vertices.Insert(innerDeadZoneY);
			}

			// right side of the graph
			foreach (vector coords : curve)
			{
				float x = coords[0] * leftoverWidth * 0.5 + innerDeadZoneEndX;

				graph.m_Vertices.Insert(x);
				graph.m_Vertices.Insert(((1 - coords[1]) * 0.5 * (maxHeight - minHeight) + minHeight));
			}
		}

		// the end of the graph
		graph.m_Vertices.Insert(maxWidth);
		graph.m_Vertices.Insert(minHeight);

		drawCommands.Insert(graph);

		canvas.SetDrawCommands(drawCommands);
	}

	//------------------------------------------------------------------------------------------------
	//! Calculates the input curve factor
	//! \param[in] x
	//! \param[in] y
	//! \param[in] max range of values
	//! \return input curve factor, or 0  if there is no curvature, of float.INFINITY if it is not possible to calculate curve factor for given values
	static float GetInputCurveFactor(float x, float y, int max = 10)
	{
		const float u = x / max;
		const float v = y / max;

		if (u <= 0 || u >= 1 || v <= 0 || v >= 1)
			return float.INFINITY;

		if (float.AlmostEqual(Math.AbsFloat(u - v), 0))
			return 0;

		float low = -max;
		float high = max;
		for (int i; i < max; i++)
		{
			float mid = (low + high) * 0.5;
			float factor;
			if (float.AlmostEqual(Math.AbsFloat(mid), 0))
				factor = u - v;
			else
				factor = (Math.Pow(Math.E, mid * u) - 1) / (Math.Pow(Math.E, mid) - 1) - v;

			if (factor > 0)
				low = mid;
			else
				high = mid;
		}

		return Math.Round((low + high) * 0.5 * 10) * 0.1;
	}

	//------------------------------------------------------------------------------------------------
	//! Adjusts the position of the knots to make space for the edge dead zone, which will be limited to 99%
	//! \param[inout] parentContainer which will contain the curve container
	//! \param[in] deadZoneValue normalized (0-1) width of edge dead zone (at the max input value), value will be automatically clamped to 0.99
	//! \return true when operation was succesfull, otherwise false
	static bool SetCurveDeadZone(notnull inout BaseContainer parentContainer, float deadZoneValue)
	{
		BaseContainer curveContainer;
		TVectorArray curve = GetInputCurve(parentContainer, curveContainer);
		if (!curve)
		{
			if (!parentContainer.SetObject(INPUT_CONTAINER_TRANSITION_CURVE_NAME, BaseContainerTools.CreateContainer(INPUT_CURVE_CONTAINER_TYPE).GetResource().ToBaseContainer()))
				return false;

			curveContainer = parentContainer.GetObject(INPUT_CONTAINER_TRANSITION_CURVE_NAME);
			if (!curveContainer)
				return false;

			curveContainer.Set(INPUT_CONTAINER_FILTER_SPLINE_TYPE_NAME, typename.EnumToString(ECurveType, ECurveType.Linear));
		}

		deadZoneValue = Math.Max(0.01, 1 - deadZoneValue);

		array<float> knots = {};
		array<float> values = {};
		curveContainer.Get(INPUT_CONTAINER_FILTER_CURVE_KNOTS_NAME, knots);

		int lastElementId = knots.Count() - 1;
		if (lastElementId > 0)
		{
			float oldValue = knots[lastElementId];
			if (oldValue == deadZoneValue)
				return false;

			float scale = deadZoneValue / oldValue;
			// scale knots positions to make space for the edge dead zone
			foreach (int id, float knot : knots)
			{
				if (id == lastElementId)
					continue; // skip last to give it exact value

				knots[id] = knot * scale;
			}

			knots[lastElementId] = deadZoneValue;
		}
		else // if there is nothing, then make a simple line
		{
			knots = {0, deadZoneValue};
			values = {0, 1};
			curveContainer.Set(INPUT_CONTAINER_FILTER_CURVE_VALUES_NAME, values);
		}

		curveContainer.Set(INPUT_CONTAINER_FILTER_CURVE_KNOTS_NAME, knots);

		if (!parentContainer.SetObject(INPUT_CONTAINER_TRANSITION_CURVE_NAME, curveContainer))
			return false;

		SCR_BaseContainerTools.CleanContainerData(parentContainer);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Generates an array of points which define a curve. Z value is not used
	//! \param[in] factor defining the curvature - factor > 0 slow (flat) start but quickly aproaches the max, 0 flat line, factor < 0 quick start but slowly(flattens) gets to the max
	//! \return array of points defining the curve where both x and y are in range of 0 to 1
	static notnull TVectorArray GenerateInputCurve(float factor)
	{
		TVectorArray output = {};
		const int max = 10;
		output.Resize(INPUT_CURVE_KNOTS_COUNT);
		foreach (int x, vector point : output)
		{
			point[0] = x * 0.1;
			if (factor == 0)
			{
				point[1] = point[0];
			}
			else
			{
				point[1] = (Math.Pow(Math.E, factor * x / max) - 1) / (Math.Pow(Math.E, factor) -1);
			}

			output[x] = point;
		}

		return output;
	}

	//------------------------------------------------------------------------------------------------
	//! Translates input curve into an vector array of points where Z value is not used
	//! \param[in] parentContainer which may contains the curve container
	//! \param[out] curveContainer that was found
	//! \return array of points used by input curve, or null if there is no curve defined
	static TVectorArray GetInputCurve(notnull BaseContainer parentContainer, out BaseContainer curveContainer = null)
	{
		curveContainer = parentContainer.GetObject(INPUT_CONTAINER_TRANSITION_CURVE_NAME);
		if (!curveContainer)
			return null;

		array<float> knots;
		curveContainer.Get(INPUT_CONTAINER_FILTER_CURVE_KNOTS_NAME, knots);
		array<float> values;
		curveContainer.Get(INPUT_CONTAINER_FILTER_CURVE_VALUES_NAME, values);

		if (knots.IsEmpty() || values.IsEmpty() || knots.Count() != values.Count())
		{
			Debug.Error("Broken curve detected! This curve has different amount of knots when comapred to the amount of values!");
			parentContainer.ClearVariable(INPUT_CONTAINER_TRANSITION_CURVE_NAME);
			curveContainer = parentContainer.GetObject(INPUT_CONTAINER_TRANSITION_CURVE_NAME);
			if (curveContainer)
			{
				if (knots.IsEmpty())
					curveContainer.Get(INPUT_CONTAINER_FILTER_CURVE_KNOTS_NAME, knots);
				else
					curveContainer.Set(INPUT_CONTAINER_FILTER_CURVE_KNOTS_NAME, knots);

				if (values.IsEmpty())
					curveContainer.Get(INPUT_CONTAINER_FILTER_CURVE_VALUES_NAME, values);
				else
					curveContainer.Set(INPUT_CONTAINER_FILTER_CURVE_VALUES_NAME, values);

				if (knots.Count() != values.Count())
					return null;
			}
			else
			{
				return null; // this should never happen, but if it does... then we will know where
			}
		}

		TVectorArray output = {};
		foreach (int i, float knot : knots)
		{
			output.Insert({knot, values[i], 0});
		}

		if (output.IsEmpty())
		{
			output.Insert(vector.Zero);
			output.Insert({1, 1, 0});
		}

		return output;
	}

	//------------------------------------------------------------------------------------------------
	static void GetDefaultAxisFilterValues(string inputBinding, out float multiplier, out float offset, out int inverted)
	{
		multiplier = 1;
		inverted = -1;

		if (inputBinding.Contains("+"))
		{
			inverted = 1;
		}
		else if (!inputBinding.Contains("-"))
		{
			multiplier = 0.5;
			offset = 1;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Fetches gamepad settings profile from user's engine settings
	static BaseContainer GetGamepadProfileContainer()
	{
		UserSettings settings = GetGame().GetEngineUserSettings();
		if (!settings)
			return null;

		BaseContainer settingsContainer = settings.GetModule(ENGINE_INPUT_DEVICE_USER_SETTINGS_NAME);
		if (!settingsContainer)
			return null;

		return settingsContainer.GetObject(ENGINE_SETTINGS_INPUT_GAMEPAD_PROFILE_NAME);
	}
}
