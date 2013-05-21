/**
 * @author Edouard DUPIN
 * 
 * @copyright 2010, Edouard DUPIN, all right reserved
 * 
 * @license GPL v3 (see license file)
 */

#include <appl/Debug.h>
#include <appl/global.h>
#include <ColorizeManager.h>
#include <tinyXML/tinyxml.h>
#include <ewol/eObject/EObject.h>
#include <ewol/eObject/EObjectManager.h>
#include <etk/os/FSNode.h>

#define PFX	"ColorizeManager "

class classColorManager: public ewol::EObject
{
	public:
		// Constructeur
		classColorManager(void);
		~classColorManager(void);
		
		const char * const GetObjectType(void)
		{
			return "ApplColorManager";
		}
		virtual void OnReceiveMessage(const ewol::EMessage& _msg);
	public:
		void        LoadFile(etk::UString &xmlFilename);
		void        LoadFile(const char * xmlFilename);
		Colorize *  Get(const char *colorName);
		Colorize *  Get(etk::UString &colorName);
		draw::Color& Get(basicColor_te myColor);
		bool        Exist(etk::UString &colorName);
		bool        Exist(const char *colorName);
		void        DisplayListOfColor(void);

	private:
		etk::UString                m_fileColor;
		etk::Vector<Colorize*>  listMyColor;		//!< List of ALL Color
		Colorize *                  errorColor;
		draw::Color                  basicColors[COLOR_NUMBER_MAX];
};


classColorManager::classColorManager(void)
{
	//ewol::widgetMessageMultiCast::Add(GetWidgetId(), ednMsgGuiChangeColor);
}

classColorManager::~classColorManager(void)
{
	delete(errorColor);

	int32_t i;
	// clean all Element
	for (i=0; i< listMyColor.Size(); i++) {
		if (NULL != listMyColor[i]) {
			delete(listMyColor[i]);
			listMyColor[i] = NULL;
		}
	}
	// clear the compleate list
	listMyColor.Clear();
}

void classColorManager::OnReceiveMessage(const ewol::EMessage& _msg)
{
	/*
	switch (id)
	{
		case APPL_MSG__RELOAD_COLOR_FILE:
			{
				// Reaload File
				// TODO : Check this : Pb in the recopy etk::UString element
				etk::UString plop = m_fileColor;
				LoadFile(plop);
			}
			break;
	}
	*/
}


void classColorManager::LoadFile(etk::UString &xmlFilename)
{
	// TODO : Remove this
	LoadFile(xmlFilename.c_str());
}

