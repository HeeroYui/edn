/**
 *******************************************************************************
 * @file CodeView.cpp
 * @brief Editeur De N'ours : Code Viewer Widget
 *			This is an abstraction
 * @author Edouard DUPIN
 * @date 05/01/2011
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

#include <CodeView.h>

#include <appl/Debug.h>
#include <appl/global.h>
#include <CodeView.h>
#include <BufferManager.h>
#include <ColorizeManager.h>
#include <ewol/ClipBoard.h>
#include <SearchData.h>

#include <ewol/widget/WidgetManager.h>
#include <ewol/eObject/EObject.h>


#undef __class__
#define __class__	"CodeView"

void CodeView::Init(void)
{
	m_label = "CodeView is disable ...";
	
	m_bufferID = -1;
	m_buttunOneSelected = false;
	
	m_lineNumberList.Clear();
	
	m_textColorFg = draw::color::black;
	
	m_textColorBg = draw::color::black;
	m_textColorBg.a = 0x40;
	SetCanHaveFocus(true);
	RegisterMultiCast(ednMsgBufferId);
	RegisterMultiCast(ednMsgGuiCopy);
	RegisterMultiCast(ednMsgGuiPaste);
	RegisterMultiCast(ednMsgGuiCut);
	RegisterMultiCast(ednMsgGuiRedo);
	RegisterMultiCast(ednMsgGuiUndo);
	RegisterMultiCast(ednMsgGuiRm);
	RegisterMultiCast(ednMsgGuiSelect);
	RegisterMultiCast(ednMsgGuiChangeCharset);
	RegisterMultiCast(ednMsgGuiFind);
	RegisterMultiCast(ednMsgGuiReplace);
	SetLimitScrolling(0.2);
}

#ifdef APPL_BUFFER_FONT_DISTANCE_FIELD
	CodeView::CodeView(etk::UString fontName, int32_t fontSize) :
		m_OObjectText(fontName, fontSize)
	{
		Init();
	}
#endif

CodeView::CodeView(void)
{
	Init();
}

CodeView::~CodeView(void)
{
	
}


/**
 * @brief Check if the number of reference buffer is good or not ...
 * @param[in] bufferID id of the current Buffer that needed to have a reference
 * @return ---
 */
void CodeView::UpdateNumberOfLineReference(int32_t bufferID)
{
	Vector2D<float>  tmpCoord;
	tmpCoord.x = 0;
	tmpCoord.y = 0;
	if (m_lineNumberList.Size()<=bufferID) {
		// update the number of elements : 
		for (int32_t iii=m_lineNumberList.Size(); iii <= bufferID; iii++) {
			// add start line at 0 :
			m_lineNumberList.PushBack(tmpCoord);
		}
	}
}


bool CodeView::CalculateMinSize(void)
{
	m_minSize.x = 50;
	m_minSize.y = 50;
	return true;
}

void CodeView::CalculateMaxSize(void)
{
	m_maxSize.x = 2048;
	#ifdef APPL_BUFFER_FONT_DISTANCE_FIELD
		int32_t letterHeight = m_OObjectText.GetHeight();
	#else
		int32_t letterHeight = m_OObjectTextNormal.GetHeight();
	#endif
	m_maxSize.y = BufferManager::Get(m_bufferID)->GetNumberOfLine() * letterHeight;
}


void CodeView::OnDraw(ewol::DrawProperty& displayProp)
{
	m_OObjectsColored.Draw();
	#ifdef APPL_BUFFER_FONT_DISTANCE_FIELD
		m_OObjectText.Draw();
	#else
		m_OObjectTextNormal.Draw();
		m_OObjectTextBold.Draw();
		m_OObjectTextItalic.Draw();
		m_OObjectTextBoldItalic.Draw();
	#endif
	WidgetScrooled::OnDraw(displayProp);
}

