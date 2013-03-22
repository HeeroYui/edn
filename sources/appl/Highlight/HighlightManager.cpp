/**
 * @author Edouard DUPIN
 * 
 * @copyright 2010, Edouard DUPIN, all right reserved
 * 
 * @license GPL v3 (see license file)
 */

#include <appl/Debug.h>
#include <appl/global.h>
#include <HighlightManager.h>
#include <ewol/eObject/EObject.h>
#include <ewol/eObject/EObjectManager.h>

#undef __class__
#define __class__	"HighlightManager"

class localClassHighlightManager: public ewol::EObject
{
	private:
		etk::Vector<Highlight*> listHighlight;		//!< List of ALL hightlight modules
	public:
		// Constructeur
		localClassHighlightManager(void) {
			//ewol::widgetMessageMultiCast::Add(GetWidgetId(), ednMsgBufferColor);
		};
		~localClassHighlightManager(void) {
			int32_t i;
			// clean all Element
			for (i=0; i< listHighlight.Size(); i++) {
				if (NULL != listHighlight[i]) {
					delete(listHighlight[i]);
					listHighlight[i] = NULL;
				}
			}
			// clear the compleate list
			listHighlight.Clear();
		};
		
		// herited function
		const char * const GetObjectType(void)
		{
			return "ApplHighlightManager";
		}
		
		// herited function
		virtual void OnReceiveMessage(ewol::EObject * CallerObject, const char * eventId, const etk::UString& data)
		{
			/*
			switch (id)
			{
				case APPL_MSG__COLOR_HAS_CHANGE:
					APPL_INFO("UPDATE the color pointer on the HL");
					for (int32_t i=0; i<listHighlight.Size(); i++) {
						if (NULL != listHighlight[i]) {
							listHighlight[i]->ReloadColor();
						}
					}
					break;
			}
			*/
		}
		
		Highlight* Get(etk::FSNode &fileName)
		{
			int32_t i;
			for (i=0; i<listHighlight.Size(); i++) {
				if (true == listHighlight[i]->FileNameCompatible(fileName) ) {
					return listHighlight[i];
				}
			}
			return NULL;
		}
		
		bool Exist(etk::FSNode &fileName)
		{
			if (NULL != Get(fileName) ) {
				return true;
			}
			return false;
		}
		
		
		void loadLanguages(void)
		{
			etk::UString xmlFilename = "languages/c/highlight.xml";
			Highlight *myHightline = new Highlight(xmlFilename);
			listHighlight.PushBack(myHightline);
			
			xmlFilename = "languages/boo/highlight.xml";
			myHightline = new Highlight(xmlFilename);
			listHighlight.PushBack(myHightline);
			
			xmlFilename = "languages/makefile/highlight.xml";
			myHightline = new Highlight(xmlFilename);
			listHighlight.PushBack(myHightline);
			
			xmlFilename = "languages/asm/highlight.xml";
			myHightline = new Highlight(xmlFilename);
			listHighlight.PushBack(myHightline);
			
			xmlFilename = "languages/xml/highlight.xml";
			myHightline = new Highlight(xmlFilename);
			listHighlight.PushBack(myHightline);
			
			xmlFilename = "languages/php/highlight.xml";
			myHightline = new Highlight(xmlFilename);
			listHighlight.PushBack(myHightline);
			
			xmlFilename = "languages/bash/highlight.xml";
			myHightline = new Highlight(xmlFilename);
			listHighlight.PushBack(myHightline);
			
			xmlFilename = "languages/matlab/highlight.xml";
			myHightline = new Highlight(xmlFilename);
			listHighlight.PushBack(myHightline);
			
			xmlFilename = "languages/java/highlight.xml";
			myHightline = new Highlight(xmlFilename);
			listHighlight.PushBack(myHightline);
			
			xmlFilename = "languages/lua/highlight.xml";
			myHightline = new Highlight(xmlFilename);
			listHighlight.PushBack(myHightline);
			
			xmlFilename = "languages/in/highlight.xml";
			myHightline = new Highlight(xmlFilename);
			listHighlight.PushBack(myHightline);
			
			xmlFilename = "languages/glsl/highlight.xml";
			myHightline = new Highlight(xmlFilename);
			listHighlight.PushBack(myHightline);
			
			//myHightline->Display();
		}

};

static localClassHighlightManager * localManager = NULL;



void HighlightManager::Init(void)
{
	if (NULL != localManager) {
		APPL_ERROR("HighlightManager ==> already exist, just unlink the previous ...");
		localManager = NULL;
	}
	localManager = new localClassHighlightManager();
	
	if (NULL == localManager) {
		APPL_CRITICAL("Allocation of HighlightManager not done ...");
	}
}

void HighlightManager::UnInit(void)
{
	if (NULL == localManager) {
		APPL_ERROR("HighlightManager ==> request UnInit, but does not exist ...");
		return;
	}
	delete(localManager);
	localManager = NULL;
}

void HighlightManager::loadLanguages(void)
{
	if (NULL == localManager) {
		return;
	}
	localManager->loadLanguages();
}

Highlight* HighlightManager::Get(etk::FSNode &fileName)
{
	if (NULL == localManager) {
		return NULL;
	}
	return localManager->Get(fileName);
}

bool HighlightManager::Exist(etk::FSNode &fileName)
{
	if (NULL == localManager) {
		return false;
	}
	return localManager->Exist(fileName);
}



