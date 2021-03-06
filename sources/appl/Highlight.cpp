/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#include <appl/debug.h>
#include <appl/global.h>
#include <appl/Highlight.h>
#include <exml/exml.h>
#include <ewol/ewol.h>
#include <gale/resource/Manager.h>

// first pass
//#define HL_DEBUG APPL_INFO
#define HL_DEBUG APPL_VERBOSE

// second pass
//#define HL2_DEBUG APPL_INFO
#define HL2_DEBUG APPL_VERBOSE

appl::Highlight::Highlight() {
	addResourceType("appl::Highlight");
}

void appl::Highlight::init(const std::string& _xmlFilename, const std::string& _colorFile) {
	gale::Resource::init(_xmlFilename);
	
	// keep color propertiy file :
	m_paintingProperties = appl::GlyphPainting::create(_colorFile);
	
	exml::Document doc;
	if (doc.load(_xmlFilename) == false) {
		APPL_ERROR(" can not load file XML : " << _xmlFilename);
		return;
	}
	exml::Element root = doc.nodes["EdnLang"];
	if (root.exist() == false) {
		APPL_ERROR("(l ?) main node not find: 'EdnLang' ...");
		return;
	}
	m_typeName = root.attributes["lang"];
	int32_t level1 = 0;
	int32_t level2 = 0;
	// parse all the elements :
	for (const auto it : root.nodes) {
		const exml::Element child = it.toElement();
		if (child.exist() == false) {
			// trash here all that is not element ...
			continue;
		}
		if (child.getValue() == "ext") {
			std::string myData = child.getText();
			if (myData.size()!=0) {
				//HL_DEBUG("(l %d) node fined : %s=\"%s\"", child->Row(), child->Value() , myData);
				m_listExtentions.push_back(myData);
			}
		} else if (child.getValue() == "pass1") {
			// get sub Nodes ...
			for (const auto it2 : child.nodes) {
				const exml::Element passChild = it2.toElement();
				if (passChild.exist() == false) {
					continue;
				}
				if (passChild.getValue() != "rule") {
					APPL_ERROR("(l "<< passChild.getPos() << ") node not suported : '"<< passChild.getValue() << "' must be [rule]" );
					continue;
				}
				// Create the patern in list
				m_listHighlightPass1.push_back(HighlightPattern(m_paintingProperties, passChild, level1++));
			}
		} else if (child.getValue() == "pass2") {
			// get sub Nodes ...
			for (const auto it2 : child.nodes) {
				const exml::Element passChild = it2.toElement();
				if (passChild.exist() == false) {
					continue;
				}
				if (passChild.getValue() != "rule") {
					APPL_ERROR("(l "<< passChild.getPos() << ") node not suported : '"<< passChild.getValue() << "' must be [rule]" );
					continue;
				}
				// Create the patern in list
				m_listHighlightPass2.push_back(HighlightPattern(m_paintingProperties, passChild, level2++));
			}
		} else if (child.getValue() == "pass") {
			std::string attributeName = child.attributes["name"];
			if (attributeName == "") {
				APPL_ERROR("Can not parse an element pass with no attribute name ... ligne=" << child.getPos());
				continue;
			}
			m_listHighlightNamed.insert(std::pair<std::string, std::vector<HighlightPattern>>(attributeName, std::vector<HighlightPattern>()));
			auto it3 = m_listHighlightNamed.find(attributeName);
			int32_t level3=0;
			// get sub Nodes ...
			for (const auto it2 : child.nodes) {
				const exml::Element passChild = it2.toElement();
				if (passChild.exist() == false) {
					continue;
				}
				if (passChild.getValue() != "rule") {
					APPL_ERROR("(l "<< passChild.getPos() << ") node not suported : '"<< passChild.getValue() << "' must be [rule]" );
					continue;
				}
				// add element in the list
				it3->second.push_back(HighlightPattern(m_paintingProperties, passChild, level3++));
			}
		} else {
			APPL_ERROR("(l "<< child.getPos() << ") node not suported : '"<< child.getValue() << "' must be [ext,pass1,pass2]" );
		}
	}
}

appl::Highlight::~Highlight() {
	// clear the compleate list
	m_listHighlightPass1.clear();
	// clear the compleate list
	m_listHighlightPass2.clear();
	// clear the compleate list
	m_listExtentions.clear();
}

