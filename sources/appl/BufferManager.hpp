/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#pragma once

#include <appl/Buffer.hpp>
#include <appl/globalMsg.hpp>
#include <ewol/widget/Widget.hpp>
#include <gale/resource/Resource.hpp>

namespace appl {
	class BufferManager;
	using BufferManagerShared = ememory::SharedPtr<appl::BufferManager>;
	using BufferManagerWeak = ememory::WeakPtr<appl::BufferManager>;
	// TODO: This is a service ...
	class BufferManager : public ewol::Object {
		public:
			esignal::Signal<ememory::SharedPtr<appl::Buffer>> signalNewBuffer;
			esignal::Signal<ememory::SharedPtr<appl::Buffer>> signalSelectFile;
			esignal::Signal<> signalTextSelectionChange;
			esignal::Signal<ememory::SharedPtr<appl::Buffer>> signalRemoveBuffer;
		protected:
			BufferManager();
		public:
			DECLARE_SINGLE_FACTORY(BufferManager, "???BufferManager???");
			virtual ~BufferManager();
		private:
			etk::Vector<ememory::SharedPtr<appl::Buffer>> m_list; // list of all buffer curently open
		public:
			/**
			 * @brief Get a specific buffer with his name (can create a new buffer).
			 * @param[in] _fileName Name of the file to open.
			 * @param[in] _createIfNeeded Create the buffer if not existed.
			 * @return a pointer on the buffer
			 */
			ememory::SharedPtr<appl::Buffer> get(const etk::Path& _fileName, bool _createIfNeeded=false);
			/**
			 * @brief Load a specific file, event if it not existed:
			 * @param[in] _fileName Name of the file to open or create.
			 */
			void open(const etk::Path& _fileName);
			/**
			 * @brief Check if a buffer is already open.
			 * @param[in] _fileName name of the file.
			 * @return true if the buffer is already open.
			 */
			bool exist(const etk::Path& _fileName);
			/**
			 * @brief Get count of all buffer availlable.
			 * @return Number of buffer
			 */
			int32_t size() const {
				return m_list.size();
			}
			/**
			 * @brief Get a pointer on a buffer Id (never remember this ID!!!).
			 * @param[in] _id Number of buffer
			 * @return pointer on the buffer
			 */
			ememory::SharedPtr<appl::Buffer> get(int32_t _id);
			/**
			 * @brief Create a new buffer empty.
			 * @return Created buffer or null.
			 */
			ememory::SharedPtr<appl::Buffer> createNewBuffer();
		private:
			ememory::SharedPtr<appl::Buffer> m_bufferSelected;
		public:
			/**
			 * @brief Set the current buffer selected
			 * @param[in] _bufferSelected Pointer on the buffer selected
			 */
			void setBufferSelected(ememory::SharedPtr<appl::Buffer> _bufferSelected);
			/**
			 * @brief Get the current buffer selected
			 * @return Pointer on the buffer selected
			 */
			ememory::SharedPtr<appl::Buffer> getBufferSelected() {
				return m_bufferSelected;
			};
		private:
			void requestDestroyFromChild(const ememory::SharedPtr<Object>& _child);
		public:
			// generic iterators:
			const etk::Vector<ememory::SharedPtr<appl::Buffer>>::Iterator begin() const {
				return m_list.begin();
			}
			const etk::Vector<ememory::SharedPtr<appl::Buffer>>::Iterator end() const {
				return m_list.end();
			}
			etk::Vector<ememory::SharedPtr<appl::Buffer>>::Iterator begin() {
				return m_list.begin();
			}
			etk::Vector<ememory::SharedPtr<appl::Buffer>>::Iterator end() {
				return m_list.end();
			}
	};
}


