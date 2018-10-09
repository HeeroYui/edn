/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#include <etk/tool.hpp>
#include <appl/Gui/TagFileList.hpp>

appl::TagFileList::TagFileList() :
  signalSelect(this, "select", ""),
  signalValidate(this, "validate", ""),
  signalUnSelect(this, "unselect", "") {
	addObjectType("appl::TagFileList");
	m_selectedLine = -1;
	setMouseLimit(1);
	// Load color properties: (use file list to be generic ...)
	m_colorProperty = ewol::resource::ColorFile::create("THEME_COLOR:///ListFileSystem.json");
	if (m_colorProperty != null) {
		m_colorIdText = m_colorProperty->request("text");
		m_colorIdBackground1 = m_colorProperty->request("background1");
		m_colorIdBackground2 = m_colorProperty->request("background2");
		m_colorIdBackgroundSelected = m_colorProperty->request("selected");
	}
}
void appl::TagFileList::init() {
	ewol::widget::List::init();
}

appl::TagFileList::~TagFileList() {
	for (auto &it : m_list) {
		ETK_DELETE(appl::TagListElement, it);
		it = null;
	}
}

etk::Color<> appl::TagFileList::getBasicBG() {
	return 0x00000010;
}

ivec2 appl::TagFileList::getMatrixSize() const {
	return ivec2(2,m_list.size());
}

fluorine::Variant appl::TagFileList::getData(int32_t _role, const ivec2& _pos) {
	switch (_role) {
		case ewol::widget::ListRole::Text:
			if (    _pos.y() >= 0
			     && (size_t)_pos.y() < m_list.size()
			     && null != m_list[_pos.y()]) {
				if (0 == _pos.x()) {
					return etk::toString(m_list[_pos.y()]->fileLine);
				}
				return m_list[_pos.y()]->filename.getString();
			}
			return "ERROR";
		case ewol::widget::ListRole::FgColor:
			return m_colorProperty->get(m_colorIdText);
		case ewol::widget::ListRole::BgColor:
			if (m_selectedLine == _pos.y()) {
				return m_colorProperty->get(m_colorIdBackgroundSelected);
			}
			if (_pos.y() % 2) {
				return m_colorProperty->get(m_colorIdBackground1);
			}
			return m_colorProperty->get(m_colorIdBackground2);
	}
	return fluorine::Variant();
}


bool appl::TagFileList::onItemEvent(const ewol::event::Input& _event, const ivec2& _pos, const vec2& _mousePosition) {
	if (_event.getStatus() == gale::key::status::pressSingle) {
		EWOL_INFO("Event on List: " << _event << " pos=" << _pos );
		if (_event.getId() == 1) {
			int32_t previousRaw = m_selectedLine;
			if (_pos.y() > (int64_t)m_list.size() ) {
				m_selectedLine = -1;
			} else {
				m_selectedLine = _pos.y();
			}
			if(    m_selectedLine  >= 0
			    && m_selectedLine < (int64_t)m_list.size()
			    && null != m_list[m_selectedLine] ) {
				if (previousRaw != m_selectedLine) {
					signalSelect.emit(m_list[m_selectedLine]->filename, m_list[_pos.y()]->fileLine);
				} else {
					signalValidate.emit(m_list[m_selectedLine]->filename, m_list[_pos.y()]->fileLine);
				}
			} else {
				signalUnSelect.emit();
			}
			// need to regenerate the display of the list : 
			markToRedraw();
			return true;
		}
	}
	return false;
}


/**
 * @brief add a Ctags item on the curent list
 * @param[in] file Compleate file name
 * @param[in] jump line id
 */
void appl::TagFileList::add(etk::Path& _file, int32_t _line) {
	appl::TagListElement *tmpFile = ETK_NEW(appl::TagListElement, _file, _line);
	if (null != tmpFile) {
		m_list.pushBack(tmpFile);
	}
	markToRedraw();
}



#include <esignal/details/Signal.hxx>
// declare for signal event
ESIGNAL_DECLARE_SIGNAL(etk::Path, int32_t);