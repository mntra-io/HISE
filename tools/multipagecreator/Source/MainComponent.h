/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DialogLibrary.h"

namespace hise
{
namespace multipage
{
using namespace juce;

struct Autosaver: public Timer
{
	Autosaver(const File& f_, State& s):
      f(f_),
      state(s)
	{
		startTimer(30000);
	}

    void timerCallback() override
	{
        if(state.currentDialog != nullptr)
        {
	        auto json = JSON::toString(state.currentDialog->exportAsJSON());

            auto newIndex = ++index % 5;
			auto autosaveFile = f.getSiblingFile("Autosave_" + String(newIndex)).withFileExtension(".json");
            autosaveFile.replaceWithText(json);
        }
	}

    File f;
    State& state;
    int index = 0;
};

struct ComponentWithEdge: public Component,
						  public ComponentBoundsConstrainer
{
    struct LAF: public LookAndFeel_V4
    {
        void drawStretchableLayoutResizerBar (Graphics &g, int w, int h, bool isVerticalBar, bool isMouseOver, bool isMouseDragging) override
        {
            

            float alpha = 0.0f;

            if(isMouseOver)
                alpha += 0.4f;

            if(isMouseDragging)
                alpha += 0.4f;

	        g.setColour(Colour(SIGNAL_COLOUR).withAlpha(alpha));
            g.fillRect(1, 1, w-2, h-2);

            g.setColour(Colours::white.withAlpha(0.05f));
            g.drawRect(0, 0, w, h, 1);
        }
    } laf;

    ComponentWithEdge(Component* content_, ResizableEdgeComponent::Edge edge_):
      resizer(this, this, edge_),
      edge(edge_),
      content(content_)
    {
        setName(content->getName());
        resizer.setLookAndFeel(&laf);
	    addAndMakeVisible(content);
        addAndMakeVisible(resizer);
    };

    void paint(Graphics& g) override
    {
        if(useTitle && content != nullptr)
        {
	        g.setColour(Colour(0xFF161616));
	        g.fillRect(titleArea);
	        GlobalHiseLookAndFeel::drawFake3D(g, titleArea);
	        g.setColour(Colours::white.withAlpha(0.7f));
		    g.setFont(GLOBAL_BOLD_FONT());
	        g.drawText(content->getName(), titleArea.toFloat(), Justification::centred);
        }
    }

    void checkBounds (Rectangle<int>& bounds, const Rectangle<int>& previousBounds, const Rectangle<int>& limits, bool , bool , bool , bool ) override
    {
        if(bounds.getHeight() < 30 || bounds.getWidth() < 120)
            bounds = previousBounds;
    }


    template <typename T> T* getContent() { return dynamic_cast<T*>(content.get()); }

    static ComponentWithEdge* wrap(Component* c, ResizableEdgeComponent::Edge e, int initialSize)
    {
	    auto ce = new ComponentWithEdge(c, e);

        if(e == ResizableEdgeComponent::topEdge || e == ResizableEdgeComponent::bottomEdge)
            ce->setSize(100, initialSize);
        else
            ce->setSize(initialSize, 100);

        return ce;
    }

    bool useTitle = true;

    void resized() override
    {
        auto b = getLocalBounds();

	    switch(edge)
	    {
	    case ResizableEdgeComponent::leftEdge: resizer.setBounds(b.removeFromLeft(5)); break;
	    case ResizableEdgeComponent::rightEdge: resizer.setBounds(b.removeFromRight(5)); break;
	    case ResizableEdgeComponent::topEdge: resizer.setBounds(b.removeFromTop(5)); break;
	    case ResizableEdgeComponent::bottomEdge: resizer.setBounds(b.removeFromBottom(5)); break;
	    default: ;
	    }

        if(useTitle)
			titleArea = b.removeFromTop(20);

        content->setBounds(b);
        content->resized();

        if(auto c = dynamic_cast<Component*>(findParentComponentOfClass<ComponentWithSideTab>()))
        {
	        c->resized();
			c->repaint();
        }

        repaint();
    }

    Rectangle<int> titleArea;

    ScopedPointer<Component> content;
    const ResizableEdgeComponent::Edge edge;
	ResizableEdgeComponent resizer;
};

struct RightTab: public ComponentWithEdge
{
    struct ListComponent: public Component
    {
        void resized() override
        {
            ScopedValueSetter<bool> svs(recursion, true);

	        auto b = getLocalBounds();

	        if(b.isEmpty())
	            return;

            if(list.size() == initProportions.size())
            {
                int idx = 0;

                auto h = (double)getHeight();

	            for(auto l: list)
				{
		            auto thisBounds = l == list.getLast() ? b.withHeight(b.getHeight() + 5) : b.removeFromTop(h * initProportions[idx++]);
					l->setBounds(thisBounds);
				}

                initProportions.clear();
            }
            else
            {
	            for(auto l: list)
				{
		            auto thisBounds = l == list.getLast() ? b.withHeight(b.getHeight() + 5) : b.removeFromTop(l->getHeight());
					l->setBounds(thisBounds);
				}
            }
        }

