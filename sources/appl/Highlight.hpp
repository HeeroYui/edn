/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#pragma once

namespace appl {
	class Highlight;
	class HighlightPattern;
	
	class HighlightInfo {
		public:
			int32_t start;
			int32_t stop;
			bool notEnded;
			appl::HighlightPattern* patern;
	};
}

#include <ememory/memory.hpp>
#include <appl/HighlightPattern.hpp>
#include <appl/GlyphPainting.hpp>
#include <etk/Buffer.hpp>
#include <exml/exml.hpp>

namespace appl {
	class Highlight : public gale::Resource {
		private:
			ememory::SharedPtr<appl::GlyphPainting> m_paintingProperties;
		public:
			// Constructeur
			Highlight();
			void init(const etk::Uri& _uriXML, const etk::Uri& _uriColorFile = "THEME_COLOR:///textViewer.json");
		public:
			DECLARE_RESOURCE_URI_FACTORY(Highlight);
			virtual ~Highlight();
		private:
			etk::String m_typeName; //!< descriptive string type like "C/C++"
		public:
			/**
			 * @brief Get the Type of the Hightlight like c++/Bash/...
			 * @return descriptive string
			 */
			const etk::String& getTypeName() {
				return m_typeName;
			}
		public:
			bool isCompatible(const etk::Path& _name);
			bool fileNameCompatible(const etk::Path& _fileName);
			void display();
			void parse(int64_t _start,
			           int64_t _stop,
			           etk::Vector<appl::HighlightInfo>& _metaData,
			           int64_t _addingPos,
			           etk::Buffer& _buffer);
			void parse2(int64_t _start,
			            int64_t _stop,
			            etk::Vector<appl::HighlightInfo>& _metaData,
			            etk::Buffer& _buffer);
			void parseSubElement(const appl::HighlightInfo& _upper,
			                     etk::Vector<appl::HighlightInfo>& _metaData,
			                     etk::Buffer& _buffer);
		private:
			etk::String m_styleName; //!< curent style name (like "c++" or "c" or "script Bash")
			etk::Vector<etk::String> m_listExtentions; //!< List of possible extention for this high-light, like : ".c", ".cpp", ".h"
			etk::Vector<etk::String> m_listFiles; //!< List of possible filename
			etk::Vector<HighlightPattern> m_listHighlightPass1; //!< List of ALL hightlight modules (pass 1  == > when we load and wride data on the buffer)
			etk::Vector<HighlightPattern> m_listHighlightPass2; //!< List of ALL hightlight modules (pass 2  == > When we display the buffer( only the display area (100 lines)) )
			// TODO : This is bad ==> the patern ar unordered ...
			etk::Map<etk::String, etk::Vector<HighlightPattern>> m_listHighlightNamed; //!< list of all sub partern to parse...
		public: // herited function :
			virtual bool updateContext() {
				// no upfate to do ...
				return true;
			};
	};
}


