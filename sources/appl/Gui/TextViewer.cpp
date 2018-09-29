/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#include <appl/debug.hpp>
#include <appl/global.hpp>
#include <appl/Gui/TextViewer.hpp>
#include <appl/BufferManager.hpp>
#include <gale/context/clipBoard.hpp>

#include <ewol/widget/Manager.hpp>
#include <appl/Gui/ViewerManager.hpp>
#include <ewol/object/Object.hpp>
#include <appl/TextPluginManager.hpp>
#include <etk/stdTools.hpp>

#define tic() \
	int64_t startTime = ewol::getTime();

#define toc(comment) \
	int64_t endTime = ewol::getTime(); \
	int64_t processTimeLocal = (endTime - startTime); \
	APPL_DEBUG(comment << (float)((float)processTimeLocal / 1000.0) << "ms");


appl::TextViewer::TextViewer() :
  propertyFontName(this, "font-name", "FreeMono;DejaVuSansMono;FreeSerif", "Name of the font for the displayed text", &appl::TextViewer::onChangePropertyFontName),
  propertyFontSize(this, "font-size", 12, "Size of the font for the displayed text", &appl::TextViewer::onChangePropertyFontSize),
  m_insertMode(false) {
	addObjectType("appl::TextViewer");
	setLimitScrolling(0.2);
	setSingleFinger(false);
	
	// load buffer manager:
	m_bufferManager = appl::BufferManager::create();
	m_pluginManager = appl::textPluginManager::create();
	m_viewerManager = appl::ViewerManager::create();
	
	// load color properties
	m_paintingProperties = appl::GlyphPainting::create("THEME:COLOR:textViewer.json");
	// get all id properties ...
	m_colorBackground = m_paintingProperties->request("CODE_basicBackgroung");
	m_colorSpace = m_paintingProperties->request("CODE_space");
	m_colorTabulation = m_paintingProperties->request("CODE_tabulation");
	m_colorCursor = m_paintingProperties->request("CODE_cursor");
	m_colorLineNumber = m_paintingProperties->request("CODE_lineNumber");
	m_colorSelection = m_paintingProperties->request("SelectedText");
	m_colorNormal = m_paintingProperties->request("normal");
}

void appl::TextViewer::init() {
	ewol::widget::WidgetScrolled::init();
	propertyCanFocus.set(true);
	m_displayText.setFont(*propertyFontName, *propertyFontSize);
	m_pluginManager->connect(*this);
	// last created has focus ...
	setCurrentSelect();
	signalShortcut.connect(sharedFromThis(), &appl::TextViewer::onCallbackShortCut);
	
	/*
	registerMultiCast(ednMsgBufferId);
	registerMultiCast(ednMsgGuiFind);
	registerMultiCast(ednMsgGuiReplace);
	registerMultiCast(appl::MsgSelectGotoLine);
	registerMultiCast(appl::MsgSelectGotoLineSelect);
	*/
	if (m_bufferManager != null) {
		m_bufferManager->signalSelectFile.connect(sharedFromThis(), &appl::TextViewer::onCallbackselectNewFile);
	} else {
		APPL_CRITICAL("Buffer manager has not been created at the init");
	}
}

appl::TextViewer::~TextViewer() {
	m_pluginManager->disconnect(*this);
}

void appl::TextViewer::onCallbackShortCut(const etk::String& _value) {
	if (m_pluginManager->onReceiveShortCut(*this, _value) == true) {
		return;
	}
}


void appl::TextViewer::onCallbackselectNewFile(const ememory::SharedPtr<appl::Buffer>& _value) {
	if (_value == null) {
		APPL_INFO("Select new file: ___NO-FILE___");
	} else {
		APPL_INFO("Select new file: " << _value->getFileName());
	}
	if (isSelectedLast() == false) {
		return;
	}
	
	// reset scroll:
	if (m_buffer != null) {
		m_buffer->signals.disconnect(sharedFromThis());
		bool needAdd = true;
		auto it = m_drawingRemenber.begin();
		while (it != m_drawingRemenber.end()) {
			ememory::SharedPtr<appl::Buffer> tmpBuff = it->first.lock();
			if (tmpBuff == null) {
				it = m_drawingRemenber.erase(it);
				continue;
			}
			if (tmpBuff == m_buffer) {
				it->second = m_originScrooled;
				APPL_VERBOSE("store origin : " << m_originScrooled);
				needAdd = false;
				break;
			}
			++it;
		}
		if (needAdd == true) {
			m_drawingRemenber.pushBack(etk::makePair(ememory::WeakPtr<appl::Buffer>(m_buffer), m_originScrooled));
			APPL_VERBOSE("Push origin : " << m_originScrooled);
		}
	}
	m_originScrooled = vec2(0,0);
	if (m_bufferManager != null) {
		m_buffer = _value;
		m_bufferManager->setBufferSelected(m_buffer);
		if (m_buffer != null) {
			m_buffer->signalIsModify.connect(sharedFromThis(), &appl::TextViewer::onCallbackIsModify);
			m_buffer->signalSelectChange.connect(sharedFromThis(), &appl::TextViewer::onCallbackSelectChange);
			for (auto element : m_drawingRemenber) {
				if (element.first.lock() == m_buffer) {
					m_originScrooled = element.second;
					APPL_VERBOSE("retrive origin : " << m_originScrooled);
					// TODO : Check if this element is not out of the display text ...
					break;
				}
			}
		}
	}
	markToRedraw();
	return;
}