        bool recursion = false;
        Array<double> initProportions;
	    OwnedArray<ComponentWithEdge> list;
    };

    struct Watcher: public ComponentMovementWatcher
    {
        Watcher(Component* child_, RightTab& parent_):
          ComponentMovementWatcher(child_),
          parent(parent_),
          child(child_)
        {
	        
        }

        Component* child;
        RightTab& parent;

        void componentPeerChanged() override {};

	    void componentMovedOrResized (bool wasMoved, bool wasResized) override
	    {
            if(!parent.getContent<ListComponent>()->recursion)
				parent.resized();
	    }

	    void componentVisibilityChanged() override
	    {
            if(!parent.getContent<ListComponent>()->recursion)
				parent.resized();
	    }
    };

    OwnedArray<Watcher> watchers; 

    

    RightTab(bool isRight):
      ComponentWithEdge(new ListComponent(), isRight ? ResizableEdgeComponent::leftEdge : ResizableEdgeComponent::rightEdge)
    {
        useTitle = false;
	    setSize(320, 0);
    };

    void remove(Component* componentToRemove)
    {
        auto l = getContent<ListComponent>();
        
        int idx = 0;
        
        for(auto c: l->list)
        {
            if(c->getContent<Component>() == componentToRemove)
            {
                break;
            }
            idx++;
        }
        
        l->list.remove(idx);
        watchers.remove(idx);
        l->initProportions.remove(idx);
        l->resized();
        resized();
    }
    
    void setInitProportions(Array<double> prop)
    {
        auto l = getContent<ListComponent>();
        jassert(l->list.size() == prop.size());
        l->initProportions = prop;
        l->resized();
    }
    
	Component* add(Component* newComponent, double initProportion)
	{
        auto nc = new ComponentWithEdge(newComponent, ResizableEdgeComponent::bottomEdge);
        auto l = getContent<ListComponent>();
		l->list.add(nc);
        l->addAndMakeVisible(nc);
        watchers.add(new Watcher(nc, *this));
        l->initProportions.add(initProportion);
        return newComponent;
	}

