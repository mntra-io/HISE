/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"
#include "MainComponent.h"
#include "Exporter.h"



namespace hise {
namespace multipage {




struct CSSDebugger: public Component,
                    public Timer,
                    public PathFactory,
				    public simple_css::CSSRootComponent
{
    CSSDebugger(MainComponent& c):
      parent(c),
      codeDoc(doc),
      editor(codeDoc),
      powerButton("bypass", nullptr, *this)
    {
        root = parent.getMainState()->currentDialog;
        
        doc.setDisableUndo(true);
        setName("CSS Inspector");
        addAndMakeVisible(editor);
        
        editor.tokenCollection = new mcl::TokenCollection("CSS");
        editor.tokenCollection->setUseBackgroundThread(false);
        editor.setLanguageManager(new simple_css::LanguageManager(codeDoc));
        editor.setFont(GLOBAL_MONOSPACE_FONT().withHeight(12.0f));
        setSize(450, 800);
        setOpaque(true);
        startTimer(1000);

        css = DefaultCSSFactory::getTemplateCollection(DefaultCSSFactory::Template::PropertyEditor);
        laf = new simple_css::StyleSheetLookAndFeel(*this);
        hierarchy.setLookAndFeel(laf);
        addAndMakeVisible(hierarchy);
        addAndMakeVisible(powerButton);
        powerButton.setToggleModeWithColourChange(true);
        powerButton.setToggleStateAndUpdateIcon(true);
        hierarchy.setTextWhenNothingSelected("Select parent component");
        addAndMakeVisible(powerButton);

        hierarchy.onChange = [&]()
        {
	        auto pd = parentData[hierarchy.getSelectedItemIndex()];
            updateWithInspectorData(pd);
        };

        powerButton.onClick = [this]()
        {
            if(powerButton.getToggleState())
                this->startTimer(1000);
            else
                this->stopTimer();
            
            clear();
        };
    }
    
    MainComponent& parent;
    
    void clear()
    {
        if(root.getComponent() != nullptr)
            root->setCurrentInspectorData({});
    }
    
    HiseShapeButton powerButton;
    
    Path createPath(const String& url) const override
    {
        Path p;
        LOAD_EPATH_IF_URL("bypass", HiBinaryData::ProcessorEditorHeaderIcons::bypassShape);
        return p;
    }
    
    ~CSSDebugger()
    {
        clear();
    }
    
    void paint(Graphics& g) override
    {
        g.fillAll(Colour(0xFF222222));
    }

    simple_css::HeaderContentFooter::InspectorData createInspectorData(Component* c)
    {
	    auto b = root.getComponent()->getLocalArea(c, c->getLocalBounds()).toFloat();
        auto data = simple_css::FlexboxComponent::Helpers::dump(*c);

        simple_css::HeaderContentFooter::InspectorData id;
        id.first = b;
        id.second = data;
        id.c = c;

        return id;
    }

    void timerCallback() override
    {
        root = parent.getMainState()->currentDialog;
        
        if(root.getComponent() == nullptr)
            return;
        
        auto* target = Desktop::getInstance().getMainMouseSource().getComponentUnderMouse();

        bool change = false;

        if(target != nullptr && target->findParentComponentOfClass<simple_css::CSSRootComponent>() == root.getComponent())
        {
            currentTarget = target;
            change = true;
        }
        
        if(currentTarget.getComponent() != nullptr && change)
        {
            auto id = createInspectorData(currentTarget.getComponent());
            auto tc = id.c.getComponent();

            StringArray items;

            parentData.clear();

            while(tc != nullptr)
            {
                if(dynamic_cast<CSSRootComponent*>(tc) != nullptr)
                    break;

                parentData.add(createInspectorData(tc));
	            tc = tc->getParentComponent();
            }

            hierarchy.clear(dontSendNotification);

            int idx = 1;
            for(const auto& pd: parentData)
                hierarchy.addItem(pd.second, idx++);

            hierarchy.setText("", dontSendNotification);

            updateWithInspectorData(id);
        }
    }

    Array<simple_css::HeaderContentFooter::InspectorData> parentData;

    void updateWithInspectorData(const simple_css::HeaderContentFooter::InspectorData& id)
    {
	    root->setCurrentInspectorData(id);
        auto s = root->css.getDebugLogForComponent(id.c.getComponent());
        
        if(doc.getAllContent() != s)
            doc.replaceAllContent(s);
    }
    
    Component::SafePointer<Component> currentTarget = nullptr;
    
    void resized() override
    {
        auto b = getLocalBounds();
        auto topArea = b.removeFromTop(24);

        powerButton.setBounds(topArea.removeFromLeft(topArea.getHeight()).reduced(2));
        hierarchy.setBounds(b.removeFromBottom(32));
        editor.setBounds(b);
    }

    juce::CodeDocument doc;
    mcl::TextDocument codeDoc;
    mcl::TextEditor editor;

    ComboBox hierarchy;

    ScopedPointer<LookAndFeel> laf;

    Component::SafePointer<simple_css::HeaderContentFooter> root;
};

struct CreateCSSTemplate: public HardcodedDialogWithState
{
    CreateCSSTemplate():
      HardcodedDialogWithState()
    {
        setSize(600, 350);
        
    }
    
    
    Dialog* createDialog(State& state) override;
};

using namespace juce;
Dialog* CreateCSSTemplate::createDialog(State& state)
{
    DynamicObject::Ptr fullData = new DynamicObject();
    fullData->setProperty(mpid::StyleData, JSON::parse(R"({"Font": "Lato", "BoldFont": "<Sans-Serif>", "FontSize": 18.0, "bgColour": 4281545523, "codeBgColour": 864585864, "linkBgColour": 8947967, "textColour": 4294967295, "codeColour": 4294967295, "linkColour": 4289374975, "tableHeaderBgColour": 864059520, "tableLineColour": 864059520, "tableBgColour": 864059520, "headlineColour": 4287692721, "UseSpecialBoldFont": false})"));
    fullData->setProperty(mpid::LayoutData, JSON::parse(R"({"DialogWidth": 700, "DialogHeight": 500, "StyleSheet": "ModalPopup"})"));
    fullData->setProperty(mpid::Properties, JSON::parse(R"({"Header": "Create Stylesheet", "Subtitle": "Subtitle", "Image": "", "ProjectName": "MyProject", "Company": "MyCompany", "Version": "1.0.0", "BinaryName": "My Binary", "UseGlobalAppData": false, "Icon": ""})"));
    using namespace factory;
    auto mp_ = new Dialog(var(fullData.get()), state, false);
    
    auto& mp = *mp_;
    
//    dynamic_cast<simple_css::HeaderContentFooter*>(mp_)->showEditor();
    
    auto& List_0 = mp.addPage<factory::List>({
      { mpid::Foldable, 0 },
      { mpid::Folded, 0 }
    });

    auto& file_1 = List_0.addChild<factory::FileSelector>({
      { mpid::Text, "CSS File" },
      { mpid::ID, "file" },
      { mpid::Enabled, 1 },
      { mpid::Code, "// initialisation, will be called on page load\nConsole.print(\"init\");\n\nelement.onValue = function(value)\n{\n    // Will be called whenever the value changes\n    Console.print(value);\n}\n" },
      { mpid::UseInitValue, 0 },
      { mpid::Required, 1 },
      { mpid::Wildcard, "*.css" },
      { mpid::SaveFile, 1 },
      { mpid::Help, "The CSS file to be created.  \n> it's highly recommended to pick a file that is relative to the `json` file you're using to create this dialog!." },
      { mpid::Directory, 0 },
      { mpid::UseOnValue, 0 }
    });

    auto& ChoiceId_2 = List_0.addChild<factory::Choice>({
      { mpid::Text, "Template" },
      { mpid::ID, "templateIndex" },
      { mpid::Enabled, 1 },
      { mpid::Code, "// initialisation, will be called on page load\nConsole.print(\"init\");\n\nelement.onValue = function(value)\n{\n    // Will be called whenever the value changes\n    Console.print(value);\n}\n" },
      { mpid::InitValue, "Dark" },
      { mpid::UseInitValue, 1 },
      { mpid::Custom, 0 },
      { mpid::ValueMode, "Index" },
      { mpid::Help, "The template to be used by the style sheet." },
      { mpid::Items, DefaultCSSFactory::getTemplateList() },
      { mpid::UseOnValue, 0 }
    });

    auto& ButtonId_3 = List_0.addChild<factory::Button>({
      { mpid::Text, "Add as asset" },
      { mpid::ID, "addAsAsset" },
      { mpid::Enabled, 1 },
      { mpid::Code, "// initialisation, will be called on page load\nConsole.print(\"init\");\n\nelement.onValue = function(value)\n{\n    // Will be called whenever the value changes\n    Console.print(value);\n}\n" },
      { mpid::InitValue, "true" },
      { mpid::UseInitValue, 1 },
      { mpid::Help, "Whether to add this file as asset to the current dialog." },
      { mpid::Required, 0 },
      { mpid::Trigger, 0 },
      { mpid::UseOnValue, 0 }
    });

    // Custom callback for page List_0
    List_0.setCustomCheckFunction([](Dialog::PageBase* b, const var& obj)
    {
        auto targetFile = File(obj["file"].toString());
        
        return Result::ok();

    });
    
    return mp_;
}
} // namespace multipage
} // namespace hise