etk::String appl::TextViewer::getBufferPath() {
	if (m_buffer == null) {
		return "";
	}
	return m_buffer->getFileName().getParent().getString();
}


bool appl::TextViewer::calculateMinSize() {
	m_minSize.setValue(50,50);
	return true;
}


void appl::TextViewer::onDraw() {
	m_displayDrawing.draw();
	m_displayText.draw();
	WidgetScrolled::onDraw();
}

void appl::TextViewer::onRegenerateDisplay() {
	if (needRedraw() == false) {
		return;
	}
	//tic();
	// For the scrooling windows
	m_displayDrawing.clear();
	m_displayText.clear();
	
	// reset the background :
	m_displayDrawing.setPos(vec3(0, 0, 0));
	m_displayDrawing.setColor((*m_paintingProperties)[m_colorBackground].getForeground());
	m_displayDrawing.rectangleWidth(m_size);
	
	if (m_buffer == null) {
		m_maxSize.setX(256);
		m_maxSize.setY(256);
		float textEndAlignament = etk::max(11.0f, m_size.x()-20.0f);
		m_displayText.setTextAlignement(10, textEndAlignament, ewol::compositing::alignLeft);
		m_displayText.setRelPos(vec3(10, 0, 0));
		etk::String tmpString("<br/>\n"
		                       "<font color=\"red\">\n"
		                       "	<b>\n"
		                       "		edn - Editeur De N'ours\n"
		                       "	</b>\n"
		                       "</font>\n"
		                       "<br/>\n"
		                       "<br/>\n"
		                       "<font color=\"indigo\">\n"
		                       "	<i>\n"
		                       "		No Buffer Availlable to display\n" + etk::toString(m_size) + 
		                       "	</i>\n"
		                       "</font>\n");
		m_displayText.setPos(vec3(0.0f, m_size.y(), 0.0f) );
		m_displayText.forceLineReturn();
		m_displayText.printDecorated(tmpString);
		// call the herited class...
		WidgetScrolled::onRegenerateDisplay();
		return;
	}
	// normal displa of the buffer :
	vec3 tmpCursorPosition(0, 0, -1);
	float tmpCursorLenght = -1.0;
	// real display ...
	m_displayText.setColor(etk::Color<>(0, 0, 0, 255));
	float countNbLine = 1;
	int32_t countColomn = 0;
	// the siplay string :
	etk::UString stringToDisplay;
	appl::Buffer::Iterator selectPosStart = m_buffer->begin();
	appl::Buffer::Iterator selectPosStop = m_buffer->begin();
	if (m_buffer->hasTextSelected() == true) {
		selectPosStart = m_buffer->selectStart();
		selectPosStop = m_buffer->selectStop();
	}
	m_displayText.setPos(vec3(-m_originScrooled.x(), m_size.y()+m_originScrooled.y(), 0));
	m_displayText.forceLineReturn();
	appl::Buffer::Iterator startingIt = m_buffer->begin();
	int64_t startLineId = 0;
	if (m_size.y() < m_displayText.getPos().y()) {
		for (startingIt = m_buffer->begin();
		     (bool)startingIt == true;
		     ++startingIt) {
			if (*startingIt == u32char::Return) {
				++startLineId;
				m_displayText.forceLineReturn();
				if (m_size.y() >= m_displayText.getPos().y()) {
					++startingIt;
					break;
				}
			}
		}
	}
	// Display line number :
	m_lastOffsetDisplay = 0;
	vec3 tmpLetterSize = m_displayText.calculateSize((char32_t)'A');
	{
		int32_t nbLine = m_buffer->getNumberOfLines();
		float nbLineCalc = nbLine;
		int32_t nbChar = 0;
		while (nbLineCalc >= 1.0f) {
			++nbChar;
			nbLineCalc /= 10.0f;
		}
		m_lastOffsetDisplay = tmpLetterSize.x() * (float)nbChar + 1.0f;
		m_displayText.setFontItalic((*m_paintingProperties)[m_colorLineNumber].getItalic());
		m_displayText.setFontBold((*m_paintingProperties)[m_colorLineNumber].getBold());
		m_displayText.setColorBg((*m_paintingProperties)[m_colorLineNumber].getBackground());
		m_displayText.setColor((*m_paintingProperties)[m_colorLineNumber].getForeground());
		m_displayText.setClippingMode(false);
		
		vec3 startWriteRealPosition = m_displayText.getPos();
		m_displayText.setPos(vec3(0.0f, startWriteRealPosition.y(), 0.0f));
		for (int32_t iii=startLineId;
		     iii<nbLine;
		     ++iii) {
			char tmpLineNumber[50];
			sprintf(tmpLineNumber, "%*d", nbChar, iii+1);
			m_displayText.print(tmpLineNumber);
			m_displayText.forceLineReturn();
			if (m_displayText.getPos().y() < -20.0f ) {
				break;
			}
		}
		m_displayText.setPos(vec3(-m_originScrooled.x()+m_lastOffsetDisplay, startWriteRealPosition.y(), 0.0f));
		m_displayText.setClipping(vec2(m_lastOffsetDisplay, 0), m_size);
	}
	appl::DisplayHLData displayLocalSyntax;
	m_buffer->hightlightGenerateLines(displayLocalSyntax, startingIt, (m_size.y()/tmpLetterSize.y()) + 5);
	float maxSizeX = 0;
	appl::HighlightInfo * HLColor = null;
	bool displayCursorAndSelection = isSelectedLast();
	appl::Buffer::Iterator it;
	for (it = startingIt;
	     (bool)it == true;
	     ++it) {
		if (it == m_buffer->cursor()) {
			// need to display the cursor :
			tmpCursorPosition = m_displayText.getPos();
			tmpCursorLenght = 0.0f;
		}
		//APPL_DEBUG("display element '" << currentValue << "'at pos : " << m_displayText.getPos() );
		//APPL_DEBUG(" element size : " << iii << " : " << bufferElementSize);
		if (*it == u32char::Return) {
			countNbLine += 1;
			countColomn = 0;
			maxSizeX = etk::max(m_displayText.getPos().x(), maxSizeX);
			// Display the end line position only if we have the focus ...
			if (   (    displayCursorAndSelection == true
			         && it >= selectPosStart
			         && it < selectPosStop)
			    || globals::isSetDisplayEndOfLine() == true) {
				ewol::compositing::Drawing& draw = m_displayText.getDrawing();
				draw.setColor(etk::Color<>(0xFF, 0x00, 0x00, 0xFF));
				draw.setPos(m_displayText.getPos() + tmpLetterSize/4.0f);
				draw.rectangle(m_displayText.getPos() + tmpLetterSize*3.0f/4.0f);
			}
			if (tmpCursorLenght == 0.0f) {
				tmpCursorLenght = tmpLetterSize.x();
			}
			m_displayText.forceLineReturn();
			m_displayText.setPos(vec3(-m_originScrooled.x()+m_lastOffsetDisplay, m_displayText.getPos().y(), 0.0f));
			if (m_displayText.getPos().y() < -20.0f ) {
				break;
			}
			continue;
		}
		HLColor = m_buffer->getElementColorAtPosition(displayLocalSyntax, (int64_t)it);
		bool haveBackground = false;
		if (    HLColor != null
		     && HLColor->patern != null) {
			m_displayText.setColor(HLColor->patern->getColorGlyph().getForeground());
			m_displayText.setColorBg(HLColor->patern->getColorGlyph().getBackground());
			haveBackground = HLColor->patern->getColorGlyph().haveBackground();
			m_displayText.setFontItalic(HLColor->patern->getColorGlyph().getItalic());
			m_displayText.setFontBold(HLColor->patern->getColorGlyph().getBold());
		} else {
			m_displayText.setFontItalic((*m_paintingProperties)[m_colorNormal].getItalic());
			m_displayText.setFontBold((*m_paintingProperties)[m_colorNormal].getBold());
			m_displayText.setColorBg((*m_paintingProperties)[m_colorNormal].getBackground());
			m_displayText.setColor((*m_paintingProperties)[m_colorNormal].getForeground());
		}
		if (haveBackground == false) {
			if (    *it == u32char::Space
			     && globals::isSetDisplaySpaceChar() == true) {
				m_displayText.setColorBg((*m_paintingProperties)[m_colorSpace].getForeground());
			} else if (    *it == u32char::Tabulation
			            && globals::isSetDisplayTabChar() == true) {
				m_displayText.setColorBg((*m_paintingProperties)[m_colorTabulation].getForeground());
			}
		}
		m_buffer->expand(countColomn, *it, stringToDisplay);
		// Display selection only if we have the focus ...
		if (displayCursorAndSelection == true) {
			if (it >= selectPosStart && it < selectPosStop) {
				m_displayText.setColor((*m_paintingProperties)[m_colorSelection].getForeground());
				m_displayText.setColorBg((*m_paintingProperties)[m_colorSelection].getBackground());
			}
		}
		//APPL_DEBUG("display : '" << currentValue << "'  == > '" << stringToDisplay << "'");
		m_displayText.print(stringToDisplay);
		if (tmpCursorLenght == 0.0f) {
			tmpCursorLenght = m_displayText.getPos().x()-tmpCursorPosition.x();
		}
		countColomn += stringToDisplay.size();
	}
	if (it == m_buffer->cursor()) {
		tmpCursorPosition = m_displayText.getPos();
		tmpCursorLenght = 5;
	}
	maxSizeX = etk::max(m_displayText.getPos().x(), maxSizeX);
	// Display cursor only if we have the focus ...
	if (    tmpCursorPosition.z() != -1
	     && getFocus() == true) {
		// display the cursor:
		//APPL_DEBUG("display cursor at position : " << tmpCursorPosition);
		m_displayText.setPos(tmpCursorPosition);
		if (m_buffer->hasTextSelected() == true) {
			m_displayText.setColorBg((*m_paintingProperties)[m_colorCursor].getForeground());
			m_displayText.printCursor(false);
		} else {
			if (m_insertMode == true) {
				m_displayText.setColorBg((*m_paintingProperties)[m_colorSelection].getBackground());
			} else {
				m_displayText.setColorBg((*m_paintingProperties)[m_colorCursor].getForeground());
			}
			m_displayText.printCursor(m_insertMode, tmpCursorLenght);
		}
	}
	// set maximum size (X&Y) :
	{
		vec3 tmpLetterSize = m_displayText.calculateSize((char32_t)'A');
		int64_t nbLines = m_buffer->getNumberOfLines();
		m_maxSize.setX(maxSizeX+m_originScrooled.x());
		m_maxSize.setY((float)nbLines*tmpLetterSize.y());
	}
	//toc("Display time : ");
	// call the herited class...
	WidgetScrolled::onRegenerateDisplay();
}