    template <typename T> T* getChild(int index)
	{
		return getContent<ListComponent>()->list[index]->getContent<T>();
	}
};

/** TODO:
 *
 * - add Add / Remove button OK
 * - allow dragging of pages
 * - add asset browser
 * - fix deleting (set enablement for add/delete correctly)
 */
struct Tree: public Component,
			 public DragAndDropContainer,
			 public PathFactory
{

    
    static var getParentRecursive(const var& parent, const var& childToLookFor)
    {
	    if(parent[mpid::Children].isArray() || parent.isArray())
	    {
            auto ar = parent.getArray();
            if(ar == nullptr)
                ar = parent[mpid::Children].getArray();

		    for(auto& v: *ar)
		    {
			    if(childToLookFor.getObject() == v.getObject())
                    return parent;

                auto cc = getParentRecursive(v, childToLookFor);

                if(cc.isObject())
                    return cc;
		    }
	    }

        return var();
    }

    static bool containsRecursive(const var& parent, const var& child)
    {
        if(parent == child)
            return true;

	    if(parent[mpid::Children].isArray())
	    {
		    for(const auto& v: *parent[mpid::Children].getArray())
		    {
			    if(containsRecursive(v, child))
                    return true;
		    }
	    }

        return false;
    }

    int getPageIndex(const var& child) const
    {
	    jassert(rootObj.isArray());

        for(int i = 0; i < rootObj.size(); i++)
        {
	        if(containsRecursive(rootObj[i], child))
                return i;
        }

        return -1;
    }

    bool removeFromParent(const var& child)
    {
        auto parent = getParentRecursive(rootObj, child);
        
	    if(parent[mpid::Children].isArray())
	    {
			return parent[mpid::Children].getArray()->removeAllInstancesOf(child) > 0;
	    }

        return false;
    }
    
    struct PageItem: public TreeViewItem
    {
        PageItem(Tree& root_, const var& obj_, int idx, bool isPage_):
          obj(obj_),
          index(idx),
          root(root_),
          isPage(isPage_)
        {
            isVisible = root_.currentDialog->findPageBaseForInfoObject(obj) != nullptr;
	        setLinesDrawnForSubItems(true);
            
            auto type = obj[mpid::Type].toString();
            
            if(type.isNotEmpty())
            {
                icon = root.iconFactory.createPath(type);
                typeColour = root.iconFactory.getColourForCategory(type);
            }
            
        }

        bool isRoot() const
        {
	        return root.tree.getRootItem() == this;
        }

        bool isPageData(const var& obj) const
        {
	        for(const auto& v: *root.currentDialog->getPageListVar().getArray())
	        {
		        if(v.getDynamicObject() == obj.getDynamicObject())
                    return true;
	        }

            return false;
        }

        bool isAction() const
        {
	        return typeColour == Colour(0xFF9CC05B);
        }
        
        Colour typeColour;

        Path icon;
        
        bool isVisible;

        bool edited = false;

        bool isEdited() const
        {
            return root.currentDialog->mouseSelector.selection.isSelected(obj);
        }

        int getItemHeight() const override
        {
	        return isPage ? 26 : 24;
        }

        var getDragSourceDescription() override
        {
	        return obj;
        } 

    	String getTooltip() override
        {
	        String s;

            s << obj[mpid::Type].toString();

            auto id = obj[mpid::ID].toString();

            if(id.isNotEmpty())
				s << " (" << id << ')';
            
            return s;
        }

		bool isInterestedInDragSource (const DragAndDropTarget::SourceDetails& dragSourceDetails) override
        {
            if(isPageData(dragSourceDetails.description))
            {
	            return isRoot();
            }

            return dragSourceDetails.description[mpid::Type].isString() && obj[mpid::Children].isArray();
        }

        void itemDropped (const DragAndDropTarget::SourceDetails& dragSourceDetails, int insertIndex) override
        {
            if(isPageData(dragSourceDetails.description))
            {
	            auto& ar = *(root.currentDialog->getPageListVar().getArray());

                if(ar.size() == 1)
                    return;

                auto oldIndex = ar.indexOf(dragSourceDetails.description);

                ar.remove(oldIndex);
                ar.insert(insertIndex, dragSourceDetails.description);

                root.currentDialog->rebuildPagesFromJSON();
                root.setRoot(*root.currentDialog);
                
                return;
            }

            if(root.removeFromParent(dragSourceDetails.description))
            {
	            obj[mpid::Children].getArray()->insert(insertIndex, dragSourceDetails.description);

                root.currentDialog->refreshCurrentPage();
            }
        }

        void itemClicked(const MouseEvent& e) override
        {
            if(isRoot())
            {
	            root.currentDialog->showMainPropertyEditor();
                return;
            }

            if(e.mods.isShiftDown())
            {
                auto typeId = obj[mpid::Type].toString();

	            auto id = root.currentDialog->getStringFromModalInput("Please enter the ID of the " + typeId, obj[mpid::ID].toString());

				if(id.isNotEmpty())
				{
					obj.getDynamicObject()->setProperty(mpid::ID, id);
                    root.currentDialog->refreshCurrentPage();
				}

                return;
            }

            if(!isVisible)
                return;

            if(!e.mods.isRightButtonDown())
            {
                auto& s = root.currentDialog->mouseSelector.selection;

                if(s.isSelected(obj))
                    s.deselect(obj);
                else
                    s.addToSelectionBasedOnModifiers(obj, e.mods);
                
		        return;
            }
            
            if(obj.hasProperty(mpid::Children))
            {
	            root.currentDialog->containerPopup(obj);
            }
            else
            {
	            root.currentDialog->nonContainerPopup(obj);
            }
        }

        void itemDoubleClicked(const MouseEvent&) override
        {
            

            auto newIndex = root.getPageIndex(obj);
            root.currentDialog->gotoPage(newIndex);

	        
        }

        void itemOpennessChanged(bool isNowOpen) override
        {
            clearSubItems();

	        if(isNowOpen)
	        {
                int idx = 0;

                auto tv = obj;

                auto childrenArePages = false;

                if(tv[mpid::Children].isArray())
                {
                    tv = tv[mpid::Children];
                }
                else
                {
	                childrenArePages = true;
                }
                    

                if(tv.isArray())
                {
	                for(const auto& v: *tv.getArray())
			        {
				        addSubItem(new PageItem(root, v, idx++, childrenArePages));
			        }
                }
	        }
        }

        void paintItem (Graphics& g, int width, int height) override
        {
            float alphaVisible = isVisible ? 1.0f : 0.4f;
            
            
            Rectangle<int> b(0, 0, width, height);

            
            PathFactory::scalePath(icon, b.removeFromLeft(height).reduced(2).toFloat());
            
            g.setColour(typeColour);
            g.fillPath(icon);

            g.setColour(root.tree.findColour(TreeView::ColourIds::linesColourId).withMultipliedAlpha(alphaVisible * 0.5f));

            if(isAction())
            {
                auto ab = (obj[mpid::EventTrigger].toString() == "OnSubmit") ? b.removeFromRight(b.getHeight()) : b.removeFromLeft(b.getHeight());
	            auto ap = root.createPath("arrow");
                root.scalePath(ap, ab.toFloat().reduced(4));
                g.fillPath(ap);
            }

            if(isEdited())
                g.setColour(Colour(SIGNAL_COLOUR));

            g.drawRoundedRectangle(b.toFloat().reduced(2.0f), 3.0f, 1.0f);

            g.setColour(typeColour.withAlpha(0.08f));
            g.fillRoundedRectangle(b.toFloat().reduced(4.0f), 2.0f);
            
            if(isSelected())
	        {
                g.setColour(Colours::white.withAlpha(0.05f));
		        g.fillRoundedRectangle(b.toFloat().reduced(4.0f), 2.0f);
	        }

            b = b.reduced(10, 0);

            auto id = obj[mpid::ID].toString();

            if(!id.isEmpty() && obj[mpid::Required])
                id << "*";

            auto t = isPage ? "Page" : obj[mpid::Type].toString();

            if(isRoot())
            {
	            t = "Project";
                alphaVisible = 1.0f;
            }
                

            g.setColour(Colours::white.withAlpha(0.8f * alphaVisible));
            g.setFont(GLOBAL_BOLD_FONT());
            g.drawText(t, b.toFloat(), Justification::left);
            g.setColour(Colours::white.withAlpha(0.4f * alphaVisible));
            g.setFont(GLOBAL_MONOSPACE_FONT());
            g.drawText(id, b.toFloat(), Justification::right);
        }

        String getUniqueName() const override
        {
	        return "Child" + String(index);
        }

	    bool mightContainSubItems() override
	    {
		    return obj[mpid::Children].size() > 0;
	    }

        int index;
        var obj;

        Tree& root;
        const bool isPage;
    };
    
    Tree():
	  addButton("add", nullptr, *this),
      deleteButton("delete", nullptr, *this)
    {
        setName("Component List");
        addAndMakeVisible(addButton);
        addAndMakeVisible(deleteButton);

        addButton.setTooltip("Add a new page");
        deleteButton.setTooltip("Remove the current page");

        addButton.onClick = [this]()
        {
			currentDialog->addListPageWithJSON();
        };

        deleteButton.onClick = [this]()
        {
	        currentDialog->removeCurrentPage();
        };

        tree.setColour(TreeView::backgroundColourId, Colour(0xFF262626));
        tree.setColour(TreeView::ColourIds::selectedItemBackgroundColourId, Colours::white.withAlpha(0.0f));
        tree.setColour(TreeView::ColourIds::linesColourId, Colour(0xFF999999));
        tree.setColour(TreeView::ColourIds::dragAndDropIndicatorColourId, Colour(SIGNAL_COLOUR));

        sf.addScrollBarToAnimate(tree.getViewport()->getVerticalScrollBar());

	    addAndMakeVisible(tree);
        
    }

    ~Tree()
    {
	    tree.deleteRootItem();
    }

    ScrollbarFader sf;

    Dialog* currentDialog = nullptr;


    void refresh()
    {
        tree.deleteRootItem();
	    tree.setRootItem(new PageItem(*this, currentDialog->getPageListVar(), 0, true));
        tree.setRootItemVisible(true);
        tree.setDefaultOpenness(true);
    }

    Factory iconFactory;
    
    Path createPath(const String& url) const override
    {
	    Path p;

        static const unsigned char arrowData[] = { 110,109,0,128,7,68,16,41,174,67,108,0,128,7,68,96,215,141,67,108,148,229,37,68,96,215,141,67,108,148,229,37,68,200,168,95,67,108,1,128,69,68,64,0,158,67,108,148,229,37,68,168,43,204,67,108,148,229,37,68,16,41,174,67,108,0,128,7,68,16,41,174,67,99,101,
		0,0 };

        LOAD_EPATH_IF_URL("add", HiBinaryData::ProcessorEditorHeaderIcons::addIcon);
        LOAD_EPATH_IF_URL("delete", EditorIcons::deleteIcon);

        LOAD_PATH_IF_URL("arrow", arrowData);
        

        return p;
    }

    void setRoot(Dialog& d)
    {
        rootObj = d.getPageListVar();
        currentDialog = &d;

        currentDialog->selectionUpdater.addListener(*this, [](Tree& t, const Array<var>& selection)
        {
	        t.repaint();
        });

        currentDialog->refreshBroadcaster.addListener(*this, [](Tree& t, int pageIndex)
        {
            t.deleteButton.setEnabled(t.currentDialog->getNumPages() > 1);
	        t.refresh();
        }, true);

        refresh();
    }

    void paint(Graphics& g)
    {
	    g.fillAll(Colour(0xFF262626));
    }

    void resized() override
    {
        auto b = getLocalBounds().reduced(10);

        auto bb = b.removeFromBottom(32);

	    tree.setBounds(b);

        addButton.setBounds(bb.removeFromLeft(bb.getHeight()).reduced(6));
        deleteButton.setBounds(bb.removeFromRight(bb.getHeight()).reduced(6));
    }

    var rootObj;
	juce::TreeView tree;

    JUCE_DECLARE_WEAK_REFERENCEABLE(Tree);

    HiseShapeButton addButton, deleteButton;
};


struct AssetManager: public Component,
					 public TableListBoxModel,
					 public PathFactory,
					 public FileDragAndDropTarget
{
    enum class Columns
    {
	    Type = 1,
        ID,
        Filename,
        Size,
        TargetOS
    };
    
    AssetManager(State& s):
      state(s),
      listbox("Assets", this),
      addButton("add", nullptr, *this),
      deleteButton("delete", nullptr, *this)
    {
        setName("Assets");

        addButton.setTooltip("Add new asset");
        deleteButton.setTooltip("Delete selected asset");

        listbox.setLookAndFeel(&tlaf);
        listbox.setColour(ListBox::ColourIds::backgroundColourId, Colours::transparentBlack);
        listbox.getHeader().setColour(TableHeaderComponent::ColourIds::backgroundColourId, Colour(0xFF333333));
        listbox.getHeader().setColour(TableHeaderComponent::ColourIds::textColourId, Colour(0xFF999999));
        listbox.getHeader().setColour(TableHeaderComponent::ColourIds::outlineColourId, Colour(0xFF444444));
        
        listbox.setHeaderHeight(22);

	    addAndMakeVisible(listbox);
        addAndMakeVisible(addButton);
        addAndMakeVisible(deleteButton);

        auto h = listbox.getRowHeight() * 2;

        listbox.getHeader().addColumn("Type", (int)Columns::Type, 50, 50, 50);
        listbox.getHeader().addColumn("ID", (int)Columns::ID, 100, 100, -1);
        listbox.getHeader().addColumn("File", (int)Columns::Filename, 100, 100, -1);
        listbox.getHeader().addColumn("Size", (int)Columns::Size, 50, 50, 100);
        listbox.getHeader().addColumn("OS", (int)Columns::TargetOS, 60, 60, 60);
        
		listbox.getHeader().setStretchToFitActive(true);

        addButton.onClick = [this]()
        {
	        FileChooser fc("Add asset", File());

            if(fc.browseForFileToOpen())
            {
            	state.assets.add(Asset::fromFile(fc.getResult()));
                state.assets.getLast()->useRelativePath = fc.getResult().isAChildOf(state.currentRootDirectory);
                listbox.updateContent();
            }
        };
    }


    bool isInterestedInFileDrag (const StringArray& files) override 
    {
        return !files.isEmpty();
    }
    void filesDropped (const StringArray& files, int x, int y) override 
    {
        for(const auto& f: files)
        {
            addAsset(f);
        }
    }
    
    Asset::Ptr addAsset(const File& f)
    {
        state.assets.add(Asset::fromFile(File(f)));
        state.assets.getLast()->useRelativePath = File(f).isAChildOf(state.currentRootDirectory);

        listbox.updateContent();
        return state.assets.getLast();
    }

    void rename(Asset::Ptr a)
    {
	    a->id = state.currentDialog->getStringFromModalInput("Please enter the asset ID", a->id);
        listbox.updateContent();
        repaint();
    }
    
    void cellClicked (int rowNumber, int columnId, const MouseEvent& e) override
    {
        if(auto a = state.assets[rowNumber])
        {
            if(e.mods.isShiftDown())
            {
	            rename(a);
                return;
            }

            if(e.mods.isRightButtonDown())
		    {
	            PopupMenu m;
	            PopupLookAndFeel plaf;
	            m.setLookAndFeel(&plaf);

	            enum CommandIds
	            {
		            ChangeAssetID = 1,
	                RevealToUser,
					DeleteAsset,
	                CreateReference,
	                ChangeFileReference,
                    UseAbsolutePath,
                    UseRelativePath,
                    OSOffset,
	                numCommandIds
	            };

	            m.addItem(ChangeAssetID, "Change ID");
	            m.addItem(RevealToUser, "Show in file browser");
	            m.addItem(CreateReference, "Create asset reference variable");
	            m.addSeparator();
                m.addItem(UseAbsolutePath, "Use absolute path", true, !a->useRelativePath);
                m.addItem(UseRelativePath, "Use relative path", true, a->useRelativePath);
                m.addSeparator();
                m.addItem(OSOffset + (int)Asset::TargetOS::All, "All operating systems", true, a->os == Asset::TargetOS::All);
                m.addItem(OSOffset + (int)Asset::TargetOS::Windows, "Windows only", true, a->os == Asset::TargetOS::Windows);
                m.addItem(OSOffset + (int)Asset::TargetOS::macOS, "macOS only", true, a->os == Asset::TargetOS::macOS);
                m.addItem(OSOffset + (int)Asset::TargetOS::Linux, "Linux only", true, a->os == Asset::TargetOS::Linux);
                m.addSeparator();
	            m.addItem(ChangeFileReference, "Change file reference");
	            m.addItem(DeleteAsset, "Delete asset");

	            if(auto r = m.show())
	            {
                    if(r >= OSOffset)
                    {
	                    a->os = (Asset::TargetOS)(r - OSOffset);
                        listbox.updateContent();
                        listbox.repaintRow(rowNumber);
                        listbox.repaint();
                        return;
                    }

		            switch((CommandIds)r)
		            {
		            case ChangeAssetID:
                        rename(a);
	                    break;
		            case RevealToUser:
	                    File(a->filename).revealToUser();
	                    break;
		            case DeleteAsset:
	                    state.assets.remove(rowNumber);
	                    listbox.updateContent();
                        listbox.repaint();
	                    break;
		            case UseAbsolutePath:
                        a->useRelativePath = false;
                        listbox.repaint();
                        break;
		            case UseRelativePath:
                        a->useRelativePath = true;
                        listbox.repaint();
                        break;
		            case CreateReference:
		            {
			            SystemClipboard::copyTextToClipboard(a->toReferenceVariable());
						break;
		            }
		            case ChangeFileReference:
	                {
				        FileChooser fc("Change file reference", File(a->filename));

			            if(fc.browseForFileToOpen())
			            {
	                        auto oldId = a->id;

	                        state.assets.set(rowNumber, Asset::fromFile(fc.getResult()));
	                        state.assets[rowNumber]->id = oldId;
			                listbox.updateContent();
                            listbox.repaint();
			            }

			            break;
			        }
		            case numCommandIds: break;
		            default: ;
		            }
	            }
		    }
        }
    }
    
    

    void paintRowBackground (Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override
    {
	    if(rowIsSelected)
        {
            g.setColour(Colours::white.withAlpha(0.05f));
            g.fillRect(0, 0, width, height);
        }
    }

    String getCellTest(int rowNumber, int columnId) const
    {
        if(auto a = state.assets[rowNumber])
        {
            const String letters = "IFTA";
            auto typeIndex = (int)a->type;

	        switch((Columns)columnId)
            {
            case Columns::Type: return letters.substring(typeIndex, typeIndex+1);
            case Columns::ID: return a->id;
            case Columns::Filename: return File(a->filename).getFileName();
            case Columns::Size: return String((double)a->data.getSize() / 1024.0 / 1024.0, 1) + String("MB");
            case Columns::TargetOS: return Asset::getOSName(a->os);
            default: return {};
            }
        }

        return {};
    }

    /** This must draw one of the cells.

        The graphics context's origin will already be set to the top-left of the cell,
        whose size is specified by (width, height).

        Note that the rowNumber value may be greater than the number of rows in your
        list, so be careful that you don't assume it's less than getNumRows().
    */void paintCell (Graphics& g, int rowNumber,int columnId, int width, int height, bool rowIsSelected) override
    {
	    g.setFont(GLOBAL_BOLD_FONT());
        g.setColour(Colours::white.withAlpha(0.5f));
        g.drawText(getCellTest(rowNumber, columnId), 0, 0, width, height, Justification::centred);;
    }

    String getCellTooltip(int rowNumber, int columnId) override
    {
	    return getCellTest(rowNumber, columnId);
    }


	int getNumRows() override { return state.assets.size(); }

    void resized() override
    {
	    auto b = getLocalBounds();
        auto bb = b.removeFromBottom(32);

        listbox.getHeader().resizeAllColumnsToFit(getWidth());

	    listbox.setBounds(b);
        
        addButton.setBounds(bb.removeFromLeft(bb.getHeight()).reduced(6));
        deleteButton.setBounds(bb.removeFromRight(bb.getHeight()).reduced(6));
    }

    Path createPath(const String& url) const override
    {
	    Path p;

        LOAD_EPATH_IF_URL("add", HiBinaryData::ProcessorEditorHeaderIcons::addIcon);
        LOAD_EPATH_IF_URL("delete", SampleMapIcons::deleteSamples);

        return p;
    }

    void paint(Graphics& g) override
    {
        g.fillAll(Colour(0xFF262626));

        if (state.assets.isEmpty())
		{
			g.setFont(GLOBAL_FONT());
			g.setColour(Colours::white.withAlpha(0.4f));

			String errorMessage = "Drop or add file assets here...";
            g.drawText(errorMessage, getLocalBounds().toFloat().removeFromTop(80.0f), Justification::centred);
		}
    }

    TableHeaderLookAndFeel tlaf;

    TableListBox listbox;
    HiseShapeButton addButton, deleteButton;

    State& state;
};

}
}

using namespace multipage;

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent   : public Component,
					    public Timer,
					    public MenuBarModel,
					    public CodeDocument::Listener,
					    public TopLevelWindowWithKeyMappings,
						public multipage::ComponentWithSideTab
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();

