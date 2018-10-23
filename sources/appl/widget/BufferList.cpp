/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#include <appl/debug.hpp>
#include <appl/global.hpp>
#include <appl/widget/BufferList.hpp>
#include <appl/BufferManager.hpp>
//#include <ColorizeManager.hpp>
#include <appl/Gui/MainWindows.hpp>
#include <ewol/object/Object.hpp>

appl::widget::BufferList::BufferList() :
  m_openOrderMode(false) {
	addObjectType("appl::BufferList");
	m_selectedID = -1;
	m_selectedIdRequested = -1;
	// load buffer manager:
	m_bufferManager = appl::BufferManager::create();
	// load color properties
	m_paintingProperties = appl::GlyphPainting::create("THEME_COLOR:///bufferList.json");
	// get all id properties ...
	m_colorBackground1 = m_paintingProperties->request("backgroung1");
	m_colorBackground2 = m_paintingProperties->request("backgroung2");
	m_colorBackgroundSelect = m_paintingProperties->request("backgroungSelected");
	m_colorTextNormal = m_paintingProperties->request("textNormal");
	m_colorTextModify = m_paintingProperties->request("textModify");
}

void appl::widget::BufferList::init() {
	ewol::widget::List::init();
	//propertyHide.set(true);
	propertyCanFocus.set(true);
	if (m_bufferManager != null) {
		m_bufferManager->signalNewBuffer.connect(sharedFromThis(), &appl::widget::BufferList::onCallbackNewBuffer);
		m_bufferManager->signalSelectFile.connect(sharedFromThis(), &appl::widget::BufferList::onCallbackselectNewFile);
		m_bufferManager->signalRemoveBuffer.connect(sharedFromThis(), &appl::widget::BufferList::onCallbackBufferRemoved);
	}
}

appl::widget::BufferList::~BufferList() {
	removeAllElement();
}


void appl::widget::BufferList::calculateMinMaxSize() {
	/*int32_t fontId = getDefaultFontId();
	int32_t minWidth = ewol::getWidth(fontId, m_label);
	int32_t minHeight = ewol::getHeight(fontId);
	m_minSize.x = 3+minWidth;
	m_minSize.y = 3+minHeight;
	*/
	m_minSize.setValue(300, 150);
}

void appl::widget::BufferList::removeAllElement() {
	m_list.clear();
}

void appl::widget::BufferList::insertAlphabetic(const appl::dataBufferStruct& _dataStruct, bool _selectNewPosition) {
	// alphabetical order:
	for (size_t iii = 0; iii < m_list.size(); ++iii) {
		if (m_list[iii].m_bufferName.getFileName().toLower() > _dataStruct.m_bufferName.getFileName().toLower()) {
			m_list.insert(m_list.begin() + iii, _dataStruct);
			if (_selectNewPosition == true) {
				m_selectedID = iii;
			}
			return;
		}
	}
	m_list.pushBack(_dataStruct);
	if (_selectNewPosition == true) {
		m_selectedID = m_list.size()-1;
	}
}

void appl::widget::BufferList::onCallbackNewBuffer(const ememory::SharedPtr<appl::Buffer>& _buffer) {
	if (_buffer == null) {
		return;
	}
	ememory::SharedPtr<appl::Buffer> buffer = _buffer;
	buffer->signalIsSave.connect(sharedFromThis(), &BufferList::onCallbackIsSave);
	buffer->signalIsModify.connect(sharedFromThis(), &BufferList::onCallbackIsModify);
	buffer->signalChangeName.connect(sharedFromThis(), &BufferList::onCallbackChangeName);
	appl::dataBufferStruct tmp(_buffer->getFileName(), _buffer);
	if (m_openOrderMode == true) {
		m_list.pushBack(tmp);
	} else {
		insertAlphabetic(tmp);
	}
	/*
	if (m_list.size() <= 1) {
		propertyHide.set(true);
		propertySetOnWidgetNamed("appl-Buffer-viewer-separator", "hide", "true");
	} else {
		propertyHide.set(false);
		propertySetOnWidgetNamed("appl-Buffer-viewer-separator", "hide", "false");
	}
	*/
	markToRedraw();
}