bool appl::TextViewer::onEventEntry(const ewol::event::Entry& _event) {
	if (m_buffer == null) {
		return false;
	}
	// First call plugin
	if (m_pluginManager->onEventEntry(*this, _event) == true) {
		markToRedraw();
		return true;
	}
	// just forward event  == > manage directly in the buffer
	if (_event.getType() == gale::key::keyboard::character) {
		//APPL_DEBUG("KB EVENT : \"" << UTF8_data << "\" size=" << strlen(UTF8_data) << "type=" << (int32_t)typeEvent);
		if (_event.getStatus() != gale::key::status::down) {
			return false;
		}
		char32_t localValue = _event.getChar();
		if (localValue == u32char::Return) {
			if (_event.getSpecialKey().getShift() == true) {
				localValue = u32char::CarrierReturn;
			}
		} else if (localValue == u32char::Suppress ) {
			//APPL_INFO("keyEvent : <suppr> pos=" << m_cursorPos);
			if (m_buffer->hasTextSelected()) {
				remove();
			} else {
				appl::Buffer::Iterator pos = m_buffer->cursor();
				appl::Buffer::Iterator posEnd = pos;
				++posEnd;
				replace("", pos, posEnd);
			}
			return true;
		} else if (localValue == u32char::Delete) {
			//APPL_INFO("keyEvent : <del> pos=" << m_cursorPos);
			if (m_buffer->hasTextSelected()) {
				remove();
			} else {
				appl::Buffer::Iterator pos = m_buffer->cursor();
				appl::Buffer::Iterator posEnd = pos;
				--pos;
				replace("", pos, posEnd);
			}
			return true;
		}
		m_buffer->setSelectMode(false);
		// normal adding char ...
		char output[5];
		output[0] = '0';
		u32char::convertUtf8(localValue, output);
		if (    m_buffer->hasTextSelected() == false
		     && _event.getSpecialKey().getInsert() == true) {
			appl::Buffer::Iterator pos = m_buffer->cursor();
			appl::Buffer::Iterator posEnd = pos;
			++posEnd;
			replace(output, pos, posEnd);
			//TODO : choice UTF  ... replace(localValue, pos, posEnd);
		} else {
			etk::String myString = output;
			write(myString);
		}
		return true;
	}
	// move events ...
	if (_event.getStatus() == gale::key::status::down) {
		// selection when shift is set:
		m_buffer->setSelectMode(_event.getSpecialKey().getShift());
		// check selection event ...
		switch(_event.getType()) {
			case gale::key::keyboard::insert:
				m_insertMode = m_insertMode==true?false:true;
				markToRedraw();
				break;
			case gale::key::keyboard::left:
				//APPL_INFO("keyEvent : <LEFT>");
				moveCursorLeft();
				break;
			case gale::key::keyboard::right:
				//APPL_INFO("keyEvent : <RIGHT>");
				moveCursorRight();
				break;
			case gale::key::keyboard::up:
				//APPL_INFO("keyEvent : <UP>");
				moveCursorUp(1);
				break;
			case gale::key::keyboard::down:
				//APPL_INFO("keyEvent : <DOWN>");
				moveCursorDown(1);
				break;
			case gale::key::keyboard::pageUp:
				//APPL_INFO("keyEvent : <PAGE-UP>");
				moveCursorUp(15); // TODO : Set the real number of line ...
				break;
			case gale::key::keyboard::pageDown:
				//APPL_INFO("keyEvent : <PAGE-DOWN>");
				moveCursorDown(15); // TODO : Set the real number of line ...
				break;
			case gale::key::keyboard::start:
				//APPL_INFO("keyEvent : <Start of line>");
				moveCursorLeft(moveEnd);
				break;
			case gale::key::keyboard::end:
				//APPL_INFO("keyEvent : <End of line>");
				moveCursorRight(moveEnd);
				break;
			default:
				break;
		}
		return true;
	}
	return false;
}

