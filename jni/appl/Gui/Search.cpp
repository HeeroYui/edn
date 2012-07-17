/**
 *******************************************************************************
 * @file Search.cpp
 * @brief Editeur De N'ours : Search system
 * @author Edouard DUPIN
 * @date 03/01/2011
 * @par Project
 * Edn
 *
 * @par Copyright
 * Copyright 2010 Edouard DUPIN, all right reserved
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY.
 *
 * Licence summary : 
 *    You can modify and redistribute the sources code and binaries.
 *    You can send me the bug-fix
 *    You can not earn money with this Software (if the source extract from Edn
 *        represent less than 50% of original Sources)
 * Term of the licence in in the file licence.txt.
 *
 *******************************************************************************
 */


#include "appl/global.h"
#include "Search.h"
#include "SearchData.h"
#include "BufferManager.h"
#include "MainWindows.h"
#include "appl/globalMsg.h"

#include <ewol/widget/ButtonImage.h>


#undef __class__
#define __class__	"Search"

extern const char * const TYPE_EOBJECT_APPL_SEARCH = __class__;


const char* const l_eventSearchEntry        = "appl-search-entry";
const char* const l_eventSearchEntryEnter   = "appl-search-entry-enter";
const char* const l_eventReplaceEntry       = "appl-replace-entry";
const char* const l_eventReplaceEntryEnter  = "appl-replace-entry-enter";
const char* const l_eventSearchBt      = "appl-search-button";
const char* const l_eventReplaceBt     = "appl-replace-button";
const char* const l_eventCaseCb        = "appl-case-sensitive-CheckBox";
const char* const l_eventWrapCb        = "appl-wrap-CheckBox";
const char* const l_eventForwardCb     = "appl-forward-CheckBox";
const char* const l_eventHideBt        = "appl-hide-button";

Search::Search(void) :
	m_searchEntry(NULL),
	m_replaceEntry(NULL)
{
	m_forward = false;
	
	ewol::ButtonImage * myButtonImage = NULL;
	
	m_searchEntry = new ewol::Entry();
	if (NULL == m_searchEntry) {
		APPL_ERROR("Widget allocation error ==> it will missing in the display");
	} else {
		m_searchEntry->RegisterOnEvent(this, ewolEventEntryModify, l_eventSearchEntry);
		m_searchEntry->RegisterOnEvent(this, ewolEventEntryEnter,  l_eventSearchEntryEnter);
		m_searchEntry->SetExpendX(true);
		m_searchEntry->SetFillX(true);
		SubWidgetAdd(m_searchEntry);
	}
	
	myButtonImage = new ewol::ButtonImage("icon/Search.svg");
	if (NULL == myButtonImage) {
		APPL_ERROR("Widget allocation error ==> it will missing in the display");
	} else {
		myButtonImage->SetMinSize(32,32);
		myButtonImage->RegisterOnEvent(this, ewolEventButtonPressed, l_eventSearchBt);
		SubWidgetAdd(myButtonImage);
	}
	
	m_replaceEntry = new ewol::Entry();
	if (NULL == m_replaceEntry) {
		APPL_ERROR("Widget allocation error ==> it will missing in the display");
	} else {
		m_replaceEntry->RegisterOnEvent(this, ewolEventEntryModify, l_eventReplaceEntry);
		m_replaceEntry->RegisterOnEvent(this, ewolEventEntryEnter, l_eventReplaceEntryEnter);
		m_replaceEntry->SetExpendX(true);
		m_replaceEntry->SetFillX(true);
		SubWidgetAdd(m_replaceEntry);
	}
	
	myButtonImage = new ewol::ButtonImage("icon/Replace.svg");
	if (NULL == myButtonImage) {
		APPL_ERROR("Widget allocation error ==> it will missing in the display");
	} else {
		myButtonImage->SetMinSize(32,32);
		myButtonImage->RegisterOnEvent(this, ewolEventButtonPressed, l_eventReplaceBt);
		SubWidgetAdd(myButtonImage);
	}
	
	myButtonImage = new ewol::ButtonImage("icon/CaseSensitive.svg");
	if (NULL == myButtonImage) {
		APPL_ERROR("Widget allocation error ==> it will missing in the display");
	} else {
		myButtonImage->SetImageSelected("icon/CaseSensitive.svg", 0xFFFFFF5F);
		myButtonImage->SetMinSize(32,32);
		myButtonImage->SetToggleMode(true);
		myButtonImage->SetValue(SearchData::GetCase());
		myButtonImage->RegisterOnEvent(this, ewolEventButtonPressed, l_eventCaseCb);
		SubWidgetAdd(myButtonImage);
	}
	
	myButtonImage = new ewol::ButtonImage("icon/WrapAround.svg");
	if (NULL == myButtonImage) {
		APPL_ERROR("Widget allocation error ==> it will missing in the display");
	} else {
		myButtonImage->SetImageSelected("icon/WrapAround.svg", 0xFFFFFF5F);
		myButtonImage->SetMinSize(32,32);
		myButtonImage->SetToggleMode(true);
		myButtonImage->SetValue(SearchData::GetWrap());
		myButtonImage->RegisterOnEvent(this, ewolEventButtonPressed, l_eventWrapCb);
		SubWidgetAdd(myButtonImage);
	}
	
	myButtonImage = new ewol::ButtonImage("icon/Up.svg");
	if (NULL == myButtonImage) {
		APPL_ERROR("Widget allocation error ==> it will missing in the display");
	} else {
		myButtonImage->SetImageSelected("icon/Down.svg");
		myButtonImage->SetMinSize(32,32);
		myButtonImage->SetToggleMode(true);
		myButtonImage->SetValue(m_forward);
		myButtonImage->RegisterOnEvent(this, ewolEventButtonPressed, l_eventForwardCb);
		SubWidgetAdd(myButtonImage);
	}
	
	myButtonImage = new ewol::ButtonImage("icon/Forbidden.svg");
	if (NULL == myButtonImage) {
		APPL_ERROR("Widget allocation error ==> it will missing in the display");
	} else {
		myButtonImage->SetMinSize(32,32);
		myButtonImage->RegisterOnEvent(this, ewolEventButtonPressed, l_eventHideBt);
		SubWidgetAdd(myButtonImage);
	}
	
	RegisterMultiCast(ednMsgGuiSearch);
	// basicly hiden ...
	Hide();
}