bool appl::Highlight::isCompatible(const std::string& _name) {
	for (auto &it : m_listExtentions) {
		APPL_DEBUG("        check : " << it << "=?=" << _name);
		std::regex expression;
		try {
			expression.assign(it, std::regex_constants::optimize | std::regex_constants::ECMAScript);
		} catch (std::regex_error e) {
			APPL_ERROR("can not parse regex : '" << e.what() << "' for : " << it);
			continue;
		}
		std::smatch resultMatch;
		std::regex_search(_name.begin(), _name.end(), resultMatch, expression, std::regex_constants::match_continuous);
		if (resultMatch.size() <= 0) {
			continue;
		}
		APPL_VERBOSE("    - begin=" << std::distance(_name.begin(), resultMatch[0].first) << "  end=" << std::distance(_name.begin(), resultMatch[0].second));
		if (resultMatch[0].first != _name.begin()) {
			continue;
		}
		if (resultMatch[0].second != _name.end()) {
			continue;
		}
		return true;
	}
	return false;
}

bool appl::Highlight::fileNameCompatible(const std::string& _fileName) {
	std::string extention;
	etk::FSNode file(_fileName);
	if (true == file.fileHasExtention() ) {
		extention = "*.";
		extention += file.fileGetExtention();
	} else {
		extention = file.getNameFile();
	}
	APPL_DEBUG(" try to find : in \"" << file << "\" extention:\"" << extention << "\" ");

	for (auto &it : m_listExtentions) {
		if (extention == it ) {
			return true;
		}
	}
	return false;
}


void appl::Highlight::display() {
	APPL_INFO("List of ALL Highlight : ");
	for (auto &it : m_listExtentions) {
		APPL_INFO("        Extention : " << it );
	}
	// display all elements
	for (auto &it : m_listHighlightPass1) {
		APPL_INFO("        Pass 1 : " << it.getName() );
		//it.display();
	}
	for (auto &it : m_listHighlightPass2) {
		APPL_INFO("        pass 2 : " << it.getName() );
		//it.display();
	}
	for (auto &it : m_listHighlightNamed) {
		APPL_INFO("        pass * : " << it.first << " : ");
		for (auto &it2 : it.second) {
			APPL_INFO("              " << it2.getName() );
			//it.display();
		}
		//it.display();
	}
}

/* TODO : Celui qui appelle suprime des element pour rien ... Enfin c'est pas très grave... 
 * Il suffirait juste de suprimer celui d'avant si il n'est pas terminer...
 */
void appl::Highlight::parse(int64_t _start,
                            int64_t _stop,
                            std::vector<appl::HighlightInfo> & _metaData,
                            int64_t _addingPos,
                            std::string& _buffer) {
	if (0 > _addingPos) {
		_addingPos = 0;
	}
	HL_DEBUG("Parse element 0 => " << m_listHighlightPass1.size() << "  == > position search: (" << _start << "," << _stop << ")" );
	int64_t elementStart = _start;
	int64_t elementStop = _stop;
	appl::HighlightInfo resultat;
	int64_t startTime = ewol::getTime();
	while (elementStart <= elementStop) {
		//HL_DEBUG("Parse element in the buffer pos=" << elementStart);
		int64_t currentTime = ewol::getTime();
		//try to fond the HL in ALL of we have
		for (int64_t jjj=0; jjj<(int64_t)m_listHighlightPass1.size(); jjj++){
			bool ret = true;
			/*
			if (_buffer[elementStart] == '\n') {
				HL_DEBUG("Parse HL id=" << jjj << " position search: (" << elementStart << "," << _stop << ") input start='\\n' " << m_listHighlightPass1[jjj].getPaternString());
			} else {
				HL_DEBUG("Parse HL id=" << jjj << " position search: (" << elementStart << "," << _stop << ") input start='" << _buffer[elementStart] << "' " << m_listHighlightPass1[jjj].getPaternString());
			}
			*/
			// Stop the search to the end (to get the end of the pattern)
			ret = m_listHighlightPass1[jjj].find(elementStart, _buffer.size(), resultat, _buffer);
			if (ret == true) {
				int64_t currentTimeEnd = ewol::getTime();
				int64_t deltaTime = currentTimeEnd - currentTime;
				HL_DEBUG("Find Pattern in the Buffer : time=" << (float)deltaTime/1000.0f << " ms (" << resultat.start << "," << resultat.stop << ") startPos=" << elementStart << " for=" << m_listHighlightPass1[jjj].getPaternString().first << " " << m_listHighlightPass1[jjj].getPaternString().second);
				// remove element in the current List where the current Element have a end inside the next...
				int64_t kkk=_addingPos;
				while(kkk < (int64_t)_metaData.size() ) {
					if (_metaData[kkk].start <= resultat.stop) {
						// remove element
						HL_DEBUG("Erase element=" << kkk);
						_metaData.erase(_metaData.begin()+kkk, _metaData.begin()+kkk+1);
						// Increase the end of search
						if (kkk < (int64_t)_metaData.size()) {
							// just before the end of the next element
							elementStop = _metaData[kkk].start-1;
						} else {
							// end of the buffer
							elementStop = _buffer.size();
						}
					} else {
						// Not find  == > exit the cycle : 
						break;
					}
				}
				// add curent element in the list ...
				_metaData.insert(_metaData.begin()+_addingPos, resultat);
				HL_DEBUG("INSERT at "<< _addingPos << " S=" << resultat.start << " E=" << resultat.stop );
				// update the current research starting element: (set position at the end of the current element
				elementStart = resultat.stop-1;
				// increment the position of insertion:
				_addingPos++;
				// We find a pattern  == > Stop search for the current element
				break;
			}
		}
		// Go to the next element (and search again ...).
		elementStart++;
	}
	int64_t stopTime = ewol::getTime();
	int64_t deltaTimeGlobal = stopTime - startTime;
	APPL_DEBUG("parse in time=" << (float)deltaTimeGlobal/1000.0f << " ms ");
	
	
}


