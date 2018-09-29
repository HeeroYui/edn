/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#include <appl/debug.hpp>
#include <appl/global.hpp>
#include <appl/HighlightManager.hpp>
#include <ewol/object/Object.hpp>
#include <ewol/object/Manager.hpp>
#include <etk/path/fileSystem.hpp>


// TODO : Review this in a generic unique resource ...

static etk::Vector<ememory::SharedPtr<appl::Highlight>>& s_list() {
	static etk::Vector<ememory::SharedPtr<appl::Highlight>> list;
	return list;
}


void appl::highlightManager::init() {
	etk::Vector<ememory::SharedPtr<appl::Highlight>>& hlList = s_list();
	if (hlList.size() != 0) {
		APPL_ERROR("HighlightManager  == > already exist, just unlink the previous ...");
		hlList.clear();
	}
	APPL_DEBUG("HighlightManager  == > INIT");
	// TODO: Add search in the  etk::Uri uri("DATA_USER://languages/");
	etk::Uri uri("DATA://languages/");
	// get the subfolder list:
	etk::Vector<etk::Uri> list = etk::uri::listRecursive(uri); // TODO: filter only the folder
	for (auto &it: list) {
		// TODO: etk::uri::isDirectory(it) ==> not implemented ...
		if (etk::path::isDirectory(it.getPath()) == false) {
			continue;
		}
		ememory::SharedPtr<appl::Highlight> myHightLight = appl::Highlight::create(it.getPath() / "highlight.xml");
		if (myHightLight != null) {
			// Check if the language name already exist
			for (auto &it2 : hlList) {
				if (    it2 != null
				     && it2->getTypeName() == myHightLight->getTypeName() ) {
					APPL_WARNING("LANGUAGE : replace pattern name: '" << myHightLight->getTypeName() << "' with file '" << it.getPath() / "highlight.xml" << "' replace: " << it2->getName());
				}
			}
			hlList.pushBack(myHightLight);
		} else {
			APPL_ERROR("Can not allocate HighLight");
		}
	}
	// display :
	for (auto &it : hlList) {
		if (it == null) {
			continue;
		}
		it->display();
	}
}

void appl::highlightManager::unInit() {
	etk::Vector<ememory::SharedPtr<Highlight>>& hlList = s_list();
	if (hlList.size() == 0) {
		APPL_DEBUG("HighlightManager  ==> no highlight");
		hlList.clear();
		return;
	}
	hlList.clear();
}

etk::String appl::highlightManager::getTypeFile(const etk::Path& _fileName) {
	if (_fileName.isEmpty() == true) {
		return "";
	}
	APPL_WARNING("Try to find type for extention : '" << _fileName << "' in " << s_list().size() << " types");
	etk::Vector<ememory::SharedPtr<Highlight>>& hlList = s_list();
	for (auto &it : hlList) {
		if (it == null) {
			continue;
		}
		APPL_WARNING("    check : " << it->getTypeName());
		if (it->isCompatible(_fileName) == true) {
			APPL_WARNING("Find type for extention : " << _fileName << " type : " << it->getTypeName());
			return it->getTypeName();
		}
	}
	return "";
}

etk::String appl::highlightManager::getFileWithTypeType(const etk::String& _type) {
	if (_type.size() == 0) {
		return "";
	}
	for (auto &it : s_list()) {
		if (it == null) {
			continue;
		}
		if (it->getTypeName() == _type) {
			return it->getName();
		}
	}
	return "";
}

etk::Vector<etk::String> appl::highlightManager::getTypeList() {
	etk::Vector<etk::String> ret;
	for (auto &it : s_list()) {
		if (it == null) {
			continue;
		}
		ret.pushBack(it->getTypeName());
	}
	return ret;
}