void CodeView::OnRegenerateDisplay(void)
{
	if (true == NeedRedraw()) {
		int64_t startTime = ewol::GetTime();
		
		// For the scrooling windows
		CalculateMaxSize();
		
		#ifdef APPL_BUFFER_FONT_DISTANCE_FIELD
			m_OObjectText.Clear();
		#else
			m_OObjectTextNormal.Clear();
			m_OObjectTextBold.Clear();
			m_OObjectTextItalic.Clear();
			m_OObjectTextBoldItalic.Clear();
		#endif
		m_OObjectsColored.Clear();
		
		
		if(true == BufferManager::Get(m_bufferID)->NeedToUpdateDisplayPosition() ) {
			Vector2D<float>  borderWidth = BufferManager::Get(m_bufferID)->GetBorderSize();
			bool centerRequested = false;
			// TODO : set it back ...
			Vector2D<float>  currentPosition = BufferManager::Get(m_bufferID)->GetPosition(999/*m_OObjectTextNormal.GetFontID()*/, centerRequested);
			SetScrollingPositionDynamic(borderWidth, currentPosition, centerRequested);
		} // else : nothing to do ...
		
		// generate the objects :
		#ifdef APPL_BUFFER_FONT_DISTANCE_FIELD
			BufferManager::Get(m_bufferID)->Display(m_OObjectText,
			                                        m_OObjectsColored,
			                                        m_originScrooled.x, m_originScrooled.y, m_size.x, m_size.y);
		#else
			BufferManager::Get(m_bufferID)->Display(m_OObjectTextNormal,
			                                        m_OObjectTextBold,
			                                        m_OObjectTextItalic,
			                                        m_OObjectTextBoldItalic,
			                                        m_OObjectsColored,
			                                        m_originScrooled.x, m_originScrooled.y, m_size.x, m_size.y);
		#endif
		// set the current size of the windows
		SetMaxSize(BufferManager::Get(m_bufferID)->GetMaxSize());
		
		int64_t stopTime = ewol::GetTime();
		APPL_DEBUG("Display Code Generation = " << stopTime - startTime << " micro-s");
		
		// call the herited class...
		WidgetScrooled::OnRegenerateDisplay();
	}
}


bool CodeView::OnEventKb(ewol::eventKbType_te typeEvent, uniChar_t unicodeData)
{
	//APPL_DEBUG("KB EVENT : \"" << UTF8_data << "\" size=" << strlen(UTF8_data) << "type=" << (int32_t)typeEvent);
	if (typeEvent == ewol::EVENT_KB_TYPE_DOWN) {
		BufferManager::Get(m_bufferID)->AddChar(unicodeData);
		MarkToRedraw();
	}
	return true;
}


bool CodeView::OnEventKbMove(ewol::eventKbType_te typeEvent, ewol::eventKbMoveType_te moveTypeEvent)
{
	if (typeEvent == ewol::EVENT_KB_TYPE_DOWN) {
		BufferManager::Get(m_bufferID)->cursorMove(moveTypeEvent);
		MarkToRedraw();
	}
	return true;
}

/**
 * @brief Event on a past event ==> this event is asynchronous due to all system does not support direct getting datas
 * @note : need to have focus ...
 * @param[in] mode Mode of data requested
 * @return ---
 */
void CodeView::OnEventClipboard(ewol::clipBoard::clipboardListe_te clipboardID)
{
	BufferManager::Get(m_bufferID)->Paste(clipboardID);
	MarkToRedraw();
}

/**
 * @brief Event on an input of this Widget
 * @param[in] type Type of the input (ewol::INPUT_TYPE_MOUSE/ewol::INPUT_TYPE_FINGER ...)
 * @param[in] IdInput Id of the current Input (PC : left=1, right=2, middle=3, none=0 / Tactil : first finger=1 , second=2 (only on this widget, no knowledge at ouside finger))
 * @param[in] typeEvent ewol type of event like EVENT_INPUT_TYPE_DOWN/EVENT_INPUT_TYPE_MOVE/EVENT_INPUT_TYPE_UP/EVENT_INPUT_TYPE_SINGLE/EVENT_INPUT_TYPE_DOUBLE/...
 * @param[in] pos Absolute position of the event
 * @return true the event is used
 * @return false the event is not used
 */
