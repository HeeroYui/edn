/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#include <appl/TextPluginHistory.hpp>
#include <appl/Gui/TextViewer.hpp>
#include <appl/TextPluginManager.hpp>

appl::TextPluginHistory::TextPluginHistory() :
  m_menuIdTitle(-1),
  m_menuIdUndo(-1),
  m_menuIdRedo(-1) {
	m_activateOnReceiveShortCut = true;
	m_activateOnWrite = true;
	m_activateOnReplace = true;
	m_activateOnRemove = true;
	addObjectType("appl::TextPluginHistory");
}


void appl::TextPluginHistory::onPluginEnable(appl::TextViewer& _textDrawer) {
	ememory::SharedPtr<ewol::widget::Menu> menu = m_menuInterface.lock();
	if (menu != null) {
		m_menuIdTitle = menu->addTitle("_T{Edit}");
		if (m_menuIdTitle != -1) {
			m_menuIdUndo = menu->add(m_menuIdTitle, "_T{Undo}", "THEME_GUI:///Undo.svg", "appl::TextPluginHistory::menu:undo");
			m_menuIdRedo = menu->add(m_menuIdTitle, "_T{Redo}", "THEME_GUI:///Redo.svg", "appl::TextPluginHistory::menu:redo");
		}
	}
	// add event :
	_textDrawer.ext_shortCutAdd("ctrl+z", "appl::TextPluginHistory::Undo");
	_textDrawer.ext_shortCutAdd("ctrl+shift+z", "appl::TextPluginHistory::Redo");
}

void appl::TextPluginHistory::onPluginDisable(appl::TextViewer& _textDrawer) {
	_textDrawer.ext_shortCutRm("appl::TextPluginHistory::Undo");
	_textDrawer.ext_shortCutRm("appl::TextPluginHistory::Redo");
	ememory::SharedPtr<ewol::widget::Menu> menu = m_menuInterface.lock();
	if (menu != null) {
		menu->remove(m_menuIdRedo);
		menu->remove(m_menuIdUndo);
		menu->remove(m_menuIdTitle);
	}
	m_menuIdTitle = -1;
	m_menuIdUndo = -1;
	m_menuIdRedo = -1;
}

bool appl::TextPluginHistory::onDataReceiveShortCut(appl::TextViewer& _textDrawer,
                                                    const etk::String& _shortCutName,
                                                    appl::PluginHistoryData& _data) {
	if (isEnable() == false) {
		return false;
	}
	if (_shortCutName == "appl::TextPluginHistory::Redo") {
		if (_data.m_redo.size() == 0) {
			return true;
		}
		if (_data.m_redo[_data.m_redo.size()-1] == null) {
			_data.m_redo.popBack();
			return true;
		}
		appl::History *tmpElement = _data.m_redo[_data.m_redo.size()-1];
		_data.m_redo.popBack();
		_data.m_undo.pushBack(tmpElement);
		_textDrawer.replaceDirect(tmpElement->m_addedText,
		                          _textDrawer.position(tmpElement->m_posAdded),
		                          _textDrawer.position(tmpElement->m_endPosRemoved) );
		
		return true;
	} else if (_shortCutName == "appl::TextPluginHistory::Undo") {
		if (_data.m_undo.size() == 0) {
			return true;
		}
		if (_data.m_undo[_data.m_undo.size()-1] == null) {
			_data.m_undo.popBack();
			return true;
		}
		appl::History *tmpElement = _data.m_undo[_data.m_undo.size()-1];
		_data.m_undo.popBack();
		_data.m_redo.pushBack(tmpElement);
		_textDrawer.replaceDirect(tmpElement->m_removedText,
		                          _textDrawer.position(tmpElement->m_posAdded),
		                          _textDrawer.position(tmpElement->m_endPosAdded) );
		
		return true;
	}
	return false;
}

void appl::TextPluginHistory::clearRedo(appl::PluginHistoryData& _data) {
	if (_data.m_redo.size() == 0) {
		return;
	}
	for (size_t iii=0; iii<_data.m_redo.size(); ++iii) {
		if (_data.m_redo[iii] == null) {
			continue;
		}
		ETK_DELETE(appl::History, _data.m_redo[iii]);
		_data.m_redo[iii] = null;
	}
	_data.m_redo.clear();
}