bool appl::TextViewer::onEventInput(const ewol::event::Input& _event) {
	if (   _event.getId() != 0
	    && _event.getStatus() == gale::key::status::down) {
		keepFocus();
	}
	//tic();
	if (m_buffer == null) {
		return false;
	}
	// First call the scrolling widget :
	if (ewol::widget::WidgetScrolled::onEventInput(_event) == true) {
		markToRedraw();
		return true;
	}
	// Second call plugin
	if (m_pluginManager->onEventInput(*this, _event) == true) {
		markToRedraw();
		return true;
	}
	vec2 relativePos = relativePosition(_event.getPos());
	// offset for the lineNumber:
	relativePos -= vec2(m_lastOffsetDisplay, 0);
	// offset for the scrolling:
	relativePos += vec2(m_originScrooled.x(), -m_originScrooled.y());
	// invert for the buffer event ...
	relativePos.setY(m_size.y()-relativePos.y());
	if (relativePos.x()<0) {
		relativePos.setX(0);
	}
	if (    _event.getId() == 12
	     && _event.getStatus() == gale::key::status::pressSingle) {
		APPL_TODO("RAT5 SAVE button ==> TODO implement");
		// Rat5 save event
		//sendMultiCast(ednMsgGuiSave, "current");
		return true;
	}
	// just forward event  == > manage directly in the buffer
	if (_event.getId() == 1) {
		// mouse selection :
		//if (_event.getType() == gale::key::typeMouse) {
			if (_event.getStatus() == gale::key::status::down) {
				//if (_event.getSpecialKey().isSetShift() == false) {
					appl::Buffer::Iterator newPos = getMousePosition(relativePos);
					m_buffer->setSelectMode(false);
					moveCursor(newPos);
					m_buffer->setSelectMode(true);
					markToRedraw();
					return true;
				//}
			} else if (_event.getStatus() == gale::key::status::up) {
				appl::Buffer::Iterator newPos = getMousePosition(relativePos);
				moveCursor(newPos);
				m_buffer->setSelectMode(false);
				// Copy selection :
				etk::String value;
				m_buffer->copy(value);
				if (value.size() != 0) {
					gale::context::clipBoard::set(gale::context::clipBoard::clipboardSelection, value);
				}
				markToRedraw();
				return true;
			}
		//}
		if (_event.getStatus() == gale::key::status::pressSingle) {
			if (    _event.getType() == gale::key::type::mouse
			     || _event.getType() == gale::key::type::finger) {
				appl::Buffer::Iterator newPos = getMousePosition(relativePos);
				moveCursor(newPos);
				markToRedraw();
				return true;
			}
		} else if (_event.getStatus() == gale::key::status::pressDouble) {
			mouseEventDouble();
			// Copy selection :
			etk::String value;
			m_buffer->copy(value);
			if (value.size() != 0) {
				gale::context::clipBoard::set(gale::context::clipBoard::clipboardSelection, value);
			}
			markToRedraw();
			return true;
		} else if (_event.getStatus() == gale::key::status::pressTriple) {
			mouseEventTriple();
			// Copy selection :
			etk::String value;
			m_buffer->copy(value);
			if (value.size() != 0) {
				gale::context::clipBoard::set(gale::context::clipBoard::clipboardSelection, value);
			}
			markToRedraw();
			return true;
		} else if (_event.getStatus() == gale::key::status::move) {
			if (m_buffer->getSelectMode() == true) {
				//int64_t timeStart = ewol::getTime();
				appl::Buffer::Iterator newPos = getMousePosition(relativePos);
				//int64_t timeMedium1 = ewol::getTime();
				moveCursor(newPos);
				//int64_t timeMedium2 = ewol::getTime();
				markToRedraw();
				/*
				int64_t timeStop = ewol::getTime();
				APPL_DEBUG("Display selection=" << (timeStop-timeStart)/1000.0f << " ms");
				APPL_DEBUG("                1=" << (timeMedium1-timeStart)/1000.0f << " ms");
				APPL_DEBUG("                2=" << (timeMedium2-timeMedium1)/1000.0f << " ms");
				*/
				return true;
			}
		}
	} else if (2 == _event.getId()) {
		if (gale::key::status::pressSingle == _event.getStatus()) {
			appl::Buffer::Iterator newPos = getMousePosition(relativePos);
			moveCursor(newPos);
			gale::context::clipBoard::request(gale::context::clipBoard::clipboardSelection);
			markToRedraw();
			return true;
		}
	}
	return false;
}