void MainComponent::build()
{
    using namespace multipage;
    using namespace factory;
	createDialog(File());
}

PopupMenu MainComponent::getMenuForIndex(int topLevelMenuIndex, const String&)
{
	PopupMenu m;

	if(topLevelMenuIndex == 0) // File
	{
		m.addItem(CommandId::FileNew, "New file");

		m.addItem(CommandId::FileLoad, "Load file");

		PopupMenu r;
		fileList.createPopupMenuItems(r, CommandId::FileRecentOffset, false, false);
		m.addSubMenu("Recent files", r);
		m.addItemWithShortcut(CommandId::FileSave, "Save file",  KeyPress('s', ModifierKeys::commandModifier, 's'), currentFile.existsAsFile());
		m.addItem(CommandId::FileSaveAs, "Save file as");
		m.addItem(CommandId::FileExportAsProjucerProject, "Export as Projucer project");
		m.addSeparator();
        m.addItem(CommandId::FileCreateCSS, "Create CSS stylesheet");
        m.addSeparator();
		m.addItem(CommandId::FileQuit, "Quit");
	}
	if(topLevelMenuIndex == 1) // Edit
	{
        if(c == nullptr)
        {
            m.addItem(123123123, "Disabled in hardcoded mode", false);
            return m;
        }
        
        m.addItemWithShortcut(CommandId::EditUndo, "Undo",  KeyPress('z', ModifierKeys::commandModifier, 's'), c->getUndoManager().canUndo());
        m.addItemWithShortcut(CommandId::EditRedo, "Redo",  KeyPress('y', ModifierKeys::commandModifier, 's'), c->getUndoManager().canRedo());
        m.addSeparator();
		m.addItemWithShortcut(CommandId::EditToggleMode, "Toggle Edit mode", KeyPress(KeyPress::F4Key), c->isEditModeAllowed(), c->isEditModeEnabled());
		m.addItemWithShortcut(CommandId::EditRefreshPage, "Refresh current page", KeyPress(KeyPress::F5Key));
		m.addSeparator();
		m.addItem(CommandId::EditClearState, "Clear state object");
        m.addItem(CommandId::EditAddPage, "Add page", c->isEditModeAllowed());
		m.addItem(CommandId::EditRemovePage, "Remove current page", c->isEditModeAllowed() && c->getNumPages() > 1);
	}
	if(topLevelMenuIndex == 2) // View
	{
		m.addItem(CommandId::ViewShowDialog, "Show dialog");
        
        if(c != nullptr)
        {
            m.addItem(CommandId::ViewShowCpp, "Show C++ code", c->isEditModeAllowed());
            m.addItem(CommandId::ViewShowJSON, "Show JSON editor", c->isEditModeAllowed());
        }

		m.addItem(CommandId::ViewShowConsole, "Show console", true, console->isVisible());
        m.addItem(CommandId::ViewShowCSSDebugger, "Show CSS Debugger", true, currentInspector != nullptr);
	}
	if(topLevelMenuIndex == 3) // Help
	{
		m.addItem(CommandId::HelpAbout, "About");
		m.addItem(CommandId::HelpVersion, "Version");
	}

	return m;
}