void appl::TextPluginHistory::clearUndo(appl::PluginHistoryData& _data) {
	if (_data.m_undo.size() == 0) {
		return;
	}
	for (size_t iii=0; iii<_data.m_undo.size(); ++iii) {
		if (_data.m_undo[iii] == null) {
			continue;
		}
		ETK_DELETE(appl::History, _data.m_undo[iii]);
		_data.m_undo[iii] = null;
	}
	_data.m_undo.clear();
}


bool appl::TextPluginHistory::onDataWrite(appl::TextViewer& _textDrawer,
                                          const appl::Buffer::Iterator& _pos,
                                          const etk::String& _strData,
                                          appl::PluginHistoryData& _data) {
	if (isEnable() == false) {
		return false;
	}
	appl::History *tmpElement = ETK_NEW(appl::History);
	if (tmpElement != null) {
		tmpElement->m_addedText = _strData;
		tmpElement->m_posAdded = (int64_t)_pos;
		tmpElement->m_endPosRemoved = (int64_t)_pos;
	} else {
		APPL_ERROR("History allocation error");
	}
	_textDrawer.writeDirect(_strData, _pos);
	if (tmpElement != null) {
		tmpElement->m_endPosAdded = (int64_t)_textDrawer.cursor();
		clearRedo(_data);
		_data.m_undo.pushBack(tmpElement);
	}
	ememory::SharedPtr<appl::textPluginManager> mng = m_pluginManager.lock();
	if (mng!=null) {
		mng->onCursorMove(_textDrawer, _textDrawer.cursor());
	}
	return true;
}

bool appl::TextPluginHistory::onDataReplace(appl::TextViewer& _textDrawer,
                                            const appl::Buffer::Iterator& _pos,
                                            const etk::String& _strData,
                                            const appl::Buffer::Iterator& _posEnd,
                                            appl::PluginHistoryData& _data) {
	if (isEnable() == false) {
		return false;
	}
	appl::History *tmpElement = ETK_NEW(appl::History);
	if (tmpElement != null) {
		tmpElement->m_posAdded = (int64_t)_pos;
		tmpElement->m_addedText = _strData;
		tmpElement->m_endPosRemoved = (int64_t)_posEnd;
		_textDrawer.copy(tmpElement->m_removedText, _pos, _posEnd);
	}
	_textDrawer.replaceDirect(_strData, _pos, _posEnd);
	if (tmpElement != null) {
		tmpElement->m_endPosAdded = (int64_t)_textDrawer.cursor();
		clearRedo(_data);
		_data.m_undo.pushBack(tmpElement);
	}
	ememory::SharedPtr<appl::textPluginManager> mng = m_pluginManager.lock();
	if (mng!=null) {
		mng->onCursorMove(_textDrawer, _textDrawer.cursor());
	}
	return true;
}

bool appl::TextPluginHistory::onDataRemove(appl::TextViewer& _textDrawer,
                                           const appl::Buffer::Iterator& _pos,
                                           const appl::Buffer::Iterator& _posEnd,
                                           appl::PluginHistoryData& _data) {
	if (isEnable() == false) {
		return false;
	}
	appl::History *tmpElement = ETK_NEW(appl::History);
	if (tmpElement != null) {
		tmpElement->m_addedText = "";
		tmpElement->m_posAdded = (int64_t)_pos;
		tmpElement->m_endPosAdded = tmpElement->m_posAdded;
		tmpElement->m_endPosRemoved = (int64_t)_posEnd;
		_textDrawer.copy(tmpElement->m_removedText, _pos, _posEnd);
		clearRedo(_data);
		_data.m_undo.pushBack(tmpElement);
	}
	_textDrawer.removeDirect();
	ememory::SharedPtr<appl::textPluginManager> mng = m_pluginManager.lock();
	if (mng!=null) {
		mng->onCursorMove(_textDrawer, _textDrawer.cursor());
	}
	return true;
}


