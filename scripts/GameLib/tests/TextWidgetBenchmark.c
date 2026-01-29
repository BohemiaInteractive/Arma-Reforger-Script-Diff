class TextWidgetBenchmark : Managed
{
	ref Widget root;
	ref array<ref Widget> texts;
	
	//! Call once to init the test
	void InitTest(int textCount, int richTextCount, bool widgetsVisible)
	{
		texts = new array<ref Widget>();
		const string loremIpsum = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Purus sit amet volutpat consequat mauris. Mauris sit amet massa vitae tortor condimentum lacinia. Nibh tellus molestie nunc non. Cum sociis natoque penatibus et magnis. A condimentum vitae sapien pellentesque habitant morbi. Blandit massa enim nec dui nunc mattis enim. Sapien eget mi proin sed libero enim. Ultrices eros in cursus turpis massa tincidunt dui ut ornare. Ornare quam viverra orci sagittis. Augue ut lectus arcu bibendum at varius. Nulla posuere sollicitudin aliquam ultrices sagittis orci a scelerisque. Fames ac turpis egestas maecenas pharetra. Sem viverra aliquet eget sit amet tellus cras. Pellentesque habitant morbi tristique senectus et netus et malesuada. Velit ut tortor pretium viverra. Massa tincidunt nunc pulvinar sapien et ligula ullamcorper. At erat pellentesque adipiscing commodo elit at imperdiet. Quisque egestas diam in arcu cursus.";
		WorkspaceWidget workspace = g_Game.GetWorkspace();
		root = workspace.CreateWidget(WidgetType.FrameWidgetTypeID, WidgetFlags.VISIBLE, new Color(1, 1, 1, 1), 0);
		vector initSize = "256 256 0";
		vector sizeIncrease = "5 5 0";
		vector initPos = "0 0 0";
		vector posIncrease = "5 5 0";
		for (int i = 0; i < textCount; ++i)
		{
			TextWidget text = TextWidget.Cast(workspace.CreateWidget(WidgetType.TextWidgetTypeID, 0, new Color(1, 1, 1, 1), 0, root));
			text.SetText(loremIpsum);
			text.SetVisible(widgetsVisible);
			//text.SetTextWrapping(true);
			//text.SetMinFontSize(10);
			
			FrameSlot.SetAnchorMin(text, 0, 0);
			FrameSlot.SetAnchorMax(text, 0, 0);
			FrameSlot.SetPos(text, initPos[0] + i * posIncrease[0], initPos[1] + i * posIncrease[1]);
			FrameSlot.SetSize(text, initSize[0] + i * sizeIncrease[0], initSize[1] + i * sizeIncrease[1]);
			texts.Insert(text);
		}
		
		for (int i = 0; i < richTextCount; ++i)
		{
			RichTextWidget text = RichTextWidget.Cast(workspace.CreateWidget(WidgetType.RichTextWidgetTypeID, WidgetFlags.WRAP_TEXT, new Color(1, 1, 1, 1), 0, root));
			text.SetVisible(widgetsVisible);
			text.SetText(loremIpsum);
			text.SetMinFontSize(10);
			
			FrameSlot.SetAnchorMin(text, 0, 0);
			FrameSlot.SetAnchorMax(text, 0, 0);
			FrameSlot.SetPos(text, initPos[0] + i * posIncrease[0], initPos[1] + i * posIncrease[1]);
			FrameSlot.SetSize(text, initSize[0] + i * sizeIncrease[0], initSize[1] + i * sizeIncrease[1]);
		}

		Print("Text benchmark initialized");
	}

	
	int x = 0;
	//! Call once per frame
	void Update()
	{
		x++;
		int p = x % 2;
		FrameSlot.SetAnchorMin(root, 0, 0);
		FrameSlot.SetAnchorMax(root, 1, 1);
		FrameSlot.SetOffsets(root, p, 0, 0, 0);
	}
}