    bool manualChange = false;

    File getSettingsFile() const
    {
        return ScopedSetting::getSettingFile();
    }

    void codeDocumentTextInserted (const String& , int ) override
    {
	    manualChange = true;
    }

    
    void codeDocumentTextDeleted (int , int ) override
    {
        manualChange = true;
    }

    File getKeyPressSettingFile() const override { return getSettingsFile().getSiblingFile("multipage_keyset.xml"); }

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    void timerCallback() override;;

    void setSavePoint();

    struct SideTab: public Component
    {
        SideTab():
          Component("Edit Properties")
        {};

        void paint(Graphics& g) override
        {
            g.fillAll(Colour(0xFF262626));
            
	        if (dialog == nullptr)
			{
				g.setFont(GLOBAL_FONT());
				g.setColour(Colours::white.withAlpha(0.4f));

				String errorMessage = "No element selected";
                g.drawText(errorMessage, getLocalBounds().toFloat().removeFromTop(80.0f), Justification::centred);
			}
        }

        void set(State* s, Dialog* d)
        {
	        dialog = d;
            state = s;

            if(dialog != nullptr)
            {
	            addAndMakeVisible(dialog);
                resized();
            }
        }
        
        void resized() override
        {
	        auto b = getLocalBounds();
            b.setHeight(b.getHeight() + 20);

            if(dialog != nullptr)
				dialog->setBounds(b);
        }