// TODO : Review this callback with the real shared_ptr on the buffer ...
void appl::widget::BufferList::onCallbackselectNewFile(const ememory::SharedPtr<appl::Buffer>& _buffer) {
	m_selectedID = -1;
	for (size_t iii=0; iii<m_list.size(); iii++) {
		if (m_list[iii].m_buffer == null) {
			continue;
		}
		if (m_list[iii].m_buffer != _buffer) {
			continue;
		}
		m_selectedID = iii;
		break;
	}
	markToRedraw();
}

void appl::widget::BufferList::onCallbackChangeName() {
	for (size_t iii = 0; iii < m_list.size(); ++iii) {
		if (m_list[iii].m_bufferName != m_list[iii].m_buffer->getFileName()) {
			m_list[iii].m_bufferName = m_list[iii].m_buffer->getFileName();
			if (m_openOrderMode == false) {
				// re-order the fine in the correct position
				appl::dataBufferStruct tmp = m_list[iii];
				m_list.erase(m_list.begin() + iii);
				insertAlphabetic(tmp, ((int64_t)iii == m_selectedID));
				break;
			}
		}
	}
	markToRedraw();
}

void appl::widget::BufferList::onCallbackBufferRemoved(const ememory::SharedPtr<appl::Buffer>& _buffer) {
	APPL_DEBUG("request remove buffer:");
	auto it = m_list.begin();
	while (it != m_list.end()) {
		if (it->m_buffer == _buffer) {
			it = m_list.erase(it);
			m_selectedID = -1;
		} else {
			++it;
		}
	}
	/*
	if (m_list.size() <= 1) {
		propertyHide.set(true);
	} else {
		propertyHide.set(false);
	}
	*/
	markToRedraw();
}
void appl::widget::BufferList::onCallbackIsSave() {
	markToRedraw();
}
void appl::widget::BufferList::onCallbackIsModify() {
	markToRedraw();
}

etk::Color<> appl::widget::BufferList::getBasicBG() {
	return (*m_paintingProperties)[m_colorBackground1].getForeground();
}

ivec2 appl::widget::BufferList::getMatrixSize() const {
	return ivec2(1,m_list.size());
}

fluorine::Variant appl::widget::BufferList::getData(int32_t _role, const ivec2& _pos) {
	switch (_role) {
		case ewol::widget::ListRole::Text:
			return m_list[_pos.y()].m_bufferName.getFileName();;
		case ewol::widget::ListRole::FgColor:
			if (    m_list[_pos.y()].m_buffer != null
			     && m_list[_pos.y()].m_buffer->isModify() == false) {
				return (*m_paintingProperties)[m_colorTextNormal].getForeground();
			} else {
				return (*m_paintingProperties)[m_colorTextModify].getForeground();
			}
		case ewol::widget::ListRole::BgColor:
			APPL_VERBOSE( m_colorBackground1 << " => " << (*m_paintingProperties)[m_colorBackground1].getForeground());
			if (m_selectedID == _pos.y()) {
				return (*m_paintingProperties)[m_colorBackgroundSelect].getForeground();
			}
			if (_pos.y() % 2) {
				return (*m_paintingProperties)[m_colorBackground1].getForeground();
			}
			return (*m_paintingProperties)[m_colorBackground2].getForeground();
		case ewol::widget::ListRole::IsSelected:
			return m_selectedID == _pos.y();
	}
	return fluorine::Variant();
}


bool appl::widget::BufferList::onItemEvent(const ewol::event::Input& _event, const ivec2& _pos, const vec2& _mousePosition) {
	if (    _event.getId() == 1
	     && _event.getStatus() == gale::key::status::pressSingle) {
		APPL_VERBOSE("Event on List: " << _event << " pos=" << _pos );
		if(    _pos.y() >= 0
		    && _pos.y() < (int64_t)m_list.size()) {
			if (m_list[_pos.y()].m_buffer != null) {
				if (m_bufferManager != null) {
					APPL_VERBOSE("Select file: '" << m_list[_pos.y()].m_buffer->getFileName() << "' in list");
					m_bufferManager->open(m_list[_pos.y()].m_buffer->getFileName());
				}
				return true;
			}
		}
	}
	return false;
}


