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

namespace appl {
	class TextPluginAutoIndent : public appl::TextViewerPlugin {
		protected:
			TextPluginAutoIndent();
		public:
			DECLARE_FACTORY(TextPluginAutoIndent);
			virtual ~TextPluginAutoIndent() {
				// nothing to do ...
			};
		public:
			virtual bool onEventEntry(appl::TextViewer& _textDrawer,
			                          const ewol::event::Entry& _event);
	};
}