void appl::TextViewer::mouseEventDouble() {
	//m_selectMode = false;
	appl::Buffer::Iterator beginPos, endPos;
	if (true == m_buffer->getPosAround(m_buffer->cursor(), beginPos, endPos)) {
		moveCursor(endPos);
		m_buffer->setSelectionPos(beginPos);
	}
}

void appl::TextViewer::mouseEventTriple() {
	//m_selectMode = false;
	moveCursor(m_buffer->getEndLine(m_buffer->cursor()));
	m_buffer->setSelectionPos(m_buffer->getStartLine(m_buffer->cursor()));
}

// TODO : optimise this with retaine the display position buffer and his position in the real view ...
appl::Buffer::Iterator appl::TextViewer::getMousePosition(const vec2& _relativePos) {
	char32_t currentValue;
	vec3 positionCurentDisplay(0,0,0);
	vec3 tmpLetterSize = m_displayText.calculateSize((char32_t)'A');
	int32_t countColomn = 0;
	etk::UString stringToDisplay;
	m_displayText.clear();
	m_displayText.forceLineReturn();
	positionCurentDisplay = m_displayText.getPos();
	for (appl::Buffer::Iterator it = m_buffer->begin();
	     (bool)it == true;
	     ++it) {
		currentValue = *it;
		if (currentValue == u32char::Return) {
			m_displayText.forceLineReturn();
			countColomn = 0;
		} else {
			if (-_relativePos.y() >= positionCurentDisplay.y()) {
				m_buffer->expand(countColomn, currentValue, stringToDisplay);
				for (size_t kkk=0; kkk<stringToDisplay.size(); ++kkk) {
					if (stringToDisplay[kkk] == u32char::Return) {
						m_displayText.forceLineReturn();
						countColomn = 0;
					} else {
						//note : Without this condithion the time od selection change to 0.6 ms to 8ms ...
						//APPL_DEBUG("check : " << -_relativePos.y() << ">=" << positionCurentDisplay.y());
						m_displayText.printChar(stringToDisplay[kkk]);
						++countColomn;
					}
				}
			}
		}
		if (-_relativePos.y() >= positionCurentDisplay.y()) {
			if (-_relativePos.y() < positionCurentDisplay.y()+tmpLetterSize.y()) {
				APPL_VERBOSE("line position : '" << (char)(*it) << "' = '" << stringToDisplay << "' n=" << countColomn << " " <<positionCurentDisplay.x() << " < " << _relativePos.x() << " < " << m_displayText.getPos().x() );
				if (    _relativePos.x() >= positionCurentDisplay.x()
				     && _relativePos.x() < m_displayText.getPos().x() ) {
					APPL_VERBOSE("find ...");
					return it;
				}
			} else {
				// previous line ...
				return --it;
			}
		}
		positionCurentDisplay = m_displayText.getPos();
	}
	return m_buffer->end();
}