// TODO : Remove this ...
void classColorManager::LoadFile(const char * xmlFilename)
{
	// Remove all old color : 
	int32_t i;
	// clean all Element
	for (i=0; i< listMyColor.Size(); i++) {
		if (NULL != listMyColor[i]) {
			delete(listMyColor[i]);
			listMyColor[i] = NULL;
		}
	}
	// clear the compleate list
	listMyColor.Clear();

	m_fileColor = xmlFilename;
	APPL_DEBUG("open file (COLOR) \"" << xmlFilename << "\" ? = \"" << m_fileColor << "\"");
	errorColor = new Colorize();
	errorColor->SetBgColor("#00FF00FF");
	errorColor->SetFgColor("#FF00FFFF");

	// allocate the document in the stack
	TiXmlDocument XmlDocument;
	// open the curent File
	etk::FSNode fileName(etk::UString("DATA:color/") + xmlFilename + etk::UString(".xml"));
	if (false == fileName.Exist()) {
		APPL_ERROR("File Does not exist : " << fileName);
		return;
	}
	int32_t fileSize = fileName.FileSize();
	if (0==fileSize) {
		APPL_ERROR("This file is empty : " << fileName);
		return;
	}
	if (false == fileName.FileOpenRead()) {
		APPL_ERROR("Can not open the file : " << fileName);
		return;
	}
	// allocate data
	char * fileBuffer = new char[fileSize+5];
	if (NULL == fileBuffer) {
		APPL_ERROR("Error Memory allocation size=" << fileSize);
		return;
	}
	memset(fileBuffer, 0, (fileSize+5)*sizeof(char));
	// load data from the file :
	fileName.FileRead(fileBuffer, 1, fileSize);
	// close the file:
	fileName.FileClose();
	// load the XML from the memory
	XmlDocument.Parse((const char*)fileBuffer, 0, TIXML_ENCODING_UTF8);

	TiXmlElement* root = XmlDocument.FirstChildElement( "EdnColor" );
	if (NULL == root ) {
		APPL_ERROR(PFX"(l ?) main node not find: \"EdnColor\" in \"" << xmlFilename << "\"");
		return;
	} else {
		TiXmlNode * pNode = root->FirstChild();
		while(NULL != pNode) {
			if (pNode->Type()==TiXmlNode::TINYXML_COMMENT) {
				// nothing to do, just proceed to next step
			} else if (!strcmp(pNode->Value(), "gui")) {
				TiXmlNode * pGuiNode = pNode->FirstChild();
				while(NULL != pGuiNode) {
					if (pGuiNode->Type()==TiXmlNode::TINYXML_COMMENT) {
						// nothing to do, just proceed to next step
					} else if (!strcmp(pGuiNode->Value(), "color")) {
						//--------------------------------------------------------------------------------------------
						//<color name="basicBackground" val="#000000"/>
						//--------------------------------------------------------------------------------------------
						const char *colorName = pGuiNode->ToElement()->Attribute("name");
						int32_t id = 0;
						if (NULL == colorName) {
							APPL_ERROR("(l "<< pGuiNode->Row() <<") node with no name");
							// get next node element
							pGuiNode = pGuiNode->NextSibling();
							continue;
						}
						if (!strcmp(colorName, "CODE_space")) {
							id = COLOR_CODE_SPACE;
						} else if (!strcmp(colorName, "CODE_tabulation")) {
							id = COLOR_CODE_TAB;
						} else if (!strcmp(colorName, "CODE_basicBackgroung")) {
							id = COLOR_CODE_BASIC_BG;
						} else if (!strcmp(colorName, "CODE_cursor")) {
							id = COLOR_CODE_CURSOR;
						} else if (!strcmp(colorName, "CODE_lineNumber")) {
							id = COLOR_CODE_LINE_NUMBER;
						} else if (!strcmp(colorName, "LIST_backgroung1")) {
							id = COLOR_LIST_BG_1;
						} else if (!strcmp(colorName, "LIST_backgroung2")) {
							id = COLOR_LIST_BG_2;
						} else if (!strcmp(colorName, "LIST_backgroungSelected")) {
							id = COLOR_LIST_BG_SELECTED;
						} else if (!strcmp(colorName, "LIST_textNormal")) {
							id = COLOR_LIST_TEXT_NORMAL;
						} else if (!strcmp(colorName, "LIST_textModify")) {
							id = COLOR_LIST_TEXT_MODIFY;
						} else {
							APPL_ERROR("(l "<<pGuiNode->Row()<<") Unknown basic gui color : \"" << colorName << "\"" );
							// get next node element
							pGuiNode = pGuiNode->NextSibling();
							continue;
						}
						const char *color = pGuiNode->ToElement()->Attribute("val");
						if (NULL != color) {
							basicColors[id] = color;
						}
					} else {
						APPL_ERROR("(l "<<pGuiNode->Row()<<") node not suported : \""<<pGuiNode->Value()<<"\" must be [color]");
					}
					// get next node element
					pGuiNode = pGuiNode->NextSibling();
				}
			} else if (!strcmp(pNode->Value(), "syntax")) {
				TiXmlNode * pGuiNode = pNode->FirstChild();
				while(NULL != pGuiNode)
				{
					if (pGuiNode->Type()==TiXmlNode::TINYXML_COMMENT) {
						// nothing to do, just proceed to next step
					} else if (!strcmp(pGuiNode->Value(), "color")) {
						Colorize *myNewColor = new Colorize();
						//--------------------------------------------------------------------------------------------
						//<color name="basicBackground"		FG="#000000"	BG="#000000"	bold="no"	italic="no"/>
						//--------------------------------------------------------------------------------------------
						// get the name of the Chaine
						const char *colorName = pGuiNode->ToElement()->Attribute("name");
						if (NULL == colorName) {
							APPL_ERROR(PFX"(l "<< pGuiNode->Row() <<") node with no name");
							// get next node element
							pGuiNode = pGuiNode->NextSibling();
							continue;
						} else {
							myNewColor->SetName(colorName);
							//APPL_INFO(PFX"Add a new color in the panel : \"%s\"", colorName);
						}
						const char *colorBG = pGuiNode->ToElement()->Attribute("BG");
						if (NULL != colorBG) {
							myNewColor->SetBgColor(colorBG);
						}
						const char *colorFG = pGuiNode->ToElement()->Attribute("FG");
						if (NULL != colorFG) {
							myNewColor->SetFgColor(colorFG);
						}
						const char *bold = pGuiNode->ToElement()->Attribute("bold");
						if (NULL != bold) {
							if(0 == strcmp(bold, "yes") ) {
								myNewColor->SetBold(true);
							}
						}
						const char *italic = pGuiNode->ToElement()->Attribute("italic");
						if (NULL != italic) {
							if(0 == strcmp(italic, "yes") ) {
								myNewColor->SetItalic(true);
							}
						}
						listMyColor.PushBack(myNewColor);
					} else {
						APPL_ERROR(PFX"(l "<<pNode->Row()<<") node not suported : \""<<pNode->Value()<<"\" must be [color]");
					}
					pGuiNode = pGuiNode->NextSibling();
				}
			} else {
				APPL_ERROR(PFX"(l "<<pNode->Row()<<") node not suported : \""<<pNode->Value()<<"\" must be [gui,syntax]");
			}
			// get next node element
			pNode = pNode->NextSibling();
		}
	}
	if (NULL != fileBuffer) {
		delete[] fileBuffer;
	}
	//SendMessage(APPL_MSG__COLOR_HAS_CHANGE);
	//SendMessage(APPL_MSG__USER_DISPLAY_CHANGE);
}