bool MainComponent::keyPressed(const KeyPress& key)
{
	if(key.getKeyCode() == KeyPress::F4Key && c != nullptr)
	{
		menuItemSelected(CommandId::EditToggleMode, 0);
		return true;
	}
	if(key.getKeyCode() == KeyPress::F5Key && c != nullptr)
	{
		menuItemSelected(CommandId::EditRefreshPage, 0);
		return true;
	}
	if(key.getKeyCode() == 's' && key.getModifiers().isCommandDown())
	{
		menuItemSelected(CommandId::FileSave, 0);
		return true;
	}
    
    return false;
}



void MainComponent::menuItemSelected(int menuItemID, int)
{
	if(menuItemID >= CommandId::FileRecentOffset)
	{
		auto f = fileList.getFile(menuItemID - CommandId::FileRecentOffset);
		createDialog(f);
		return;
	}

	switch((CommandId)menuItemID)
	{
	case FileNew:
		createDialog(File());
		break;
 
    case FileCreateCSS:
        addAndMakeVisible(modalDialog = new ModalDialog(*this, new library::CreateCSSFromTemplate()));

        break;
	case FileLoad:
		{
			FileChooser fc("Open JSON file", File(), "*.json");
			if(fc.browseForFileToOpen())
			{
				createDialog(fc.getResult());
			}

			break;
		}
	case FileSave:
		{
            c->getState().callEventListeners("save", {});
            
			if(currentFile.existsAsFile())
				currentFile.replaceWithText(JSON::toString(c->exportAsJSON()));

			setSavePoint();

			break;
		}
	case FileSaveAs:
		{
			FileChooser fc("Save JSON file", File(), "*.json");

			if(fc.browseForFileToSave(true))
			{
				currentFile = fc.getResult();
                
                c->getState().callEventListeners("save", {});
                
				currentFile.replaceWithText(JSON::toString(c->exportAsJSON()));
				rt.currentRootDirectory = currentFile.getParentDirectory();
				setSavePoint();
			}
                
			break;
		}
	case FileExportAsProjucerProject:
	{
		addAndMakeVisible(modalDialog = new ModalDialog(*this, new multipage::library::ProjectExporter(rt.currentRootDirectory, rt)));
		
		break;
	}
	case FileQuit: JUCEApplication::getInstance()->systemRequestedQuit(); break;
	case EditClearState: rt.globalState.getDynamicObject()->clear(); break;
	case EditUndo: c->getUndoManager().undo(); c->refreshCurrentPage(); break;
	case EditRedo: c->getUndoManager().redo(); c->refreshCurrentPage(); break;
	case EditToggleMode: 
		c->setEditMode(!c->isEditModeEnabled());
		c->repaint();
        resized();
		break;
	case EditRefreshPage: c->refreshCurrentPage(); tree->setRoot(*c); resized(); break;
    case EditAddPage: c->addListPageWithJSON(); break;
	case EditRemovePage: c->removeCurrentPage(); break;
	case ViewShowDialog:
		{
			if(manualChange && AlertWindow::showOkCancelBox(MessageBoxIconType::QuestionIcon, "Apply manual JSON changes?", "Do you want to reload the dialog after the manual edit?"))
			{
				stateViewer.setVisible(false);
				var obj;
				auto ok = JSON::parse(doc.getAllContent(), obj);

				if(ok.wasOk())
				{
                    c = nullptr;
					addAndMakeVisible(c = new multipage::Dialog(obj, rt));
					resized();

                    c->setFinishCallback([](){
    					JUCEApplication::getInstance()->systemRequestedQuit();
				    });

					c->showFirstPage();
				}
			}

			c->setVisible(true);
			stateViewer.setVisible(false);
			break;
		}
	case ViewShowJSON:
		manualChange = false;
		doc.removeListener(this);
		c->setVisible(false);
		stateViewer.setVisible(true);
        c->getState().callEventListeners("save", {});
		doc.replaceAllContent(JSON::toString(c->exportAsJSON()));
		doc.addListener(this);
		break;
	case ViewShowConsole:
		console->setVisible(!console->isVisible());
		resized();
        menuItemsChanged();
		break;
	case ViewShowCpp:
		{
#if HISE_MULTIPAGE_INCLUDE_EDIT
            c->getState().callEventListeners("save", {});
			multipage::CodeGenerator cg(rt.currentRootDirectory, "MyClass", c->exportAsJSON());
			manualChange = false;
            cg.setUseRawMode(true),
			doc.removeListener(this);
			c->setVisible(false);
			stateViewer.setVisible(true);

			MemoryOutputStream mos;
            cg.write(mos, multipage::CodeGenerator::FileType::DialogHeader, nullptr);
			cg.write(mos, multipage::CodeGenerator::FileType::DialogImplementation, nullptr);
			doc.replaceAllContent(mos.toString());
#endif
			break;
		}
    case ViewShowCSSDebugger:
    {
        if(currentInspector != nullptr)
        {
            leftTab.remove(currentInspector);
            currentInspector = nullptr;
        }
        else
        {
            currentInspector = leftTab.add(new CSSDebugger(*this), 0.5);
            leftTab.setInitProportions({0.5, 0.5});
        }
        
        menuItemsChanged();
        
        resized();
        break;
    }
	case HelpAbout: break;
	case HelpVersion: break;
	default: ;
	}
}