/**
 * @brief second pass of the hightlight
 * @param[in] _start Start searching data
 * @param[in] _stop End searching data
 * @param[out] _metaData Output list of all find patern
 * @param[in] _buffer buffer where we need to search data
 */
void appl::Highlight::parse2(int64_t _start,
                             int64_t _stop,
                             std::vector<appl::HighlightInfo>& _metaData,
                             std::string& _buffer) {
	HL2_DEBUG("Parse element 0 => " << m_listHighlightPass2.size() <<
	          "  == > position search: (" << _start << "," << _stop << ")" );
	int64_t elementStart = _start;
	int64_t elementStop = _stop;
	appl::HighlightInfo resultat;
	
	while (elementStart < elementStop) {
		if (elementStart == 306) {
			//elog::setLevel(elog::logLevelVerbose);
		}
		//HL2_DEBUG("Parse element in the buffer pos=" << elementStart << "," << _buffer.size() << ")" );
		//try to fond the HL in ALL of we have
		for (int64_t jjj=0; jjj<int64_t(m_listHighlightPass2.size()); jjj++){
			HL2_DEBUG("Parse HL id=" << jjj << " position search: (" <<
			         elementStart << "," << elementStop << ") in='"
			         << /*_buffer[elementStart]*/ std::string(_buffer.begin()+elementStart,_buffer.begin()+elementStop) << "' " << m_listHighlightPass2[jjj].getPaternString().first << " " << m_listHighlightPass1[jjj].getPaternString().second);
			// Stop the search to the end (to get the end of the pattern)
			bool ret = m_listHighlightPass2[jjj].find(elementStart, elementStop, resultat, _buffer);
			if (ret == true) {
				// find an element:
				_metaData.push_back(resultat);
				HL2_DEBUG("data='" << std::string(_buffer.begin()+elementStart,_buffer.begin()+resultat.stop) << "'");
				elementStart = resultat.stop-1;
				break;
			}
		}
		// Go to the next element (and search again ...).
		elementStart++;
	}
}

/**
 * @brief second pass of the hightlight pattern (have found something before)
 * @param[in] _upper upper pattern to find the data
 * @param[out] _metaData Output list of all find patern
 * @param[in] _buffer buffer where we need to search data
 */
void appl::Highlight::parseSubElement(const appl::HighlightInfo& _upper,
                                      std::vector<appl::HighlightInfo> &_metaData,
                                      std::string &_buffer) {
	if (_upper.patern->getSubPatternName().size() == 0) {
		return;
	}
	HL2_DEBUG("Parse element 0 => " << m_listHighlightNamed.size() <<
	          "  == > position search: (" << _upper.start << "," << _upper.stop << ")" );
	int64_t elementStart = _upper.start;
	int64_t elementStop = _upper.stop;
	appl::HighlightInfo resultat;
	// Find element in the list:
	auto itHL = m_listHighlightNamed.find(_upper.patern->getSubPatternName());
	if (itHL == m_listHighlightNamed.end()) {
		APPL_ERROR("Patern does not exist : " << _upper.patern->getSubPatternName() << " note : Removing it ...");
		_upper.patern->setSubPatternName("");
		return;
	}
	
	while (elementStart < elementStop) {
		//try to fond the HL in ALL of we have
		for (auto &it : itHL->second){
			HL2_DEBUG("Parse HL position search: (" << elementStart << "," << elementStop << ") in='" << _buffer[elementStart] << "' " << it.getPaternString().first << " " << it.getPaternString().second);
			// Stop the search to the end (to get the end of the pattern)
			bool ret = it.find(elementStart, elementStop, resultat, _buffer);
			if (ret == true) {
				_metaData.push_back(resultat);
				elementStart = resultat.stop-1;
				break;
			}
		}
		// Go to the next element (and search again ...).
		elementStart++;
	}
}
