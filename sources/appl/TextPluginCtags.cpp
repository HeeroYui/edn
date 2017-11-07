/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#include <appl/TextPluginCtags.hpp>
#include <gale/context/clipBoard.hpp>
#include <appl/Gui/TextViewer.hpp>
#include <ewol/widget/meta/FileChooser.hpp>
#include <ewol/context/Context.hpp>
#include <appl/Gui/TagFileSelection.hpp>

static etk::String g_staticCtagsFileName;

void appl::setCtagsFileName(const etk::String& _file) {
	g_staticCtagsFileName = _file;
}

appl::TextPluginCtags::TextPluginCtags() :
  m_tagFolderBase(""),
  m_tagFilename(""),
  m_ctagFile(nullptr) {
	m_activateOnReceiveShortCut = true;
	// load buffer manager:
	m_bufferManager = appl::BufferManager::create();
	addObjectType("appl::TextPluginCtags");
	if (g_staticCtagsFileName != "") {
		m_tagFilename = g_staticCtagsFileName;
		loadTagFile();
	}
}

appl::TextPluginCtags::~TextPluginCtags() {
	
}

void appl::TextPluginCtags::onPluginEnable(appl::TextViewer& _textDrawer) {
	_textDrawer.ext_shortCutAdd("ctrl+d", "appl::TextPluginCtags::JumpDestination");
	_textDrawer.ext_shortCutAdd("ctrl+shift+d", "appl::TextPluginCtags::JumpBack");
	_textDrawer.ext_shortCutAdd("ctrl+alt+d", "appl::TextPluginCtags::OpenCtagsFile");
}

void appl::TextPluginCtags::onPluginDisable(appl::TextViewer& _textDrawer) {
	_textDrawer.ext_shortCutRm("appl::TextPluginCtags::JumpDestination");
	_textDrawer.ext_shortCutRm("appl::TextPluginCtags::JumpBack");
	_textDrawer.ext_shortCutRm("appl::TextPluginCtags::OpenCtagsFile");
}

void appl::TextPluginCtags::jumpTo(const etk::String& _name) {
	if (m_ctagFile == nullptr) {
		APPL_WARNING("No ctags file open");
		return;
	}
	tagEntry entry;
	APPL_INFO("try to find the tag : " << _name);
	if (tagsFind (m_ctagFile, &entry, _name.c_str(), 0) != TagSuccess) {
		APPL_INFO("no tag find ...");
		return;
	}
	tagEntry entrySave = entry;
	int32_t numberOfTags = 0;
	
	// For all tags : Save in an internal Structure :
	etk::String tmpFile(m_tagFolderBase + "/" + entry.file);
	etk::FSNode myfile(tmpFile);
	int32_t lineID = entry.address.lineNumber;
	printTag(&entry);
	
	if (tagsFindNext (m_ctagFile, &entry) == TagSuccess) {
		APPL_INFO("Multiple file destination ...");
		ememory::SharedPtr<appl::TagFileSelection> tmpWidget = appl::TagFileSelection::create();
		if (tmpWidget == nullptr) {
			APPL_ERROR("Can not allocate widget  == > display might be in error");
		} else {
			tmpWidget->addCtagsNewItem(myfile.getFileSystemName(), lineID);
			do {
				tmpFile = m_tagFolderBase + "/" + entry.file;
				myfile = tmpFile;
				lineID = entry.address.lineNumber;
				printTag(&entry);
				tmpWidget->addCtagsNewItem(myfile.getFileSystemName(), lineID);
			} while (tagsFindNext (m_ctagFile, &entry) == TagSuccess);
			ewol::getContext().getWindows()->popUpWidgetPush(tmpWidget);
			tmpWidget->signalSelect.connect(sharedFromThis(), &appl::TextPluginCtags::onCallbackOpenCtagsSelectReturn);
		}
	} else {
		jumpFile(myfile.getName(), lineID - 1);
	}
}

void appl::TextPluginCtags::jumpFile(const etk::String& _filename, int64_t _lineId) {
	// save the current file in the history
	// TODO : registerHistory();
	if (m_bufferManager != nullptr) {
		etk::String plop = _filename;
		while (plop[0] == '/') {
			plop.erase(0);
		}
		m_bufferManager->open(plop);
	}
	//sendMultiCast(appl::MsgSelectGotoLineSelect, etk::toString(_lineId));
	APPL_TODO("request jup at line ...");
}