void appl::TextViewer::onEventClipboard(enum gale::context::clipBoard::clipboardListe _clipboardID) {
	if (m_buffer != null) {
		etk::String data = gale::context::clipBoard::get(_clipboardID);
		write(data);
	}
	markToRedraw();
}

void appl::TextViewer::onCallbackIsModify() {
	markToRedraw();
}
void appl::TextViewer::onCallbackSelectChange() {
	markToRedraw();
}


void appl::TextViewer::onGetFocus() {
	showKeyboard();
	APPL_INFO("Focus - In");
	setCurrentSelect();
	markToRedraw();
}

void appl::TextViewer::onLostFocus() {
	hideKeyboard();
	APPL_INFO("Focus - out");
	markToRedraw();
}

void appl::TextViewer::onChangePropertyFontSize() {
	m_displayText.setFontSize(*propertyFontSize);
	setScrollingSize(*propertyFontSize*3.0*1.46); // 1.46 is a magic number ...
}

void appl::TextViewer::onChangePropertyFontName() {
	m_displayText.setFontName(*propertyFontName);
}

// TODO : Update process time ==> a little expensive (2->4ms) in end of file
void appl::TextViewer::updateScrolling() {
	if (m_buffer == null) {
		return;
	}
	vec2 realCursorPosition(0,0);
	uint32_t lineId = m_buffer->getCursorLinesId();
	m_displayText.clear();
	m_displayText.forceLineReturn();
	float lineSize = -m_displayText.getPos().y();
	for (size_t iii=0; iii<lineId; ++iii) {
		m_displayText.forceLineReturn();
	}
	realCursorPosition.setY(-m_displayText.getPos().y());
	realCursorPosition.setX(getScreenSize(m_buffer->getStartLine(m_buffer->cursor()), m_buffer->cursor()));
	APPL_VERBOSE("position=" << realCursorPosition << " scrool=" << m_originScrooled << " size" << m_size);
	if (realCursorPosition.x() < m_originScrooled.x()+lineSize*2.0f) {
		m_originScrooled.setX(realCursorPosition.x()-lineSize*2.0f);
	} else if (realCursorPosition.x() > m_originScrooled.x()+(m_size.x()-m_lastOffsetDisplay)-lineSize*2.0f-10) {
		m_originScrooled.setX(realCursorPosition.x()-(m_size.x()-m_lastOffsetDisplay)+lineSize*2.0f+10);
	}
	if (realCursorPosition.y() < m_originScrooled.y()+lineSize*2.0f) {
		m_originScrooled.setY(realCursorPosition.y()-lineSize*2.0f);
	} else if (realCursorPosition.y() > m_originScrooled.y()+m_size.y()-lineSize*2.0f) {
		m_originScrooled.setY(realCursorPosition.y()-m_size.y()+lineSize*2.0f);
	}
	m_originScrooled.setMax(vec2(0,0));
	// TODO : Limit min position too ...
}