Search::~Search(void)
{
	
}

/**
 * @brief Check if the object has the specific type.
 * @note In Embended platforme, it is many time no -rtti flag, then it is not possible to use dynamic cast ==> this will replace it
 * @param[in] objectType type of the object we want to check
 * @return true if the object is compatible, otherwise false
 */
bool Search::CheckObjectType(const char * const objectType)
{
	if (NULL == objectType) {
		APPL_ERROR("check error : \"" << TYPE_EOBJECT_APPL_SEARCH << "\" != NULL(pointer) ");
		return false;
	}
	if (objectType == TYPE_EOBJECT_APPL_SEARCH) {
		return true;
	} else {
		if(true == ewol::SizerHori::CheckObjectType(objectType)) {
			return true;
		}
		APPL_ERROR("check error : \"" << TYPE_EOBJECT_APPL_SEARCH << "\" != \"" << objectType << "\"");
		return false;
	}
}

/**
 * @brief Get the current Object type of the EObject
 * @note In Embended platforme, it is many time no -rtti flag, then it is not possible to use dynamic cast ==> this will replace it
 * @param[in] objectType type description
 * @return true if the object is compatible, otherwise false
 */
const char * const Search::GetObjectType(void)
{
	return TYPE_EOBJECT_APPL_SEARCH;
}

/**
 * @brief Receive a message from an other EObject with a specific eventId and data
 * @param[in] CallerObject Pointer on the EObject that information came from
 * @param[in] eventId Message registered by this class
 * @param[in] data Data registered by this class
 * @return ---
 */
void Search::OnReceiveMessage(ewol::EObject * CallerObject, const char * eventId, etk::UString data)
{
	ewol::SizerHori::OnReceiveMessage(CallerObject, eventId, data);
	//APPL_INFO("Search receive message : \"" << eventId << "\" data=\"" << data << "\"");
	if ( eventId == l_eventSearchEntry) {
		SearchData::SetSearch(data);
	} else if ( eventId == l_eventSearchEntryEnter) {
		SearchData::SetSearch(data);
		if (true==m_forward) {
			SendMultiCast(ednMsgGuiFind, "Previous");
		} else {
			SendMultiCast(ednMsgGuiFind, "Next");
		}
	} else if ( eventId == l_eventReplaceEntry) {
		SearchData::SetReplace(data);
	} else if ( eventId == l_eventReplaceEntryEnter) {
		SearchData::SetReplace(data);
		SendMultiCast(ednMsgGuiReplace, "Normal");
		if (true==m_forward) {
			SendMultiCast(ednMsgGuiFind, "Previous");
		} else {
			SendMultiCast(ednMsgGuiFind, "Next");
		}
	} else if ( eventId == l_eventSearchBt) {
		if (true==m_forward) {
			SendMultiCast(ednMsgGuiFind, "Previous");
		} else {
			SendMultiCast(ednMsgGuiFind, "Next");
		}
	} else if ( eventId == l_eventReplaceBt) {
		SendMultiCast(ednMsgGuiReplace, "Normal");
		if (true==m_forward) {
			SendMultiCast(ednMsgGuiFind, "Previous");
		} else {
			SendMultiCast(ednMsgGuiFind, "Next");
		}
	} else if ( eventId == l_eventCaseCb) {
		if (data == "true") {
			SearchData::SetCase(true);
		} else {
			SearchData::SetCase(false);
		}
	} else if ( eventId == l_eventWrapCb) {
		if (data == "true") {
			SearchData::SetWrap(true);
		} else {
			SearchData::SetWrap(false);
		}
	} else if ( eventId == l_eventForwardCb) {
		if (data == "true") {
			m_forward = true;
		} else {
			m_forward = false;
		}
	} else if ( eventId == l_eventHideBt) {
		Hide();
	} else if ( eventId == ednMsgGuiSearch) {
		if (true == IsHide()) {
			Show();
			if (m_searchEntry!= NULL) {
				m_searchEntry->KeepFocus();
			}
		} else {
			if(    (m_searchEntry!=NULL  && true==m_searchEntry->GetFocus())
			    || (m_replaceEntry!=NULL && true==m_replaceEntry->GetFocus()) ) {
				Hide();
			} else if (m_searchEntry!= NULL) {
				m_searchEntry->KeepFocus();
			} else {
				Hide();
			}
		}
	}
}

/**
 * @brief Inform object that an other object is removed ...
 * @param[in] removeObject Pointer on the EObject remeved ==> the user must remove all reference on this EObject
 * @note : Sub classes must call this class
 * @return ---
 */
void Search::OnObjectRemove(ewol::EObject * removeObject)
{
	ewol::SizerHori::OnObjectRemove(removeObject);
	if (removeObject == m_searchEntry) {
		m_searchEntry = NULL;
	}
	if (removeObject == m_replaceEntry) {
		m_replaceEntry = NULL;
	}
}

