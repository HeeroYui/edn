/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#pragma once

#include <etk/types.hpp>
#include <appl/globalMsg.hpp>
#include <appl/Highlight.hpp>
#include <ewol/widget/Widget.hpp>

namespace appl {
	namespace highlightManager {
		/**
		 * @brief Init the Highlight manager
		 */
		void init();
		/**
		 * @brief Un-Init the Highlight manager
		 */
		void unInit();
		/**
		 * @brief Un-Init the Highlight manager
		 * @param[in] _fileName name of the file
		 * @return type of highlight
		 */
		etk::String getTypeFile(const etk::Path& _fileName);
		/**
		 * @brief Get filename with type.
		 * @param[in] _type Type name of the highlight.
		 * @return filename of the highlight.
		 */
		etk::String getFileWithTypeType(const etk::String& _type);
		/**
		 * @brief Get the list of extention type
		 * @return the requested list.
		 */
		etk::Vector<etk::String> getTypeList();
	};
}

