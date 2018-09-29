/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#pragma once
#include <etk/types.hpp>
#include <ewol/object/Object.hpp>
#include <appl/Gui/TextViewer.hpp>
#include <ewol/compositing/Text.hpp>
#include <appl/TextPlugin.hpp>
#include <appl/ctags/readtags.hpp>

// create ctags file : "ctags-exuberant --fields=n -R"
// --fields=n add the line number needed for this software version ..
// ctags --recurse -f tags --fields=n -h ".h.hpp" --tag-relative=yes framework/atria-soft/

namespace appl {
	void setCtagsFileName(const etk::Uri& _uri);
	class TextPluginCtags : public appl::TextViewerPlugin {
		private:
			// Global plugin data (not specific on buffer :
			/*
			etk::Vector<etk::Pair<etk::String, int64_t>> m_historyList;
			*/
			etk::Path m_tagFolderBase;
			etk::String m_tagFilename;
			tagFile* m_ctagFile;
			void loadTagFile();
			void printTag(const tagEntry *_entry);
			void jumpTo(const etk::String& _name);
			void jumpFile(const etk::Path& _filename, int64_t _lineId);
			ememory::SharedPtr<appl::BufferManager> m_bufferManager; //!< handle on the buffer manager
		protected:
			TextPluginCtags();
		public:
			DECLARE_FACTORY(TextPluginCtags);
			virtual ~TextPluginCtags();
		public:
			virtual void onPluginEnable(appl::TextViewer& _textDrawer);
			virtual void onPluginDisable(appl::TextViewer& _textDrawer);
			virtual bool onReceiveShortCut(appl::TextViewer& _textDrawer,
			                               const etk::String& _shortCutName);
			// callback function:
			void onCallbackOpenCtagsOpenFileReturn(const etk::Path& _path);
			void onCallbackOpenCtagsSelectReturn(const etk::Path& _path, const int32_t& _line);
	};
}

