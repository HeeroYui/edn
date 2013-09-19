/**
 * @author Edouard DUPIN
 * 
 * @copyright 2010, Edouard DUPIN, all right reserved
 * 
 * @license GPL v3 (see license file)
 */

#ifndef __APPL_TEXT_VIEWER_H__
#define __APPL_TEXT_VIEWER_H__

#include <appl/Debug.h>
#include <CodeView.h>
#include <appl/Buffer/Buffer.h>
#include <appl/globalMsg.h>

#include <ewol/widget/WidgetScrolled.h>
#include <ewol/compositing/Text.h>
#include <ewol/compositing/Drawing.h>

namespace appl
{
	class TextViewer : public widget::WidgetScrooled
	{
		public:
			TextViewer(const etk::UString& _fontName="", int32_t _fontSize=-1);
			virtual ~TextViewer(void);
		private:
			appl::Buffer* m_buffer;
			// drawing elements :
			ewol::Text m_displayText;
			ewol::Drawing m_displayDrawing;
		public:
			void SetFontSize(int32_t _size);
			void SetFontName(const etk::UString& _fontName);
		private:
			void CalculateMaxSize(void);
		protected: // derived function
			virtual void OnDraw(void);
		public:  // Derived function
			const char * const GetObjectType(void) { return "ApplCodeView"; };
			virtual bool CalculateMinSize(void);
			virtual void OnRegenerateDisplay(void);
			virtual void OnReceiveMessage(const ewol::EMessage& _msg);
			virtual bool OnEventInput(const ewol::EventInput& _event);
			virtual bool OnEventEntry(const ewol::EventEntry& _event);
			virtual void OnEventClipboard(ewol::clipBoard::clipboardListe_te clipboardID);
			virtual void OnGetFocus(void);
			virtual void OnLostFocus(void);
	};
};

#endif
