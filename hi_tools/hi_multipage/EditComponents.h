/*  ===========================================================================
 *
 *   This file is part of HISE.
 *   Copyright 2016 Christoph Hart
 *
 *   HISE is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   HISE is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with HISE.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   Commercial licenses for using HISE in an closed source project are
 *   available on request. Please visit the project's website to get more
 *   information about commercial licensing:
 *
 *   http://www.hise.audio/
 *
 *   HISE is based on the JUCE library,
 *   which must be separately licensed for closed source applications:
 *
 *   http://www.juce.com
 *
 *   ===========================================================================
 */

#pragma once

namespace hise
{


namespace multipage {
using namespace juce;

struct CodeGenerator
{
    enum class FileType
    {
	    AssetHeader,
        AssetData,
        DialogHeader,
        DialogImplementation,
        ProjucerFile,
        BatchFile,
        MainCpp,
        numFileTypes
    };

    CodeGenerator(const File& rootDirectory_, String className_, const var& totalData, int numTabs_=0):
      data(totalData),
      className(className_),
      rootDirectory(rootDirectory_),
      numTabs(numTabs_)
	{}

    void write(OutputStream& output, FileType t, State::Job* job) const;

    File getFile(FileType type) const
    {
        auto root = rootDirectory.getChildFile("Binaries");
        switch(type)
        {
        case FileType::AssetHeader: return root.getChildFile("Source/Assets.h");
        case FileType::AssetData:  return root.getChildFile("Source/Assets.cpp");
        case FileType::DialogHeader:  return root.getChildFile("Source/Dialog.h");
        case FileType::BatchFile:
#if JUCE_WINDOWS
            return root.getChildFile("batchCompile.bat");
#else
            return root.getChildFile("batchCompileOSX");
#endif
        case FileType::DialogImplementation:  return root.getChildFile("Source/Dialog.cpp");
        case FileType::ProjucerFile:  return root.getChildFile(className).withFileExtension(".jucer");
        case FileType::MainCpp:  return root.getChildFile("Source/Main.cpp");
        case FileType::numFileTypes: 
        default: return File();
        }
    }

    String company;
    String version;
    String hisePath;

private:

    struct ScopedTabSetter
    {
	    ScopedTabSetter(const CodeGenerator& cg):
          parent(cg)
	    {
		    parent.numTabs++;
	    }

        ~ScopedTabSetter()
	    {
		    parent.numTabs--;
	    }

        const CodeGenerator& parent;
    };

    File rootDirectory;

    String generateRandomId(const String& prefix) const;
    String getNewLine() const;
    String createAddChild(const String& parentId, const var& childData, const String& itemType="Page", bool attachCustomFunction=false) const;
    
    static String arrayToCommaString(const var& value);
    
    mutable StringArray existingVariables;

    mutable int idCounter = 0;
    mutable int numTabs;
    var data;
    String className;
};



struct EditorOverlay: public Component,
				      public SettableTooltipClient
{
    struct Updater: public ComponentMovementWatcher
    {
	    Updater(EditorOverlay& parent_, Component* attachedComponent);

        void componentMovedOrResized (bool wasMoved, bool wasResized) override;

	    void componentPeerChanged() override {};

	    void componentVisibilityChanged() override
	    {
		    parent.setVisible(getComponent()->isVisible());
	    }

        EditorOverlay& parent;
    };

    ScopedPointer<Updater> watcher;

    

    void setAttachToComponent(Component* c)
    {
	    watcher = new Updater(*this, c);
    }

    EditorOverlay(Dialog& parent);

    void mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel) override;

    void resized() override;

    void mouseUp(const MouseEvent& e) override;

    void paint(Graphics& g) override;

    static void onEditChange(EditorOverlay& c, bool isOn)
    {
	    c.setVisible(isOn);
    }

    void setOnClick(const std::function<void(bool)>& f)
    {
	    cb = f;
    }

    std::function<Rectangle<int>(Component*)> localBoundFunction;

    std::function<void(bool)> cb;

    Path outline;
    Rectangle<float> bounds;
	
    JUCE_DECLARE_WEAK_REFERENCEABLE(EditorOverlay);
};

} // multipage
} // hise
