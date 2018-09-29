/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#pragma once
#include <appl/debug.hpp>
#include <ewol/widget/PopUp.hpp>
#include <appl/Gui/TagFileList.hpp>

namespace appl {
	class TagFileSelection : public ewol::widget::PopUp {
		public:
			esignal::Signal<etk::Path, int32_t> signalSelect;
			esignal::Signal<> signalCancel;
		private:
			ememory::SharedPtr<appl::TagFileList> m_listTag;
			etk::Path m_eventPath;
			int32_t m_eventLine = -1;
		public:
			TagFileSelection();
			void init();
		public:
			DECLARE_FACTORY(TagFileSelection);
			virtual ~TagFileSelection();
			/**
			 * @brief add a Ctags item on the curent list
			 * @param[in] _path Compleate file name
			 * @param[in] _line Jump line id
			 */
			void addCtagsNewItem(etk::Path _path, int32_t _line);
		public: // callback function
			void onCallbackCtagsSelection();
			void onCallbackCtagsCancel();
			void onCallbackCtagsListValidate(const etk::Path& _path, const int32_t& _line);
			void onCallbackCtagsListSelect(const etk::Path& _path, const int32_t& _line);
			void onCallbackCtagsListUnSelect();
	};
}

