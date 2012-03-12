/**
 *******************************************************************************
 * @file ColorizeManager.cpp
 * @brief Editeur De N'ours : Colorising Manager
 * @author Edouard DUPIN
 * @date 14/12/2010
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
#include <tools_debug.h>
#include <tools_globals.h>
#include <ColorizeManager.h>
#include <tinyXML/tinyxml.h>
#include <ewol/EObject.h>
#include <ewol/EObjectManager.h>

#define PFX	"ColorizeManager "



class classColorManager: public ewol::EObject
{
	public:
		// Constructeur
		classColorManager(void);
		~classColorManager(void);
	public:
		/**
		 * @brief Receive a message from an other EObject with a specific eventId and data
		 * @param[in] CallerObject Pointer on the EObject that information came from
		 * @param[in] eventId Message registered by this class
		 * @param[in] data Data registered by this class
		 * @return ---
		 */
		virtual void OnReceiveMessage(ewol::EObject * CallerObject, const char * eventId, etk::UString data);
	public:
		void        LoadFile(etk::UString &xmlFilename);
		void        LoadFile(const char * xmlFilename);
		Colorize *  Get(const char *colorName);
		Colorize *  Get(etk::UString &colorName);
		color_ts &  Get(basicColor_te myColor);
		bool        Exist(etk::UString &colorName);
		bool        Exist(const char *colorName);
		void        DisplayListOfColor(void);

	private:
		etk::UString                 m_fileColor;
		etk::VectorType<Colorize*>  listMyColor;		//!< List of ALL Color
		Colorize *                  errorColor;
		color_ts                    basicColors[COLOR_NUMBER_MAX];
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

void classColorManager::OnReceiveMessage(ewol::EObject * CallerObject, const char * eventId, etk::UString data)
{
	/*
	switch (id)
	{
		case EDN_MSG__RELOAD_COLOR_FILE:
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
	LoadFile(xmlFilename.Utf8Data());
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
	EDN_DEBUG("open file (COLOR) \"" << xmlFilename << "\" ? = \"" << m_fileColor << "\"");
	errorColor = new Colorize();
	errorColor->SetBgColor("#000000");
	errorColor->SetFgColor("#FFFFFF");

	// allocate the document in the stack
	TiXmlDocument XmlDocument;
	// open the curent File
	etk::File fileName(xmlFilename, etk::FILE_TYPE_DATA);
	if (false == fileName.Exist()) {
		EDN_ERROR("File Does not exist : " << fileName);
		return;
	}
	int32_t fileSize = fileName.Size();
	if (0==fileSize) {
		EDN_ERROR("This file is empty : " << fileName);
		return;
	}
	if (false == fileName.fOpenRead()) {
		EDN_ERROR("Can not open the file : " << fileName);
		return;
	}
	// allocate data
	char * fileBuffer = new char[fileSize+5];
	if (NULL == fileBuffer) {
		EDN_ERROR("Error Memory allocation size=" << fileSize);
		return;
	}
	memset(fileBuffer, 0, (fileSize+5)*sizeof(char));
	// load data from the file :
	fileName.fRead(fileBuffer, 1, fileSize);
	// close the file:
	fileName.fClose();
	// load the XML from the memory
	XmlDocument.Parse((const char*)fileBuffer, 0, TIXML_ENCODING_UTF8);

	TiXmlElement* root = XmlDocument.FirstChildElement( "EdnColor" );
	if (NULL == root ) {
		EDN_ERROR(PFX"(l ?) main node not find: \"EdnColor\" in \"" << xmlFilename << "\"");
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
							EDN_ERROR("(l "<< pGuiNode->Row() <<") node with no name");
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
							EDN_ERROR("(l "<<pGuiNode->Row()<<") Unknown basic gui color : \"" << colorName << "\"" );
							// get next node element
							pGuiNode = pGuiNode->NextSibling();
							continue;
						}
						const char *color = pGuiNode->ToElement()->Attribute("val");
						if (NULL != color) {
							int r=0;
							int v=0;
							int b=0;
							int a=-1;
							sscanf(color, "#%02x%02x%02x%02x", &r, &v, &b, &a);
							basicColors[id].red = (float)r/255.0;
							basicColors[id].green = (float)v/255.0;
							basicColors[id].blue = (float)b/255.0;
							if (-1 == a) {
								basicColors[id].alpha = 1;
							} else {
								basicColors[id].alpha = (float)a/255.0;
							}
							/*
							EDN_INFO(" Specify color for system ID="<< id );
							EDN_INFO("    " << color << " ==> r="<< r <<" v="<< v <<" b="<< b );
							EDN_INFO("    " << color << " ==> r="<< basicColors[id].red <<" v="<< basicColors[id].green <<" b="<< basicColors[id].blue );
							*/
						}
					} else {
						EDN_ERROR("(l "<<pGuiNode->Row()<<") node not suported : \""<<pGuiNode->Value()<<"\" must be [color]");
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
							EDN_ERROR(PFX"(l "<< pGuiNode->Row() <<") node with no name");
							// get next node element
							pGuiNode = pGuiNode->NextSibling();
							continue;
						} else {
							myNewColor->SetName(colorName);
							//EDN_INFO(PFX"Add a new color in the panel : \"%s\"", colorName);
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
						EDN_ERROR(PFX"(l "<<pNode->Row()<<") node not suported : \""<<pNode->Value()<<"\" must be [color]");
					}
					pGuiNode = pGuiNode->NextSibling();
				}
			} else {
				EDN_ERROR(PFX"(l "<<pNode->Row()<<") node not suported : \""<<pNode->Value()<<"\" must be [gui,syntax]");
			}
			// get next node element
			pNode = pNode->NextSibling();
		}
	}
	if (NULL != fileBuffer) {
		delete[] fileBuffer;
	}
	//SendMessage(EDN_MSG__COLOR_HAS_CHANGE);
	//SendMessage(EDN_MSG__USER_DISPLAY_CHANGE);
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
	EDN_ERROR(PFX"Color does not Existed ["<< colorName<<"]" );
	// an error
	return errorColor;
}

Colorize *classColorManager::Get(etk::UString &colorName)
{
	// TODO : Remove this
	return Get(colorName.Utf8Data());
}

color_ts & classColorManager::Get(basicColor_te myColor)
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
	return Exist(colorName.Utf8Data());
}

void classColorManager::DisplayListOfColor(void)
{
	int32_t i;
	EDN_INFO(PFX"List of ALL COLOR : ");
	for (i=0; i<listMyColor.Size(); i++) {
		//etk::UString elementName = listMyColor[i]->GetName();
		//EDN_INFO(i << " : \"" <<  elementName.c_str() << "\"" );
		listMyColor[i]->Display(i);
	}
}



static classColorManager * localManager = NULL;


void ColorizeManager::Init(void)
{
	if (NULL == localManager) {
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
	ewol::EObjectManager::MarkToRemoved(localManager);
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

color_ts errorColor;

color_ts& ColorizeManager::Get(basicColor_te myColor)
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