	    ScopedPointer<multipage::State> state;
        ScopedPointer<multipage::Dialog> dialog;
    };

    void refreshDialog() override
    {
	    if(c != nullptr)
	    {
		    c->refreshCurrentPage();
            resized();
	    }
    }

    State* getMainState() override { return &rt; }

    Component::SafePointer<Dialog> currentSideTabDialog;

    bool setSideTab(multipage::State* dialogState, multipage::Dialog* newDialog) override
    {
        currentSideTabDialog = newDialog;

        auto sideDialog = rightTab.getChild<SideTab>(0);

        sideDialog->set(dialogState, newDialog);
        
        resized();
        return sideDialog->dialog != nullptr;
    }

    void checkSave()
    {
	    if(modified)
	    {
		    if(currentFile.existsAsFile() && AlertWindow::showOkCancelBox(MessageBoxIconType::QuestionIcon, "Save changes", "Do you want to save the changes"))
	        { 
                c->getState().callEventListeners("save", {});
		        currentFile.replaceWithText(JSON::toString(c->exportAsJSON()));
                setSavePoint();
	        }
	    }
    }

    void build();

    /** This method must return a list of the names of the menus. */
    StringArray getMenuBarNames()
    {
	    return { "File", "Edit", "View", "Help" };
    }

    enum CommandId
    {
	    FileNew = 1,
        FileCreateCSS,
        FileLoad,
        FileSave,
        FileSaveAs,
        FileExportAsProjucerProject,
        FileQuit,
        EditUndo,
        EditRedo,
        EditClearState,
        EditToggleMode,
        EditRefreshPage,
        EditAddPage,
        EditRemovePage,
        ViewShowDialog,
        ViewShowJSON,
        ViewShowCpp,
        ViewShowConsole,
        ViewShowCSSDebugger,
        HelpAbout,
        HelpVersion,
        FileRecentOffset = 9000
    };
    