bool appl::TextViewer::moveCursor(const appl::Buffer::Iterator& _pos) {
	if (m_buffer == null) {
		return false;
	}
	markToRedraw();
	if (m_pluginManager->onCursorMove(*this, _pos) == true) {
		updateScrolling();
		return true;
	}
	m_buffer->moveCursor((int64_t)_pos);
	updateScrolling();
	return true;
}

bool appl::TextViewer::write(const etk::String& _data) {
	if (m_buffer == null) {
		return false;
	}
	if (m_buffer->hasTextSelected() == true) {
		return replace(_data);
	}
	return write(_data, m_buffer->cursor());
}

bool appl::TextViewer::write(const etk::String& _data, const appl::Buffer::Iterator& _pos) {
	if (m_buffer == null) {
		return false;
	}
	markToRedraw();
	if (m_pluginManager->onWrite(*this, _pos, _data) == true) {
		// no call of the move cursor, because pluging might call theses function to copy and cut data...
		updateScrolling();
		return true;
	}
	bool ret = m_buffer->write(_data, _pos);
	m_pluginManager->onCursorMove(*this, m_buffer->cursor());
	updateScrolling();
	return ret;
}

bool appl::TextViewer::replace(const etk::String& _data, const appl::Buffer::Iterator& _pos, const appl::Buffer::Iterator& _posEnd) {
	if (m_buffer == null) {
		return false;
	}
	markToRedraw();
	if (m_pluginManager->onReplace(*this, _pos, _data, _posEnd) == true) {
		// no call of the move cursor, because pluging might call theses function to copy and cut data...
		updateScrolling();
		return true;
	}
	bool ret = m_buffer->replace(_data, _pos, _posEnd);
	m_pluginManager->onCursorMove(*this, m_buffer->cursor());
	updateScrolling();
	return ret;
}

bool appl::TextViewer::replace(const etk::String& _data) {
	if (m_buffer == null) {
		return false;
	}
	if (m_buffer->hasTextSelected() == false) {
		return write(_data);
	}
	return replace(_data, m_buffer->selectStart(), m_buffer->selectStop());
}

void appl::TextViewer::remove() {
	if (m_buffer == null) {
		return;
	}
	if (m_buffer->hasTextSelected() == false) {
		// nothing to do ...
		return;
	}
	markToRedraw();
	if (m_pluginManager->onRemove(*this, m_buffer->selectStart(), m_buffer->selectStop()) == true) {
		return;
	}
	m_buffer->removeSelection();
	m_pluginManager->onCursorMove(*this, m_buffer->cursor());
}



void appl::TextViewer::moveCursorRight(appl::TextViewer::moveMode _mode) {
	if (m_buffer == null) {
		return;
	}
	markToRedraw();
	appl::Buffer::Iterator it;
	switch (_mode) {
		default:
		case moveLetter:
			it = m_buffer->cursor();
			++it;
			moveCursor(it);
			break;
		case moveWord:
			// TODO : ...
			break;
		case moveEnd:
			it = m_buffer->getEndLine(m_buffer->cursor());
			moveCursor(it);
			break;
	}
}

void appl::TextViewer::moveCursorLeft(appl::TextViewer::moveMode _mode) {
	if (m_buffer == null) {
		return;
	}
	markToRedraw();
	appl::Buffer::Iterator it;
	switch (_mode) {
		default:
		case moveLetter:
			it = m_buffer->cursor();
			--it;
			moveCursor(it);
			break;
		case moveWord:
			// TODO : ...
			break;
		case moveEnd:
			it = m_buffer->getStartLine(m_buffer->cursor());
			moveCursor(it);
			break;
	}
}

