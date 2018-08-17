/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#pragma once
#include <appl/debug.hpp>
#include <ewol/widget/List.hpp>
#include <ewol/resource/ColorFile.hpp>

namespace appl {
	class TagListElement {
		public:
			etk::String filename;
			int32_t      fileLine;
			TagListElement(etk::String& _file, int32_t _line) :
			  filename(_file),
			  fileLine(_line) {
				
			};
			virtual ~TagListElement() {
				
			};
	};
	class TagFileList : public ewol::widget::List {
		public:
			esignal::Signal<etk::String> signalSelect;
			esignal::Signal<etk::String> signalValidate;
			esignal::Signal<> signalUnSelect;
		private:
			int32_t m_selectedLine;
			etk::Vector<appl::TagListElement*> m_list;
		protected:
			ememory::SharedPtr<ewol::resource::ColorFile> m_colorProperty; //!< theme color property.
			int32_t m_colorIdText; //!< Color of the text.
			int32_t m_colorIdBackground1; //!< Color of the Background.
			int32_t m_colorIdBackground2; //!< Color of the Background 2.
			int32_t m_colorIdBackgroundSelected; //!< Color of line selected.
		protected:
			TagFileList();
			void init() override;
		public:
			DECLARE_FACTORY(TagFileList);
			virtual ~TagFileList();
			// display API :
			etk::Color<> getBasicBG() override;
			bool getTitle(int32_t _colomn, etk::String& _myTitle, etk::Color<>& _fg, etk::Color<>& _bg) override;
			ivec2 getMatrixSize() const override;
			fluorine::Variant getData(int32_t _role, const ivec2& _pos) override;
			bool onItemEvent(int32_t _IdInput, enum gale::key::status _typeEvent, const ivec2& _pos, const vec2& _mousePosition) override;
		public:
			/**
			 * @brief add a Ctags item on the curent list
			 * @param[in] file Compleate file name
			 * @param[in] jump line id
			 */
			void add(etk::String& _file, int32_t _line);
	};
}