	PopupMenu getMenuForIndex (int topLevelMenuIndex, const String&) override;

    bool keyPressed(const KeyPress& key) override;

    void menuItemSelected (int menuItemID, int) override;

    multipage::AssetManager* assetManager;

private:

    int64 prevHash = 0;
    bool modified = false;
    bool firstAfterSave = false;

    File currentFile;

    juce::RecentlyOpenedFilesList fileList;

    void createDialog(const File& f);

    ScopedPointer<Autosaver> autosaver;

    //==============================================================================
    // Your private member variables go here...
    int counter = 0;
	OpenGLContext context;

    multipage::State rt;
    ScopedPointer<multipage::Dialog> c;

    TooltipWindow tooltips;

    juce::CodeDocument doc;
    mcl::TextDocument stateDoc;
    mcl::TextEditor stateViewer;

    Component* currentInspector = nullptr;

    AlertWindowLookAndFeel plaf;
    MenuBarComponent menuBar;

    ScopedPointer<multipage::HardcodedDialogWithState> hardcodedDialog;

    struct ModalDialog: public Component
    {
        ModalDialog(MainComponent& parent, multipage::HardcodedDialogWithState* newContent):
          content(newContent)
        {
	        addAndMakeVisible(content);
            parent.addAndMakeVisible(this);
            setBounds(parent.getLocalBounds());
            content->centreWithSize(content->getWidth(), content->getHeight());

            content->setOnCloseFunction([&parent]()
			{
				parent.modalDialog = nullptr;
			});
        }

        void paint(Graphics& g) override
        {
	        g.fillAll(Colour(0xCC161616));

            DropShadow sh;
            sh.colour = Colours::black.withAlpha(0.7f);
            sh.radius = 20;
            sh.drawForRectangle(g, content->getBoundsInParent());
        }

	    ScopedPointer<multipage::HardcodedDialogWithState> content;
    };

    RightTab rightTab;
    RightTab leftTab; // fuck yeah...

    ScopedPointer<ModalDialog> modalDialog;

    
    
    Tree* tree;
    ScopedPointer<ComponentWithEdge> console;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};