void appl::TextPluginCtags::loadTagFile() {
	tagFileInfo info;
	// close previous tag file
	if (nullptr != m_ctagFile) {
		tagsClose(m_ctagFile);
		m_ctagFile = nullptr;
	}
	if (m_tagFilename == "") {
		return;
	}
	// load (open) the tag file : 
	APPL_INFO("try to open tag file : " << m_tagFilename);
	m_ctagFile = tagsOpen(m_tagFilename.c_str(), &info);
	if (nullptr != m_ctagFile) {
		APPL_INFO("open exuberant Ctags file is OK ...");
	} else {
		APPL_INFO("Error to open ctags file ...");
	}
}

void appl::TextPluginCtags::printTag(const tagEntry *_entry) {
	#if 1
		APPL_INFO("find Tag file : name=\"" << _entry->name << "\" in file=\"" << _entry->file
			<< "\" at line="<< (int32_t)_entry->address.lineNumber);
	#else
		APPL_INFO("find Tag file : name=\"" << _entry->name << "\" in file=\"" << _entry->file
			<< "\" pattern=\"" << _entry->address.pattern 
			<< "\" at line="<< (int32_t)_entry->address.lineNumber);
		
		APPL_INFO("Extention field : ");
		if (_entry->kind != nullptr  && _entry->kind [0] != '\0') {
			APPL_INFO("        kind : " << _entry->kind);
		}
		if (_entry->fileScope) {
			APPL_INFO("        file : ");
		}
		for (int32_t iii = 0 ; iii < _entry->fields.count ; ++iii) {
			APPL_INFO("               " << _entry->fields.list[iii].key << ":" << _entry->fields.list[iii].value );
		}
	#endif
}

void appl::TextPluginCtags::onCallbackOpenCtagsOpenFileReturn(const etk::String& _value) {
	// open the new one :
	etk::FSNode tmpFilename = _value;
	m_tagFilename = tmpFilename.getNameFile();
	m_tagFolderBase = tmpFilename.getNameFolder();
	APPL_INFO("Receive load Ctags file : " << m_tagFolderBase << "/" << m_tagFilename << " ");
	loadTagFile();
}

void appl::TextPluginCtags::onCallbackOpenCtagsSelectReturn(const etk::String& _value) {
	// parse the input data
	char tmp[4096];
	int32_t lineID;
	// TODO : Review this ...
	sscanf(_value.c_str(), "%d:%s", &lineID, tmp);
	jumpFile(tmp, lineID - 1);
}

bool appl::TextPluginCtags::onReceiveShortCut(appl::TextViewer& _textDrawer,
                                              const etk::String& _shortCutName) {
	if (isEnable() == false) {
		return false;
	}
	if (_shortCutName == "appl::TextPluginCtags::OpenCtagsFile") {
		APPL_INFO("Request opening ctag file");
		ememory::SharedPtr<ewol::widget::FileChooser> tmpWidget = ewol::widget::FileChooser::create();
		if (tmpWidget == nullptr) {
			APPL_ERROR("Can not allocate widget  == > display might be in error");
			return true;
		}
		tmpWidget->propertyLabelTitle.set("Open Exuberant Ctags file");
		tmpWidget->propertyLabelValidate.set("Open");
		// try to get the current folder :
		etk::String path = _textDrawer.getBufferPath();
		APPL_ERROR("get path : '" << path << "'");
		if (path != "") {
			tmpWidget->propertyPath.set(path);
		}
		ewol::getContext().getWindows()->popUpWidgetPush(tmpWidget);
		tmpWidget->signalValidate.connect(sharedFromThis(), &appl::TextPluginCtags::onCallbackOpenCtagsOpenFileReturn);
		return true;
	} else if (_shortCutName == "appl::TextPluginCtags::JumpDestination") {
		if (_textDrawer.hasBuffer() == false) {
			return false;
		}
		etk::String textToSearch;
		if (_textDrawer.hasTextSelected() == true) {
			_textDrawer.copy(textToSearch, _textDrawer.selectStart(), _textDrawer.selectStop() );
		} else {
			appl::Buffer::Iterator _beginPos;
			appl::Buffer::Iterator _endPos;
			if (_textDrawer.getPosAround(_textDrawer.cursor(), _beginPos, _endPos) == false) {
				APPL_WARNING("Can not get data around...");
				return true;
			}
			_textDrawer.copy(textToSearch, _beginPos, _endPos);
		}
		jumpTo(textToSearch);
		return true;
	} else if (_shortCutName == "appl::TextPluginCtags::JumpBack") {
		if (_textDrawer.hasBuffer() == false) {
			return false;
		}
		return true;
	}
	return false;
}
