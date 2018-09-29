/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#include <appl/debug.hpp>
#include <appl/global.hpp>
#include <appl/GlyphPainting.hpp>
#include <ejson/ejson.hpp>
#include <gale/resource/Manager.hpp>


appl::GlyphPainting::GlyphPainting() {
	addResourceType("appl::GlyphPainting");
}

void appl::GlyphPainting::init(const etk::Uri& _uri) {
	gale::Resource::init(_uri);
	APPL_DEBUG("SFP : load \"" << _uri << "\"");
	reload();
}

appl::GlyphPainting::~GlyphPainting() {
	
}

void appl::GlyphPainting::reload() {
	ejson::Document doc;
	if (doc.load(etk::Uri(m_name)) == false) {
		APPL_ERROR("Can not load file : '" << m_name << "' = " << etk::Uri(m_name));
		return;
	}
	// for debug only :
	/*
	APPL_WARNING("Load file : '" << m_name << "' = " << etk::FSNode(m_name).getFileSystemName());
	etk::String tmppppp;
	doc.generate(tmppppp);
	APPL_DEBUG(tmppppp);
	*/
	ejson::Array baseArray = doc["ednColor"].toArray();
	if (baseArray.exist() == false) {
		APPL_ERROR("Can not get basic array : 'ednColor'");
		return;
	}
	for (const auto it : baseArray) {
		ejson::Object tmpObj = it.toObject();
		if (tmpObj.exist() == false) {
			APPL_DEBUG(" can not get object in 'ednColor' it=" << it);
			continue;
		}
		etk::String name = tmpObj["name"].toString().get();
		etk::String background = tmpObj["background"].toString().get("#FFF0");
		etk::String foreground = tmpObj["foreground"].toString().get("#000F");
		bool italic = tmpObj["italic"].toBoolean().get(false);
		bool bold = tmpObj["bold"].toBoolean().get(false);
		APPL_VERBOSE("find new color : '" << name << "' fg='" << foreground << "' bg='" << background << "' italic='" << italic << "' bold='" << bold << "'");
		bool findElement = false;
		for (size_t jjj=0; jjj<m_list.size(); ++jjj) {
			if (m_list[jjj].getName() != name) {
				continue;
			}
			m_list[jjj].setForeground(foreground);
			m_list[jjj].setBackground(background);
			m_list[jjj].setItalic(italic);
			m_list[jjj].setBold(bold);
			findElement = true;
		}
		if (findElement == true) {
			continue;
		}
		appl::GlyphDecoration tmpDeco(name);
		tmpDeco.setForeground(foreground);
		tmpDeco.setBackground(background);
		tmpDeco.setItalic(italic);
		tmpDeco.setBold(bold);
		m_list.pushBack(tmpDeco);
	}
}


int32_t appl::GlyphPainting::request(const etk::String& _name) {
	for (size_t iii=0; iii<m_list.size(); ++iii) {
		if (m_list[iii].getName() == _name) {
			return iii;
		}
	}
	// create an empty deco ...
	appl::GlyphDecoration tmpDeco(_name);
	m_list.pushBack(tmpDeco);
	return m_list.size()-1;
}
