/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#include "appl/global.hpp"
#include "appl/Gui/Search.hpp"
#include "appl/BufferManager.hpp"
#include "appl/Gui/TextViewer.hpp"
#include "appl/Gui/MainWindows.hpp"
#include "appl/globalMsg.hpp"
#include <ewol/widget/Button.hpp>
#include <ewol/widget/Entry.hpp>

appl::widget::Search::Search() :
  m_forward(true),
  m_caseSensitive(false),
  m_wrap(true) {
	addObjectType("appl::widget::Search");
}
void appl::widget::Search::init() {
	ewol::widget::Composer::init();
	loadFromFile("DATA:///GUI-Search.xml", getId());
	m_viewerManager = appl::ViewerManager::create();
	// link event
	subBind(ewol::widget::Button, "[" + etk::toString(getId()) + "]SEARCH:close",         signalPressed, sharedFromThis(), &appl::widget::Search::OnCallbackHide);
	subBind(ewol::widget::Entry,  "[" + etk::toString(getId()) + "]SEARCH:search-entry",  signalModify,  sharedFromThis(), &appl::widget::Search::OnCallbackSearchValue);
	subBind(ewol::widget::Entry,  "[" + etk::toString(getId()) + "]SEARCH:search-entry",  signalEnter,   sharedFromThis(), &appl::widget::Search::OnCallbackSearchEntryValidate);
	subBind(ewol::widget::Button, "[" + etk::toString(getId()) + "]SEARCH:search",        signalPressed, sharedFromThis(), &appl::widget::Search::OnCallbackSearch);
	subBind(ewol::widget::Entry,  "[" + etk::toString(getId()) + "]SEARCH:replace-entry", signalModify,  sharedFromThis(), &appl::widget::Search::OnCallbackReplaceValue);
	subBind(ewol::widget::Entry,  "[" + etk::toString(getId()) + "]SEARCH:replace-entry", signalEnter,   sharedFromThis(), &appl::widget::Search::OnCallbackReplaceEntryValidate);
	subBind(ewol::widget::Button, "[" + etk::toString(getId()) + "]SEARCH:replace",       signalPressed, sharedFromThis(), &appl::widget::Search::OnCallbackReplace);
	subBind(ewol::widget::Button, "[" + etk::toString(getId()) + "]SEARCH:case",          signalValue,   sharedFromThis(), &appl::widget::Search::OnCallbackCase);
	subBind(ewol::widget::Button, "[" + etk::toString(getId()) + "]SEARCH:wrap",          signalValue,   sharedFromThis(), &appl::widget::Search::OnCallbackWrap);
	subBind(ewol::widget::Button, "[" + etk::toString(getId()) + "]SEARCH:up-down",       signalValue,   sharedFromThis(), &appl::widget::Search::OnCallbackForward);
	// set default properties
	propertySetOnWidgetNamed("[" + etk::toString(getId()) + "]SEARCH:case", "value", etk::toString(m_caseSensitive));
	propertySetOnWidgetNamed("[" + etk::toString(getId()) + "]SEARCH:wrap", "value", etk::toString(m_wrap));
	propertySetOnWidgetNamed("[" + etk::toString(getId()) + "]SEARCH:up-down", "value", etk::toString(m_forward));
	// get widget
	m_searchEntry = ememory::dynamicPointerCast<ewol::widget::Entry>(getSubObjectNamed("[" + etk::toString(getId()) + "]SEARCH:search-entry"));
	m_replaceEntry = ememory::dynamicPointerCast<ewol::widget::Entry>(getSubObjectNamed("[" + etk::toString(getId()) + "]SEARCH:replace-entry"));
	// basicly hiden ...
	propertyHide.set(true);
}

appl::widget::Search::~Search() {
	
}

void appl::widget::Search::find() {
	if (m_viewerManager == null) {
		APPL_WARNING("No viewer manager selected!!!");
		return;
	}
	ememory::SharedPtr<appl::TextViewer> viewer = m_viewerManager->getViewerSelected();
	if (viewer == null) {
		APPL_INFO("No viewer selected!!!");
		return;
	}
	viewer->unSelect();
	appl::Buffer::Iterator resultStart;
	appl::Buffer::Iterator resultStop;
	if (m_forward == true) {
		if (viewer->find(viewer->cursor(), m_searchData, resultStart, resultStop, m_caseSensitive) == false) {
			if (    m_wrap == false 
			     || viewer->find(viewer->begin(), m_searchData, resultStart, resultStop, m_caseSensitive) == false) {
				// TODO : Display an IHM pop-up
				APPL_WARNING("No element find ...");
				return;
			}
		}
		viewer->select(resultStart, resultStop);
	} else {
		if (viewer->rfind(viewer->cursor(), m_searchData, resultStart, resultStop, m_caseSensitive) == false) {
			if (    m_wrap == false 
			     || viewer->rfind(viewer->end(), m_searchData, resultStart, resultStop, m_caseSensitive) == false) {
				// TODO : Display an IHM pop-up
				APPL_WARNING("No element find ...");
				return;
			}
		}
		viewer->select(resultStop, resultStart);
	}
}

void appl::widget::Search::replace() {
	if (m_viewerManager == null) {
		APPL_WARNING("No viewer manager selected!!!");
		return;
	}
	ememory::SharedPtr<appl::TextViewer> viewer = m_viewerManager->getViewerSelected();
	if (viewer == null) {
		APPL_INFO("No viewer selected!!!");
		return;
	}
	if (viewer->hasTextSelected() == false) {
		// nothing to replace ...
		return;
	}
	viewer->replace(m_replaceData);
}

void appl::widget::Search::OnCallbackHide() {
	propertyHide.set(true);
}
void appl::widget::Search::OnCallbackSearchValue(const etk::String& _value) {
	m_searchData = etk::toUString(_value);
}
void appl::widget::Search::OnCallbackSearch() {
	find();
}
void appl::widget::Search::OnCallbackSearchEntryValidate(const etk::String& _value) {
	m_searchData = etk::toUString(_value);
	find();
}
void appl::widget::Search::OnCallbackReplaceValue(const etk::String& _value) {
	m_replaceData = etk::toUString(_value);
}
void appl::widget::Search::OnCallbackReplace() {
	replace();
	find();
}
void appl::widget::Search::OnCallbackReplaceEntryValidate(const etk::String& _value) {
	m_replaceData = etk::toUString(_value);
	replace();
	find();
}
void appl::widget::Search::OnCallbackCase(const bool& _value) {
	m_caseSensitive = _value;
}
void appl::widget::Search::OnCallbackWrap(const bool& _value) {
	m_wrap = _value;
}
void appl::widget::Search::OnCallbackForward(const bool& _value) {
	m_forward = _value;
}

void appl::widget::Search::selectSearch() {
	if (m_searchEntry != null) {
		m_searchEntry->keepFocus();
	}
}

bool appl::widget::Search::isSelectSearch() {
	if (m_searchEntry != null) {
		return m_searchEntry->getFocus();
	}
	return false;
}

void appl::widget::Search::selectReplace() {
	if (m_replaceEntry != null) {
		m_replaceEntry->keepFocus();
	}
}

bool appl::widget::Search::isSelectReplace() {
	if (m_replaceEntry != null) {
		return m_replaceEntry->getFocus();
	}
	return false;
}