bool CodeView::OnEventInput(ewol::inputType_te type, int32_t IdInput, ewol::eventInputType_te typeEvent, Vector2D<float>  pos)
{
	Vector2D<float>  relativePos = RelativePosition(pos);
	// corection for the openGl abstraction
	//relativePos.y = m_size.y - relativePos.y;
	
	Vector2D<float>  limitedPos = relativePos;
	limitedPos.x = etk_avg(1, limitedPos.x, m_size.x-1);
	limitedPos.y = etk_avg(1, limitedPos.y, m_size.y-1);
	
	if (m_bufferID < 0) {
		return false;
	}
	if (true == WidgetScrooled::OnEventInput(type, IdInput, typeEvent, pos)) {
		ewol::widgetManager::FocusKeep(this);
		// nothing to do ... done on upper widget ...
		return true;
	}
	if (1 == IdInput) {
		#ifndef __MODE__Touch
			if (ewol::EVENT_INPUT_TYPE_DOWN == typeEvent) {
				m_buttunOneSelected = true;
				ewol::widgetManager::FocusKeep(this);
				// TODO : Set something good
				BufferManager::Get(m_bufferID)->MouseEvent(limitedPos);
				MarkToRedraw();
			} else if (ewol::EVENT_INPUT_TYPE_UP == typeEvent) {
				m_buttunOneSelected = false;
				BufferManager::Get(m_bufferID)->Copy(ewol::clipBoard::CLIPBOARD_SELECTION);
				MarkToRedraw();
			} else 
		#endif
		if (ewol::EVENT_INPUT_TYPE_SINGLE == typeEvent) {
			#ifdef __MODE__Touch
				ewol::widgetManager::FocusKeep(this);
				BufferManager::Get(m_bufferID)->MouseEvent(limitedPos);
				MarkToRedraw();
			#else
				// nothing to do ...
			#endif
		} else if (ewol::EVENT_INPUT_TYPE_DOUBLE == typeEvent) {
			BufferManager::Get(m_bufferID)->MouseEventDouble();
			MarkToRedraw();
		} else if (ewol::EVENT_INPUT_TYPE_TRIPLE == typeEvent) {
			BufferManager::Get(m_bufferID)->MouseEventTriple();
			MarkToRedraw();
		} else if (ewol::EVENT_INPUT_TYPE_MOVE == typeEvent) {
			if (true == m_buttunOneSelected) {
				int xxx, yyy;
				xxx = relativePos.x;
				yyy = relativePos.y;
				if (xxx<0) {
					xxx = 0;
				}
				if (yyy<0) {
					yyy = 0;
				}
				//APPL_INFO("mouse-motion BT1 %d, %d", xxx, yyy);
				// TODO : Set something good
				BufferManager::Get(m_bufferID)->MouseSelectFromCursorTo(limitedPos);
				MarkToRedraw();
			}
		}
	} else if (2 == IdInput) {
		if (ewol::EVENT_INPUT_TYPE_SINGLE == typeEvent) {
			// TODO : Set something good
			BufferManager::Get(m_bufferID)->MouseEvent(limitedPos);
			ewol::clipBoard::Request(ewol::clipBoard::CLIPBOARD_SELECTION);
			ewol::widgetManager::FocusKeep(this);
		}
	}
	return true;
}



/**
 * @brief Receive a message from an other EObject with a specific eventId and data
 * @param[in] CallerObject Pointer on the EObject that information came from
 * @param[in] eventId Message registered by this class
 * @param[in] data Data registered by this class
 * @return ---
 */
