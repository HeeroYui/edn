/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#pragma once
#include <etk/types.h>
#include <ewol/object/Object.h>
#include <appl/Gui/TextViewer.h>
#include <ewol/compositing/Text.h>
#include <appl/TextPlugin.h>

namespace appl {
	class TextPluginRmLine : public appl::TextViewerPlugin {
		protected:
			TextPluginRmLine();
		public:
			DECLARE_FACTORY(TextPluginRmLine);
			virtual ~TextPluginRmLine() {
				// nothing to do ...
			};
		public:
			virtual void onPluginEnable(appl::TextViewer& _textDrawer);
			virtual void onPluginDisable(appl::TextViewer& _textDrawer);
			virtual bool onReceiveShortCut(appl::TextViewer& _textDrawer, const std::string& _shortCutName);
	};
}