void appl::TextViewer::moveCursorUp(uint32_t _nbLine) {
	if (m_buffer == null) {
		return;
	}
	markToRedraw();
	// find the position of the start of the line.
	appl::Buffer::Iterator lineStartPos = m_buffer->getStartLine(m_buffer->cursor());
	// check if we can go up ...
	if (lineStartPos == m_buffer->begin()) {
		return;
	}
	// Decide what column to move to, if there's a preferred column use that
	if (m_buffer->getFavoriteUpDownPos() < 0) {
		m_buffer->setFavoriteUpDownPos(getScreenSize(lineStartPos, m_buffer->cursor()));
	}
	// get the previous line
	appl::Buffer::Iterator prevLineStartPos = m_buffer->countBackwardNLines(lineStartPos-1, _nbLine);
	//APPL_INFO("Move line UP result : prevLineStartPos=" << prevLineStartPos);
	// get the display char position
	appl::Buffer::Iterator newPos = getPosSize(prevLineStartPos, m_buffer->getFavoriteUpDownPos());
	//APPL_INFO("Move to colomn : column=" << column << " newPos=" << newPos);
	float posStore = m_buffer->getFavoriteUpDownPos();
	moveCursor(newPos);
	m_buffer->setFavoriteUpDownPos(posStore);
}

void appl::TextViewer::moveCursorDown(uint32_t _nbLine) {
	if (m_buffer == null) {
		return;
	}
	markToRedraw();
	// check if we are not at the end of Buffer
	if (m_buffer->cursor() == m_buffer->end() ) {
		return;
	}
	// find the position of the start of the line.
	appl::Buffer::Iterator lineStartPos = m_buffer->getStartLine(m_buffer->cursor());
	
	if (m_buffer->getFavoriteUpDownPos() < 0) {
		m_buffer->setFavoriteUpDownPos(getScreenSize(lineStartPos, m_buffer->cursor()));
	}
	// get the next line :
	appl::Buffer::Iterator nextLineStartPos = m_buffer->countForwardNLines(lineStartPos, _nbLine);
	//APPL_INFO("Move line DOWN result : nextLineStartPos=" << nextLineStartPos);
	// get the display char position
	appl::Buffer::Iterator newPos = getPosSize(nextLineStartPos, m_buffer->getFavoriteUpDownPos());
	//APPL_INFO("Move to colomn : column=" << column << " newPos=" << newPos);
	float posStore = m_buffer->getFavoriteUpDownPos();
	moveCursor(newPos);
	m_buffer->setFavoriteUpDownPos(posStore);
}

// TODO : Rename ...
appl::Buffer::Iterator appl::TextViewer::getPosSize(const appl::Buffer::Iterator& _startLinePos, float _distance) {
	char32_t currentValue;
	int32_t countColomn = 0;
	etk::UString stringToDisplay;
	m_displayText.clear();
	m_displayText.forceLineReturn();
	for (appl::Buffer::Iterator it = _startLinePos;
	     (bool)it == true;
	     ++it) {
		currentValue = *it;
		m_buffer->expand(countColomn, currentValue, stringToDisplay);
		for (size_t kkk=0; kkk<stringToDisplay.size(); ++kkk) {
			if (stringToDisplay[kkk] == u32char::Return) {
				return it;
			} else {
				m_displayText.printChar(stringToDisplay[kkk]);
			}
		}
		if (m_displayText.getPos().x() >= _distance) {
			return it;
		}
		countColomn += stringToDisplay.size();
	}
	return m_buffer->end();
}

// TODO : Rename ...
float appl::TextViewer::getScreenSize(const appl::Buffer::Iterator& _startLinePos, const appl::Buffer::Iterator& _stopPos) {
	float ret = 0;
	char32_t currentValue;
	int32_t countColomn = 0;
	etk::UString stringToDisplay;
	m_displayText.clear();
	
	for (appl::Buffer::Iterator it = _startLinePos;
	     (bool)it == true && it <= _stopPos;
	     ++it) {
		currentValue = *it;
		//APPL_DEBUG("parse : " << currentValue);
		m_buffer->expand(countColomn, currentValue, stringToDisplay);
		for (size_t kkk=0; kkk<stringToDisplay.size(); ++kkk) {
			if (stringToDisplay[kkk] == u32char::Return) {
				return m_displayText.getPos().x() + 2; // TODO : Add the +2 for the end of line ...
			} else {
				m_displayText.printChar(stringToDisplay[kkk]);
			}
		}
		ret = m_displayText.getPos().x();
		countColomn += stringToDisplay.size();
	}
	return ret;
}

void appl::TextViewer::setCurrentSelect() {
	if (m_viewerManager != null) {
		m_viewerManager->setViewerSelected(ememory::dynamicPointerCast<appl::TextViewer>(sharedFromThis()), m_buffer);
	}
}

bool appl::TextViewer::isSelectedLast() {
	if (m_viewerManager != null) {
		return m_viewerManager->isLastSelected(ememory::dynamicPointerCast<appl::TextViewer>(sharedFromThis()));
	}
	return false;
}