void MainComponent::createDialog(const File& f)
{
	var obj;

	if(f.existsAsFile())
	{
		auto ok = JSON::parse(f.loadFileAsString(), obj);

		if(ok.failed())
		{
			c->logMessage(multipage::MessageType::Navigation, "Error at parsing JSON: " + ok.getErrorMessage());
			return;
		}
		
		fileList.addFile(f);
		rt.currentRootDirectory = f.getParentDirectory();

		autosaver = new Autosaver(f, rt);
	}

	currentFile = f;

    c = nullptr;
    hardcodedDialog = nullptr;

	rt.reset(obj);

	addAndMakeVisible(c = new multipage::Dialog(obj, rt));

	c->showFirstPage();
	
	if(f.existsAsFile())
		c->logMessage(multipage::MessageType::Navigation, "Load file " + f.getFullPathName());

	
	assetManager->listbox.updateContent();
	assetManager->repaint();
	assetManager->resized();

    c->setFinishCallback([](){
    	JUCEApplication::getInstance()->systemRequestedQuit();
    });

	resized();

	tree->setRoot(*c);

	c->toBack();

	setSavePoint();
}

//==============================================================================
MainComponent::MainComponent():
  rt({}),
  doc(),
  stateDoc(doc),
  stateViewer(stateDoc),
  menuBar(this),
  tooltips(this),
  leftTab(false),
  rightTab(true)
{
#if 0
	simple_css::Editor::showEditor(this, [this](simple_css::StyleSheet::Collection& c)
	{
		auto d = currentSideTabDialog.getComponent();

		if(d != nullptr)
		{
			d->update(c);
		}
	});
#endif

	TopLevelWindowWithKeyMappings::loadKeyPressMap();

	mcl::TextEditor::initKeyPresses(this);

    addAndMakeVisible(leftTab);
	addAndMakeVisible(rightTab);

	auto watchTable = new ScriptWatchTable();

    tree = dynamic_cast<Tree*>(leftTab.add(new Tree(), 0.5));
	rightTab.add(new SideTab(), 0.5);
	rightTab.add(watchTable, 0.3);
	assetManager = dynamic_cast<AssetManager*>(rightTab.add(new AssetManager(rt), 0.2));

//	addAndMakeVisible(tree = ComponentWithEdge::wrap(new hise::multipage::Tree(), ResizableEdgeComponent::rightEdge, 360));
	addAndMakeVisible(console = ComponentWithEdge::wrap(new hise::multipage::EventConsole(rt), ResizableEdgeComponent::topEdge, 200));

	watchTable->setHolder(&rt);

	Array<var> l({ var("Name"), var("Value")});
	watchTable->restoreColumnVisibility(var(l));
	watchTable->setRefreshRate(200, 1);
	watchTable->setUseParentTooltipClient(true);
	watchTable->setName("State variables");

    LookAndFeel::setDefaultLookAndFeel(&plaf);

    auto settings = JSON::parse(getSettingsFile());

    if(auto so = settings.getDynamicObject())
        fileList.restoreFromString(so->getProperty("RecentFiles").toString());

#if JUCE_MAC
    setMacMainMenu (this);
#else
    menuBar.setLookAndFeel(&plaf);
    addAndMakeVisible(menuBar);
#endif
    build();
	startTimer(3000);

	

    addChildComponent(stateViewer);

#if JUCE_WINDOWS
    
    setSize(1300, 900);
	//setSize (2560, 1080);
#else
    setSize(1300, 720);
#endif
}