void CodeView::OnReceiveMessage(ewol::EObject * CallerObject, const char * eventId, etk::UString data)
{
	ewol::WidgetScrooled::OnReceiveMessage(CallerObject, eventId, data);
	APPL_DEBUG("Extern Event : " << CallerObject << "  type : " << eventId << "  data=\"" << data << "\"");
	
	if(eventId == ednMsgBufferId) {
		//keep the reference of the display offset :
		if(    m_bufferID >=0
		    && m_bufferID < m_lineNumberList.Size()) {
			m_lineNumberList[m_bufferID] = m_originScrooled;
		}
		int32_t bufferID = 0;
		sscanf(data.c_str(), "%d", &bufferID);
		APPL_INFO("Select a new Buffer ... " << bufferID);
		// set the new buffer ID
		m_bufferID = bufferID;
		// update the start display position...
		UpdateNumberOfLineReference(m_bufferID);
		// set back if needed the display position ...
		if(    m_bufferID >=0
		    && m_bufferID < m_lineNumberList.Size()) {
			m_originScrooled = m_lineNumberList[m_bufferID];
		}
	} else if (eventId == ednMsgGuiCopy) {
		BufferManager::Get(m_bufferID)->Copy(ewol::clipBoard::CLIPBOARD_STD);
	} else if (eventId == ednMsgGuiCut) {
		BufferManager::Get(m_bufferID)->Cut(ewol::clipBoard::CLIPBOARD_STD);
	} else if (eventId == ednMsgGuiPaste) {
		ewol::clipBoard::Request(ewol::clipBoard::CLIPBOARD_STD);
	} else if (eventId == ednMsgGuiUndo) {
		BufferManager::Get(m_bufferID)->Undo();
	} else if (eventId == ednMsgGuiRedo) {
		BufferManager::Get(m_bufferID)->Redo();
	} else if (eventId == ednMsgGuiRm) {
		// data : "Word" "Line" "Paragraph"
		if (data == "Word") {
			APPL_WARNING(" on event " << eventId << " data=\"" << data << "\" ==> not coded" );
		} else if (data == "Line") {
			BufferManager::Get(m_bufferID)->RemoveLine();
		} else if (data == "Paragraph") {
			APPL_WARNING(" on event " << eventId << " data=\"" << data << "\" ==> not coded" );
		} else {
			APPL_ERROR(" on event " << eventId << " unknow data=\"" << data << "\"" );
		}
	} else if (eventId == ednMsgGuiSelect) {
		// data : "ALL" "NONE"
		if (data == "ALL") {
			BufferManager::Get(m_bufferID)->SelectAll();
		} else if (data == "NONE") {
			BufferManager::Get(m_bufferID)->SelectNone();
		} else {
			APPL_ERROR(" on event " << eventId << " unknow data=\"" << data << "\"" );
		}
	} else if (eventId == ednMsgGuiChangeCharset) {
		// data : "UTF-8" "ISO-8859-1" "ISO-8859-15"
		if (data == "UTF-8") {
			BufferManager::Get(m_bufferID)->SetCharset(unicode::EDN_CHARSET_UTF8);
		} else if (data == "ISO-8859-1") {
			BufferManager::Get(m_bufferID)->SetCharset(unicode::EDN_CHARSET_ISO_8859_1);
		} else if (data == "ISO-8859-15") {
			BufferManager::Get(m_bufferID)->SetCharset(unicode::EDN_CHARSET_ISO_8859_15);
		} else {
			APPL_ERROR(" on event " << eventId << " unknow data=\"" << data << "\"" );
		}
	} else if (eventId == ednMsgGuiFind) {
		etk::UString myDataString;
		SearchData::GetSearch(myDataString);
		if (data == "Next") {
			BufferManager::Get(m_bufferID)->Search(myDataString, false, SearchData::GetCase(), SearchData::GetWrap(), SearchData::GetRegExp() );
		} else if (data == "Previous") {
			BufferManager::Get(m_bufferID)->Search(myDataString, true, SearchData::GetCase(), SearchData::GetWrap(), SearchData::GetRegExp() );
		}
	} else if (eventId == ednMsgGuiReplace) {
		etk::UString myDataString;
		SearchData::GetReplace(myDataString);
		if (data == "Normal") {
			BufferManager::Get(m_bufferID)->Replace(myDataString);
		} else if (data == "All") {
			
		}
	}
	/*
	switch (id)
	{
		case APPL_MSG__CURRENT_GOTO_LINE:
			if (dataID<0) {
				dataID = 0;
			}
			BufferManager::Get(m_bufferID)->JumpAtLine(dataID);
			break;
		case APPL_MSG__CURRENT_SET_CHARSET:
			BufferManager::Get(m_bufferID)->SetCharset((unicode::charset_te)dataID);
			break;
	}
	*/
	// Force redraw of the widget
	MarkToRedraw();
}


void CodeView::OnGetFocus(void)
{
	/*
	ewol::widgetMessageMultiCast::Send(GetWidgetId(), ednMsgBufferId, m_bufferID);
	*/
	ewol::KeyboardShow();
	APPL_INFO("Focus - In");
}


void CodeView::OnLostFocus(void)
{
	ewol::KeyboardHide();
	APPL_INFO("Focus - out");
}

void CodeView::SetFontSize(int32_t size)
{
	#ifdef APPL_BUFFER_FONT_DISTANCE_FIELD
		m_OObjectText.SetSize(size);
	#else
		m_OObjectTextNormal.SetSize(size);
		m_OObjectTextBold.SetSize(size);
		m_OObjectTextItalic.SetSize(size);
		m_OObjectTextBoldItalic.SetSize(size);
	#endif
	SetScrollingSize(size*3.0*1.46); // 1.46 is a magic nmber ...
}

#ifdef APPL_BUFFER_FONT_DISTANCE_FIELD
	void CodeView::SetFontName(etk::UString fontName)
	{
		m_OObjectText.SetFont(fontName);
	}
#else
	void CodeView::SetFontNameNormal(etk::UString fontName)
	{
		m_OObjectTextNormal.SetFont(fontName);
	}
	
	void CodeView::SetFontNameBold(etk::UString fontName)
	{
		m_OObjectTextBold.SetFont(fontName);
	}
	
	void CodeView::SetFontNameItalic(etk::UString fontName)
	{
		m_OObjectTextItalic.SetFont(fontName);
	}
	
	void CodeView::SetFontNameBoldItalic(etk::UString fontName)
	{
		m_OObjectTextBoldItalic.SetFont(fontName);
	}
#endif