// TODO : Remove this ...
Colorize *classColorManager::Get(const char *colorName)
{
	int32_t i;
	for (i=0; i<listMyColor.Size(); i++) {
		etk::UString elementName = listMyColor[i]->GetName();
		if (elementName == colorName) {
			return listMyColor[i];
		}
	}
	APPL_ERROR(PFX"Color does not Existed ["<< colorName<<"]" );
	// an error
	return errorColor;
}

Colorize *classColorManager::Get(etk::UString &colorName)
{
	// TODO : Remove this
	return Get(colorName.c_str());
}

draw::Color & classColorManager::Get(basicColor_te myColor)
{
	if (myColor < COLOR_NUMBER_MAX) {
		return basicColors[myColor];
	} else {
		return basicColors[0];
	}
}


// TODO : Remove this ...
bool classColorManager::Exist(const char *colorName)
{
	int32_t i;
	for (i=0; i<listMyColor.Size(); i++) {
		etk::UString elementName = listMyColor[i]->GetName();
		if (elementName == colorName) {
			return true;
		}
	}
	return false;
}

bool classColorManager::Exist(etk::UString &colorName)
{
	// TODO : Remove this
	return Exist(colorName.c_str());
}

void classColorManager::DisplayListOfColor(void)
{
	int32_t i;
	APPL_INFO(PFX"List of ALL COLOR : ");
	for (i=0; i<listMyColor.Size(); i++) {
		//etk::UString elementName = listMyColor[i]->GetName();
		//APPL_INFO(i << " : \"" <<  elementName.c_str() << "\"" );
		listMyColor[i]->Display(i);
	}
}



static classColorManager * localManager = NULL;


void ColorizeManager::Init(void)
{
	if (NULL != localManager) {
		EWOL_ERROR("ColorizeManager ==> already exist, just unlink the previous ...");
		localManager = NULL;
	}
	localManager = new classColorManager();
	
	if (NULL == localManager) {
		EWOL_CRITICAL("Allocation of HighlightManager not done ...");
	}
}

void ColorizeManager::UnInit(void)
{
	if (NULL == localManager) {
		EWOL_ERROR("ColorizeManager ==> request UnInit, but does not exist ...");
		return;
	}
	delete(localManager);
	localManager = NULL;
}

void ColorizeManager::LoadFile(etk::UString &xmlFilename)
{
	if (NULL == localManager) {
		return;
	}
	localManager->LoadFile(xmlFilename);
}


void ColorizeManager::LoadFile(const char * xmlFilename)
{
	if (NULL == localManager) {
		return;
	}
	localManager->LoadFile(xmlFilename);
}

Colorize* ColorizeManager::Get(const char *colorName)
{
	if (NULL == localManager) {
		return NULL;
	}
	return localManager->Get(colorName);
}

Colorize* ColorizeManager::Get(etk::UString &colorName)
{
	if (NULL == localManager) {
		return NULL;
	}
	return localManager->Get(colorName);
}

draw::Color errorColor;

draw::Color& ColorizeManager::Get(basicColor_te myColor)
{
	if (NULL == localManager) {
		return errorColor;
	}
	return localManager->Get(myColor);
}

bool ColorizeManager::Exist(etk::UString &colorName)
{
	if (NULL == localManager) {
		return false;
	}
	return localManager->Exist(colorName);
}

bool ColorizeManager::Exist(const char *colorName)
{
	if (NULL == localManager) {
		return false;
	}
	return localManager->Exist(colorName);
}

void ColorizeManager::DisplayListOfColor(void)
{
	if (NULL == localManager) {
		return;
	}
	localManager->DisplayListOfColor();
}