MainComponent::~MainComponent()
{
	TopLevelWindowWithKeyMappings::saveKeyPressMap();

#if JUCE_MAC
    setMacMainMenu (nullptr);
#endif
    
    auto f = fileList.toString();

	auto no = new DynamicObject();
    no->setProperty("RecentFiles", f);
    getSettingsFile().replaceWithText(JSON::toString(var(no), true));

#if JUCE_WINDOWS
	//context.detach();
#endif
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
	g.fillAll(Colour(0xFF222222));

    if(c != nullptr)
    {
        g.setColour(c->getStyleData().backgroundColour);
        g.fillRect(c->getBoundsInParent());
        g.setColour(Colours::white.withAlpha(0.2f));
        g.drawRect(c->getBoundsInParent().reduced(-1.0f), 1.0f);
    }
    

	auto b = getLocalBounds().toFloat();

	if(c != nullptr && tree != nullptr)
	{
		b.removeFromLeft(tree->getWidth());
		b.removeFromRight(rightTab.getWidth());

		Colour c = JUCE_LIVE_CONSTANT(Colour(0x22000000));

		g.setGradientFill(ColourGradient(c, b.getX(), 0.0f, Colours::transparentBlack, b.getX() + 5.0f, 0.0f, false));
		g.fillRect(b.removeFromLeft(20));

		g.setGradientFill(ColourGradient(c, b.getRight(), 0.0f, Colours::transparentBlack, b.getRight() - 5.0f, 0.0f, false));
		g.fillRect(b.removeFromRight(20));
	}
}

