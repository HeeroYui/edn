/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#include <etk/types.hpp>
#include <etk/stdTools.hpp>
#include <etk/system.hpp>
#include <etk/path/fileSystem.hpp>

#include <appl/Buffer.hpp>
#include <appl/debug.hpp>
#include <gale/context/clipBoard.hpp>
#include <appl/HighlightManager.hpp>

appl::Buffer::Iterator& appl::Buffer::Iterator::operator++ () {
	m_value = u32char::Null;
	if (m_current < 0) {
		m_current = 0;
		return *this;
	}
	if (m_data != null) {
		if (m_current < (int64_t)m_data->m_data.size() ) {
			int8_t nbChar = utf8::length(m_data->m_data[m_current]);
			if (nbChar != 0) {
				m_current+=nbChar;
			} else {
				m_current++;
			}
		}
		if (m_current >= (int64_t)m_data->m_data.size()) {
			m_current = m_data->m_data.size();
		}
	}
	return *this;
}

appl::Buffer::Iterator& appl::Buffer::Iterator::operator-- () {
	m_value = u32char::Null;
	if (m_data != null) {
		if (m_current > 0) {
			int32_t iii = -1;
			while(    utf8::first(m_data->m_data[m_current+iii]) == false
			       && iii >= -6
			       && m_current-iii>0) {
				--iii;
			};
			m_current += iii;
		} else {
			m_current = -1;
		}
		return *this;
	} else {
		m_current = -1;
	}
	return *this;
}

char32_t appl::Buffer::Iterator::operator* () {
	if (m_value != u32char::Null) {
		return m_value;
	}
	if (m_data == null) {
		APPL_ERROR("request an element that iterator not link");
		return m_value;
	}
	if (    m_current < 0
	     || m_current >= (int64_t)m_data->m_data.size()) {
		APPL_ERROR("request an element out of bounding !!! 0 <= " << m_current << " < " << m_data->m_data.size());
		return m_value;
	}
	char tmpVal[5];
	memset(tmpVal, 0, sizeof(tmpVal));
	tmpVal[0] = m_data->m_data[m_current];
	int8_t nbChar = utf8::length(tmpVal[0]);
	for (int32_t iii=1; iii<nbChar && m_current+iii<(int64_t)m_data->m_data.size(); ++iii) {
		tmpVal[iii] = m_data->m_data[m_current+iii];
	}
	// transform ...
	m_value = utf8::convertChar32(tmpVal);
	return m_value;
}


appl::Buffer::Iterator appl::Buffer::position(int64_t _pos) {
	return appl::Buffer::Iterator(this, _pos);
}

appl::Buffer::Iterator appl::Buffer::begin() {
	return position(0);
}

appl::Buffer::Iterator appl::Buffer::end() {
	// TODO : chek the validity of the char ...
	return position( m_data.size() );
}

appl::Buffer::Iterator appl::Buffer::cursor() {
	if (m_cursorPos<= 0) {
		return begin();
	}
	return position( m_cursorPos );
}

appl::Buffer::Iterator appl::Buffer::selectStart() {
	return position( getStartSelectionPos() );
}

appl::Buffer::Iterator appl::Buffer::selectStop() {
	return position( getStopSelectionPos() );
}

appl::Buffer::Buffer() :
  signalFileIsModify(this, "file-is-modify", ""),
  signalIsModify(this, "is-modify", ""),
  signalIsSave(this, "is-save", ""),
  signalSelectChange(this, "select-change", ""),
  signalChangeName(this, "change-name", ""),
  m_hasFileName(false),
  m_fileName(""),
  m_isModify(false),
  m_cursorPos(0),
  m_cursorSelectPos(-1),
  m_cursorPreferredCol(-1),
  m_nbLines(1),
  m_highlight(null) {
	addObjectType("appl::Buffer");
	static int32_t bufferBaseId = 0;
	m_fileName = "No Name " + etk::toString(bufferBaseId);
	bufferBaseId++;
}

void appl::Buffer::init() {
	ewol::Object::init();
}

appl::Buffer::~Buffer() {
	APPL_ERROR("REAL remove buffer : '" << propertyName << "'");
}

bool appl::Buffer::loadFile(const etk::Path& _name) {
	APPL_DEBUG("Convert filename :'" << _name << "'");
	m_fileName = _name;
	m_hasFileName = true;
	m_isModify = true;
	m_cursorPos = 0;
	setHighlightType("");
	m_nbLines = 0;
	if (m_data.dumpFrom(_name) == true ) {
		countNumberofLine();
		tryFindHighlightType();
		m_isModify = false;
		m_fileIsModify = false;
		return true;
	}
	return false;
}