void MainComponent::resized()
{
    auto b = getLocalBounds();

	if(b.isEmpty())
		return;

#if !JUCE_MAC
    menuBar.setBounds(b.removeFromTop(24));
	b.removeFromTop(5);
#endif

    if(c != nullptr)
    {
		auto rb = b.removeFromRight(rightTab.getWidth());

		rightTab.setBounds(rb);
		
        leftTab.setBounds(b.removeFromLeft(leftTab.getWidth()));

		if(console->isVisible())
			console->setBounds(b.removeFromBottom(console->getHeight()));

		b.removeFromLeft(2);
		b.removeFromRight(2);

		auto pb = b.reduced(20);

		auto dialogBounds = c->positionInfo.getBounds(pb);

		if(dialogBounds.getWidth() > pb.getWidth() ||
		   dialogBounds.getHeight() > pb.getHeight())
		{
			auto wratio = (float)dialogBounds.getWidth() / (float)pb.getWidth();
			auto hratio = (float)dialogBounds.getHeight() / (float)pb.getHeight();

			auto ratio = jmax(wratio, hratio);

			auto topLeft = dialogBounds.getTopLeft();

			topLeft.applyTransform(AffineTransform::scale(1.0f / ratio));

			
			c->setTransform(AffineTransform::scale(1.0f / ratio));

			auto parentArea = dialogBounds.transformedBy(c->getTransform().inverted());

			auto width = dialogBounds.getWidth();
			auto height = dialogBounds.getHeight();

		    c->setBounds (parentArea.getCentreX() - width / 2,
		               parentArea.getCentreY() - height / 2,
		               width, height);

			//c->centreWithSize(dialogBounds.getWidth(), dialogBounds.getHeight());

		}
		else
		{
			c->setTransform(AffineTransform());
			c->setBounds(dialogBounds);
		}
		
    }
	else if(console->isVisible())
		console->setBounds(b.removeFromBottom(200));
        

	if(hardcodedDialog != nullptr)
		hardcodedDialog->setBounds(hardcodedDialog->dialog->positionInfo.getBounds(b));

    stateViewer.setBounds(b);
    
    repaint();
}

void MainComponent::timerCallback()
{
	if(c == nullptr)
		return;

    c->getUndoManager().beginNewTransaction();

	const int64 thisHash = JSON::toString(c->exportAsJSON(), true).hashCode64();

    if(firstAfterSave)
    {
	    prevHash = thisHash;
        firstAfterSave = false;
    }
    else
    {
	    modified |= thisHash != prevHash;
		prevHash = thisHash;
    }
}

void MainComponent::setSavePoint()
{
	modified = false;
    firstAfterSave = true;
	
	menuItemsChanged();
}