void appl::Buffer::setFileName(const etk::Path& _name) {
	APPL_DEBUG("Convert filename :'" << _name << "'");
	if (m_fileName == _name) {
		return;
	}
	m_fileName = _name;
	m_hasFileName = true;
	signalChangeName.emit();
	setModification(true);
}

bool appl::Buffer::storeFile() {
	if (m_data.dumpIn(m_fileName) == true) {
		APPL_INFO("saving file : " << m_fileName);
		etk::String extention = m_fileName.getExtention();
		if (etk::isIn(extention, {"cpp", "hpp", "h", "hpp"}) {
			etk::Path clangFile = etk::path::findInParent(m_fileName, ".clang-format");
			if (etk::path::isFile(clangFile) == true) {
				etk::String output = etk::exec("clang-format " + m_fileName.getAbsolute());// + " -assume-filename=" + clangFile.getAbsolute() + "
				etk::String tmp = m_data.getString();
				/*
				APPL_WARNING(" input = '" << tmp << "'");
				APPL_ERROR(" output = '" << output << "'");
				*/
				if (tmp != output) {
					/*
					etk::io::File file(m_fileName + "_tmp");
					file.open(etk::io::OpenMode::Write);
					file.write(&tmp[0], 1, tmp.size());
					file.close();
					*/
					APPL_ERROR(" ==> data is differents");
					m_fileIsModify = true;
					setModification(true);
					signalFileIsModify();
					return true;
				}
			}
		}
		setModification(false);
		return true;
	}
	return false;
}

void appl::Buffer::setModification(bool _status) {
	if (m_isModify == _status) {
		return;
	}
	m_isModify = _status;
	if (m_isModify == true) {
		signalIsModify.emit();
	} else {
		signalIsSave.emit();
	}
}

// TODO : Naming error
void appl::Buffer::countNumberofLine() {
	m_nbLines = 1;
	for (Iterator it = begin();
	     (bool)it == true;
	     ++it) {
		if (*it == u32char::Return) {
			++m_nbLines;
		}
	}
}


appl::Buffer::Iterator appl::Buffer::getStartLine(const appl::Buffer::Iterator& _pos) {
	appl::Buffer::Iterator startPos;
	if (false == searchBack(_pos, u32char::Return, startPos)) {
		return begin();
	}
	// note search will return the position of \n ==> the lione start just after ...
	return startPos+1;
}

appl::Buffer::Iterator appl::Buffer::getEndLine(const appl::Buffer::Iterator& _pos) {
	appl::Buffer::Iterator endPos;
	if (false == search(_pos, u32char::Return, endPos)) {
		endPos = end();
	}
	// Note the line end at the \n
	return endPos;
}


bool appl::Buffer::search(const appl::Buffer::Iterator& _pos, const char32_t& _search, appl::Buffer::Iterator& _result) {
	// move in the string
	for (Iterator it = _pos;
	     (bool)it == true;
	     ++it) {
		if (*it == _search) {
			_result = it;
			return true;
		}
	}
	_result = end();
	return false;
}

bool appl::Buffer::searchBack(const appl::Buffer::Iterator& _pos, const char32_t& _search, appl::Buffer::Iterator& _result) {
	// move in the string
	for (Iterator it = _pos - 1;
	     (bool)it == true;
	     --it) {
		//APPL_DEBUG("compare : " << *it << " ?= " << _search);
		if (*it == _search) {
			//APPL_DEBUG("find : " << (int32_t)it);
			_result = it;
			return true;
		}
	}
	_result = begin();
	return false;
}

bool appl::Buffer::search(const appl::Buffer::Iterator& _pos,
                          const etk::UString& _search,
                          appl::Buffer::Iterator& _result,
                          bool _caseSensitive) {
	if (_search.size() <= 0 ) {
		return false;
	}
	if (_caseSensitive == true) {
		// move in the string
		for (Iterator it = _pos;
		     (bool)it == true;
		     ++it) {
			if (*it == _search[0]) {
				// find the first char ==> check next...
				bool find = true;
				Iterator tmp = it;
				for (size_t iii=0; iii<_search.size(); ++iii) {
					if (*tmp != _search[iii]) {
						find = false;
						break;
					}
					++tmp;
					if ((bool)tmp == false) {
						if (iii != _search.size()-1) {
							find = false;
						}
						break;
					}
				}
				if (find == true) {
					_result = it;
					return true;
				}
			}
		}
	} else {
		char32_t firstElement = u32char::toLower(_search[0]);
		// move in the string
		for (Iterator it = _pos;
		     (bool)it == true;
		     ++it) {
			if ((char32_t)u32char::toLower(*it) == firstElement) {
				// find the first char ==> check next...
				bool find = true;
				Iterator tmp = it;
				for (size_t iii=0; iii<_search.size(); ++iii) {
					if (u32char::toLower(*tmp) != u32char::toLower(_search[iii])) {
						find = false;
						break;
					}
					++tmp;
					if ((bool)tmp == false) {
						if (iii != _search.size()-1) {
							find = false;
						}
						break;
					}
				}
				if (find == true) {
					_result = it;
					return true;
				}
			}
		}
	}
	_result = end();
	return false;
}

bool appl::Buffer::searchBack(const appl::Buffer::Iterator& _pos,
                              const etk::UString& _search,
                              appl::Buffer::Iterator& _result,
                              bool _caseSensitive) {
	if (_search.size() <= 0 ) {
		return false;
	}
	char32_t lastElement = _search[_search.size()-1];
	if (_caseSensitive == true) {
		// move in the string
		for (Iterator it = _pos - 1;
		     (bool)it == true;
		     --it) {
			//APPL_DEBUG("compare : " << *it << " ?= " << _search);
			if (*it == lastElement) {
				// find the last char ==> check previous...
				bool find = true;
				_result = it;
				for (int64_t iii=_search.size()-1; iii>=0; --iii) {
					if (*_result != _search[iii]) {
						find = false;
						break;
					}
					--_result;
					if ((bool)_result == false) {
						if (iii != 0) {
							find = false;
						}
						break;
					}
				}
				if (find == true) {
					_result++;
					return true;
				}
			}
		}
	} else {
		lastElement = u32char::toLower(lastElement);
		// move in the string
		for (Iterator it = _pos - 1;
		     (bool)it == true;
		     --it) {
			//APPL_DEBUG("compare : " << *it << " ?= " << _search);
			if ((char32_t)u32char::toLower(*it) == lastElement) {
				// find the last char ==> check previous...
				bool find = true;
				_result = it;
				for (int64_t iii=_search.size()-1; iii>=0; --iii) {
					if (u32char::toLower(*_result) != u32char::toLower(_search[iii])) {
						find = false;
						break;
					}
					--_result;
					if ((bool)_result == false) {
						if (iii != 0) {
							find = false;
						}
						break;
					}
				}
				if (find == true) {
					_result++;
					return true;
				}
			}
		}
	}
	_result = begin();
	return false;
}

void appl::Buffer::moveCursor(int64_t _pos) {
	m_cursorPreferredCol = -1;
	APPL_VERBOSE("move cursor : " << _pos << "/" << m_data.size());
	// selecting mode ...
	if (m_selectMode == true) {
		if (m_cursorSelectPos == -1) {
			m_cursorSelectPos = m_cursorPos;
			if (m_cursorSelectPos < 0) {
				m_cursorSelectPos = 0;
			}
		}
		//APPL_DEBUG("Select : " << m_cursorSelectPos << " ==> " << _pos);
		m_cursorPos = _pos;
		if (m_cursorPos == m_cursorSelectPos) {
			m_cursorSelectPos = -1;
		}
		signalSelectChange.emit();
		return;
	}
	// move mode
	m_cursorPos = _pos;
	m_cursorSelectPos = -1;
	signalSelectChange.emit();
}

bool appl::Buffer::getPosAround(const appl::Buffer::Iterator& _startPos,
                                appl::Buffer::Iterator &_beginPos,
                                appl::Buffer::Iterator &_endPos) {
	char32_t currentValue = *position(_startPos);
	_beginPos = begin();
	_endPos = end();
	if (    currentValue == u32char::Tabulation
	     || currentValue == u32char::Space) {
		APPL_DEBUG("select spacer");
		// Search back
		for (Iterator it = --position(_startPos);
		     (bool)it == true;
		     --it) {
			currentValue = *it;
			if (    currentValue != u32char::Tabulation
			     && currentValue != u32char::Space) {
				_beginPos = ++it;
				break;
			}
		}
		// Search forward
		for (Iterator it = position(_startPos);
		     (bool)it == true;
		     ++it) {
			currentValue = *it;
			if (    currentValue != u32char::Tabulation
			     && currentValue != u32char::Space) {
				_endPos = it;
				break;
			}
		}
		return true;
	} else if(    u32char::isSpecialChar(currentValue) == false
	           || currentValue == '_') {
		APPL_DEBUG("select normal Char");
		// Search back
		for (Iterator it = --position(_startPos);
		     (bool)it == true;
		     --it) {
			currentValue = *it;
			if (    currentValue != '_'
			     && u32char::isSpecialChar(currentValue) == true) {
				_beginPos = ++it;
				break;
			}
		}
		// Search forward
		for (Iterator it = position(_startPos);
		     (bool)it == true;
		     ++it) {
			currentValue = *it;
			if (    currentValue != '_'
			     && u32char::isSpecialChar(currentValue) == true) {
				_endPos = it;
				break;
			}
		}
		return true;
	} else {
		APPL_DEBUG("select same char");
		char32_t comparechar = currentValue;
		// Search back
		for (Iterator it = --position(_startPos);
		     (bool)it == true;
		     --it) {
			currentValue = *it;
			if (comparechar != currentValue) {
				_beginPos = ++it;
				break;
			}
		}
		// Search forward
		for (Iterator it = position(_startPos);
		     (bool)it == true;
		     ++it) {
			currentValue = *it;
			if (comparechar != currentValue) {
				_endPos = it;
				break;
			}
		}
		return true;
	}
	_beginPos = begin();
	_endPos = begin();
	return false;
}

void appl::Buffer::setSelectionPos(const appl::Buffer::Iterator& _pos) {
	m_cursorSelectPos = _pos;
	signalSelectChange.emit();
}

void appl::Buffer::unSelect() {
	m_cursorSelectPos = -1;
	signalSelectChange.emit();
}

static const char *ControlCodeTable[32] = {
	 "NUL", "soh", "stx", "etx", "eot", "enq", "ack", "bel", "bs",  "ht", "nl",  "vt",  "np", "cr", "so", "si",
	 "dle", "dc1", "dc2", "dc3", "dc4", "nak", "syn", "etb", "can", "em", "sub", "esc", "fs", "gs", "rs", "us"};

void appl::Buffer::expand(int32_t& _indent, const char32_t& _value, etk::UString& _out) const {
	_out.clear();
	int32_t tabDist = 4;
	if (_value == u32char::Tabulation) {
		int32_t nSpaces = tabDist - (_indent % tabDist);
		for (int32_t iii=0; iii<nSpaces; iii++) {
			_out += u32char::Space;
		}
		return;
	}
	// convert ASCII control codes to readable character sequences
	if (_value == u32char::Null) {
		_out += U"<nul>";
		return;
	}
	if (_value == u32char::Return) {
		// nothing to display...
		_out += u32char::Return;
		return;
	}
	if (_value <= 31) {
		_out += '<';
		const char * tmp = ControlCodeTable[_value];
		while (*tmp!='\0') {
			_out += *tmp;
			tmp++;
		}
		_out += '>';
		return;
	}
	if (_value == u32char::Delete) {
		_out += U"<del>";
		return;
	}
	// nothing to do ...
	_out += _value;
	//APPL_DEBUG("plop : " << _out);
}

appl::Buffer::Iterator appl::Buffer::countForwardNLines(const appl::Buffer::Iterator& _startPos, int32_t _nLines) {
	char32_t value;
	int32_t lineCount = 0;
	//APPL_INFO("startPos=" << startPos << " nLines=" << nLines);
	for (Iterator it = position(_startPos);
	     (bool)it == true;
	     ++it) {
		value = *it;
		if (value == u32char::Return) {
			lineCount++;
			if (lineCount == _nLines) {
				//APPL_INFO("    == > (1) at position=" << myPosIt.Position()+1 );
				return ++it;
			}
		}
	}
	//APPL_INFO("    == > (2) at position=" << myPosIt.Position() );
	return end();
}

appl::Buffer::Iterator appl::Buffer::countBackwardNLines(const appl::Buffer::Iterator& _startPos, int32_t _nLines) {
	//APPL_INFO("startPos=" << startPos << " nLines=" << nLines);
	char32_t value;
	int32_t lineCount = 0;
	for (Iterator it = --position(_startPos);
	     (bool)it == true;
	     --it) {
		value = *it;
		if (value == u32char::Return) {
			lineCount++;
			if (lineCount >= _nLines) {
				//APPL_INFO("    == > (1) at position=" << myPosIt.Position()+1 );
				return ++it;
			}
		}
	}
	//APPL_INFO("    == > (2) at position=0");
	return begin();
}



bool appl::Buffer::copy(etk::String& _data) {
	_data.clear();
	if (hasTextSelected() == true) {
		int32_t startPos = getStartSelectionPos();
		int32_t endPos = getStopSelectionPos();
		for (Iterator it = position(startPos);
		     it != position(endPos) &&
		     (bool)it == true;
		     ++it) {
			_data += *it;
		}
		return true;
	}
	return false;
}

void appl::Buffer::copy(etk::String& _data, const appl::Buffer::Iterator& _pos, const appl::Buffer::Iterator& _posEnd) {
	_data.clear();
	for (Iterator it = _pos;
	     it != _posEnd &&
	     (bool)it == true;
	     ++it) {
		_data += *it;
	}
}

bool appl::Buffer::write(const etk::String& _data, const appl::Buffer::Iterator& _pos) {
	int64_t position = (int64_t)_pos;
	if (position < 0){
		position = 0;
	}
	APPL_VERBOSE("write at pos: " << (int64_t)_pos << " ==> " << position << " data : " << _data);
	m_data.insert(position, (int8_t*)(_data.c_str()), _data.size());
	if (m_cursorPos < 0) {
		m_cursorPos = 0;
	}
	//APPL_VERBOSE("*******  Regenerate after write");
	regenerateHighLightAt(position, 0, _data.size());
	m_selectMode = false;
	moveCursor(position+_data.size());
	countNumberofLine(); // TODO : use more intelligent counter
	setModification(true);
	return true;
}

bool appl::Buffer::replace(const etk::String& _data, const appl::Buffer::Iterator& _pos, const appl::Buffer::Iterator& _posEnd) {
	int64_t position = (int64_t)_pos;
	if (position < 0){
		position = 0;
	}
	m_data.replace(position, (int64_t)_posEnd-(int64_t)_pos, (int8_t*)(_data.c_str()), _data.size());
	//APPL_VERBOSE("*******  Regenerate after replace  pos=" << position << "  size remove=" << (int64_t)_posEnd-(int64_t)_pos << "  size add=" << _data.size());
	regenerateHighLightAt(position, (int64_t)_posEnd-(int64_t)_pos, _data.size());
	m_selectMode = false;
	moveCursor(position+_data.size());
	countNumberofLine(); // TODO : use more intelligent counter
	setModification(true);
	return true;
}

void appl::Buffer::removeSelection() {
	if (hasTextSelected() == false) {
		return;
	}
	int64_t startPos = getStartSelectionPos();
	int64_t endPos = getStopSelectionPos();
	m_data.remove(startPos, endPos-startPos);
	//APPL_VERBOSE("*******  Regenerate after remove");
	regenerateHighLightAt(startPos, endPos-startPos, 0);
	m_selectMode = false;
	moveCursor(startPos);
	countNumberofLine(); // TODO : use more intelligent counter
	setModification(true);
}

void appl::Buffer::tryFindHighlightType() {
	etk::String type = appl::highlightManager::getTypeFile(m_fileName.getFileName());
	if (type.size() == 0) {
		return;
	}
	APPL_DEBUG("Find extention type: " << type);
	setHighlightType(type);
}

void appl::Buffer::setHighlightType(const etk::String& _type) {
	m_highlightType = "";
	cleanHighLight();
	m_highlight.reset();
	etk::String resourceName = appl::highlightManager::getFileWithTypeType(_type);
	if (resourceName == "") {
		return;
	}
	m_highlightType = _type;
	m_highlight = appl::Highlight::create(resourceName);
	//APPL_VERBOSE("*******  Regenerate after set type HL");
	generateHighLightAt(0, m_data.size());
}

void appl::Buffer::regenerateHighLightAt(int64_t _pos, int64_t _nbDeleted, int64_t _nbAdded) {
	// prevent ERROR...
	if (m_highlight == null) {
		return;
	}
	// prevent No data Call
	if (    _nbDeleted == 0
	     && _nbAdded == 0) {
		return;
	}
	// normal case
	APPL_VERBOSE("(_pos="<<_pos<<", _nbDeleted="<<_nbDeleted<<", _nbAdded=" << _nbAdded << "\");");
	int64_t posEnd = _pos + _nbDeleted;
	// search position of the old element to reparse IT...
	int64_t startId = -1;
	int64_t stopId = -1;
	// clean data if needed
	if (m_HLDataPass1.size() == 0) {
		// Parse the new element ...
		generateHighLightAt(0, m_data.size());
		return;
	}
	// find element previous
	findMainHighLightPosition(_pos, posEnd, startId, stopId, true);
	
	APPL_VERBOSE(" list old parse:");
	for (auto &elem : m_HLDataPass1) {
		APPL_VERBOSE("    " << elem.start << "=>" << elem.stop);
	}
	// Remove previous element to prevent many errors like parsing of // for example
	startId--;
	APPL_VERBOSE("Find startId=" << startId << " stopId=" << stopId << " list size=" << m_HLDataPass1.size());
	
	// remove deprecated element
	if (    startId <= -1
	     && stopId <= -1) {
		m_HLDataPass1.clear();
		APPL_VERBOSE("1 * clear");
	} else if (startId <= -1) {
		if (stopId == 0){
			m_HLDataPass1.erase(m_HLDataPass1.begin());
			APPL_VERBOSE("1 * Erase 0");
		} else {
			m_HLDataPass1.erase(m_HLDataPass1.begin(), m_HLDataPass1.begin()+stopId);
			APPL_VERBOSE("2 * Erase 0->" << stopId);
		}
	} else if (stopId <= -1) {
		APPL_VERBOSE("3 * Erase " << startId+1 << "-> end");
		m_HLDataPass1.erase(m_HLDataPass1.begin()+startId+1, m_HLDataPass1.end());
		stopId = -1;
	} else {
		int32_t currentSize = m_HLDataPass1.size();
		APPL_VERBOSE("4 * Erase " << startId+1 << "->" << stopId << " in " << currentSize << " elements" );
		m_HLDataPass1.erase(m_HLDataPass1.begin()+startId+1, m_HLDataPass1.begin()+stopId+1);
		if (stopId == currentSize-1) {
			stopId = -1;
		}
	}
	APPL_VERBOSE(" list afterRemove:");
	if (m_HLDataPass1.size() != 0) {
		for (auto &elem : m_HLDataPass1) {
			APPL_VERBOSE("    " << elem.start << "=>" << elem.stop);
		}
	} else {
		APPL_VERBOSE("    EMPTY");
	}
	// Check Range guard
	if (startId >= m_HLDataPass1.size()) {
		startId = int64_t(m_HLDataPass1.size())-1;
	}
	if (stopId >= m_HLDataPass1.size()) {
		stopId = -1;
	}
	// update position after the range position:
	int64_t elemStart;
	if (startId <= -1) {
		elemStart = 0;
	} else {
		elemStart = startId+1;
	}
	for (auto it(m_HLDataPass1.begin()+elemStart); it != m_HLDataPass1.end(); ++it) {
		//APPL_DEBUG("move element=" << i);
		it->start += _nbAdded - _nbDeleted;
		it->stop  += _nbAdded - _nbDeleted;
	}
	//Regenerate Element inside range
	if (    startId <= -1
	     && stopId <= -1) {
		APPL_VERBOSE("*******  Regenerate ALL");
		generateHighLightAt(0, m_data.size());
	} else if(startId <= -1) {
		APPL_VERBOSE("*******  Regenerate START");
		generateHighLightAt(0, m_HLDataPass1[0].start, 0);
	} else if(stopId <= -1) {
		APPL_VERBOSE("*******  Regenerate STOP");
		generateHighLightAt(m_HLDataPass1[m_HLDataPass1.size() -1].stop, m_data.size(), m_HLDataPass1.size());
	} else {
		APPL_VERBOSE("*******  Regenerate RANGE");
		generateHighLightAt(m_HLDataPass1[startId].stop, m_HLDataPass1[startId+1].start, startId+1);
	}
}

void appl::Buffer::findMainHighLightPosition(int64_t _startPos,
                                             int64_t _endPos,
                                             int64_t& _startId,
                                             int64_t& _stopId,
                                             bool _backPreviousNotEnded) {
	_startId = -1;
	_stopId = -1;
	/* rules to start stop:
		HighLight data ----
		remove area    ****
		Start pos      S
		End pos        E

	Some Case :
		-----------          ------------          -------------            ----------
			      S              ****              E                                  

		-----------          ------------          -------------            ----------
			      S                **********      E                                  

		-----------          ------------          -------------            ----------
			                            S   ****   E                                  

		-----------          ------------          -------------            ----------
			      S     *********                  E                                  

		-----------          ------------          -------------            ----------
			      S     *********************      E                                  

		-----------          ------------          -------------            ----------
			      S               ************************                  E         

		-----------          ------------          -------------            ----------
			      S     *****************          E                                  

		-----------          ------------          -------------            ----------
			      S          ***************       E                                  

		-----------          ------------       
			      S          ***************       E=-1

			                 ------------          -------------            ----------
			      S=-1      ***************        E                                  
	*/
	for (size_t iii = 0; iii < m_HLDataPass1.size(); ++iii) {
		if (m_HLDataPass1[iii].stop > _startPos) {
			break;
		}
		_startId = iii;
	}
	// go back while the previous element is not eneded
	if (_backPreviousNotEnded == true) {
		for (int64_t iii = _startId; iii >= 0; --iii) {
			if (m_HLDataPass1[iii].notEnded == false) {
				break;
			}
			_startId = iii-1;
		}
	}
	int64_t elemStart;
	if(_startId == -1) {
		elemStart = 0;
	} else {
		elemStart = _startId+1;
	}
	for (size_t iii = elemStart; iii < m_HLDataPass1.size(); ++iii) {
		if (m_HLDataPass1[iii].start > _endPos) {
			_stopId = iii;
			break;
		}
	}
}

void appl::Buffer::generateHighLightAt(int64_t _pos, int64_t _endPos, int64_t _addingPos) {
	if (m_highlight == null) {
		return;
	}
	//APPL_DEBUG("area : (" << _pos << "," << _endPos << ") insert at : " << _addingPos);
	m_highlight->parse(_pos, _endPos, m_HLDataPass1, _addingPos, m_data);
}

void appl::Buffer::cleanHighLight() {
	// remove all element in the list...
	m_HLDataPass1.clear();
}


appl::HighlightInfo* appl::Buffer::getElementColorAtPosition(int64_t _pos, int64_t &_starPos) {
	int32_t start = etk::max((int64_t)0, _starPos-1);
	for (size_t iii = start; iii < m_HLDataPass1.size(); ++iii) {
		_starPos = iii;
		if (    m_HLDataPass1[iii].start <= _pos
		     && m_HLDataPass1[iii].stop > _pos) {
			return &m_HLDataPass1[iii];
		}
		if(m_HLDataPass1[iii].start > _pos) {
			return null;
		}
	}
	return null;
}


void appl::Buffer::hightlightGenerateLines(appl::DisplayHLData& _MData, const appl::Buffer::Iterator& _HLStart, int64_t _nbLines) {
	_MData.posHLPass1 = 0;
	_MData.posHLPass2 = 0;
	if (m_highlight == null) {
		return;
	}
	//int64_t timeStart = ewol::getTime();
	
	appl::Buffer::Iterator HLStartLine = getStartLine(_HLStart);
	int64_t HLStartPos = (int64_t)HLStartLine;
	_MData.HLData.clear();
	int64_t HLStop = (int64_t)countForwardNLines(HLStartLine, _nbLines);
	int64_t startId = 0;
	int64_t stopId = 0;
	// find element previous
	findMainHighLightPosition(_HLStart, HLStop, startId, stopId, true);

	//APPL_DEBUG("List of section between : "<< startId << " & " << stopId << "       pass1 store size=" << m_HLDataPass1.size());
	int64_t endSearch = stopId+1;
	if (stopId == -1) {
		endSearch = m_HLDataPass1.size();
	}
	int64_t kkk;
	for (kkk = etk::max(startId, (int64_t)0); kkk < endSearch; ++kkk) {
		// empty section :
		if (kkk == 0) {
			if (HLStartPos < m_HLDataPass1[kkk].start) {
				APPL_VERBOSE("   == > (empty section 1 ) kkk=" << kkk <<
				             " start=" << HLStartPos <<
				             " stop=" << m_HLDataPass1[kkk].start );
				m_highlight->parse2(HLStartPos,
				                    m_HLDataPass1[kkk].start,
				                    _MData.HLData,
				                    m_data);
			} // else : nothing to do ...
		} else {
			APPL_VERBOSE("   == > (empty section 2 ) kkk=" << kkk <<
			             " start=" << m_HLDataPass1[kkk-1].stop <<
			             " stop=" << m_HLDataPass1[kkk].start );
			m_highlight->parse2(m_HLDataPass1[kkk-1].stop,
			                    m_HLDataPass1[kkk].start,
			                    _MData.HLData,
			                    m_data);
		}
		// under section :
		APPL_VERBOSE("   == > (under section   ) kkk="<<kkk<<" start="<<m_HLDataPass1[kkk].start<<" stop="<<m_HLDataPass1[kkk].stop << " subSectionOfID=" << 99999999);
		m_highlight->parseSubElement(m_HLDataPass1[kkk],
		                             _MData.HLData,
		                             m_data);
	}
	if (endSearch == (int32_t)m_HLDataPass1.size() ){
		//if(		k < (int32_t)m_HLDataPass1.size()) {
		if (m_HLDataPass1.size() != 0) {
			APPL_VERBOSE("   == > (empty section 3 ) kkk=" << kkk <<
			             " start=" << m_HLDataPass1[kkk-1].stop <<
			             " stop=" << HLStop );
			m_highlight->parse2(m_HLDataPass1[kkk-1].stop,
			                    HLStop,
			                    _MData.HLData,
			                    m_data);
		} else {
			APPL_VERBOSE("   == > (empty section 4 ) kkk=" << kkk <<
			             " start=0 stop=" << HLStop );
			m_highlight->parse2(0,
			                    HLStop,
			                    _MData.HLData,
			                    m_data);
		}
	}
	/*
	int64_t timeStop = ewol::getTime();
	APPL_DEBUG("Display 2nd pass = " << (timeStop-timeStart)/1000.0f << " milli-second");
	*/
}


appl::HighlightInfo* appl::Buffer::getElementColorAtPosition(appl::DisplayHLData& _MData, int64_t _pos) {
	int64_t start = etk::max((int64_t)0, _MData.posHLPass2-1);
	for (int64_t iii=start; iii<(int32_t)_MData.HLData.size(); iii++) {
		_MData.posHLPass2 = iii;
		if(		_MData.HLData[iii].start <= _pos
			&&	_MData.HLData[iii].stop    > _pos)
		{
			return &_MData.HLData[iii];
		}
		if(_MData.HLData[iii].start > _pos) {
			return getElementColorAtPosition(_pos, _MData.posHLPass1);
		}
	}
	return getElementColorAtPosition(_pos, _MData.posHLPass1);
}

uint32_t appl::Buffer::getCursorLinesId() {
	if (m_data.size() == 0) {
		return 0;
	}
	uint32_t line = 0;
	for (Iterator it = begin();
	    (bool)it == true && it <= cursor();
	     ++it) {
		if (*it == u32char::Return) {
			++line;
		}
	}
	return line;
}

namespace etk {
	template<> etk::String toString<ememory::SharedPtr<appl::Buffer>>(const ememory::SharedPtr<appl::Buffer>& _obj) {
		if (_obj != null) {
			return _obj->getFileName().getString();
		}
		return "";
	}
	template<> etk::UString toUString<ememory::SharedPtr<appl::Buffer>>(const ememory::SharedPtr<appl::Buffer>& _obj) {
		return etk::toUString(etk::toString(_obj));
	}
	
	template<> bool from_string<ememory::SharedPtr<appl::Buffer>>(ememory::SharedPtr<appl::Buffer>& _variableRet, const etk::String& _value) {
		if (_variableRet != null) {
			_variableRet->loadFile(_value);
			return true;
		}
		return false;
	}
	template<> bool from_string<ememory::SharedPtr<appl::Buffer>>(ememory::SharedPtr<appl::Buffer>& _variableRet, const etk::UString& _value) {
		return from_string(_variableRet, etk::toString(_value));
	}
	template<> etk::String toString<appl::Buffer>(const appl::Buffer& _obj) {
		return _obj.getFileName().getString();
	}
	template<> etk::UString toUString<appl::Buffer>(const appl::Buffer& _obj) {
		return etk::toUString(etk::toString(_obj));
	}
	
	template<> bool from_string<appl::Buffer>(appl::Buffer& _variableRet, const etk::String& _value) {
		_variableRet.loadFile(_value);
		return true;
	}
	template<> bool from_string<appl::Buffer>(appl::Buffer& _variableRet, const etk::UString& _value) {
		return from_string(_variableRet, etk::toString(_value));
	}
};

#include <esignal/details/Signal.hxx>
// declare for signal event
ESIGNAL_DECLARE_SIGNAL(ememory::SharedPtr<appl::Buffer>);


