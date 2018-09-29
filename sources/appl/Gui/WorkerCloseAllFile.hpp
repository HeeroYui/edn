/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#pragma once
#include <ewol/object/Worker.hpp>
#include <appl/BufferManager.hpp>
#include <appl/Gui/WorkerCloseFile.hpp>

namespace appl {
	class WorkerCloseAllFile : public ewol::object::Worker {
		protected:
			WorkerCloseAllFile();
			void init();
		public:
			DECLARE_FACTORY(WorkerCloseAllFile);
			virtual ~WorkerCloseAllFile();
		private:
			etk::Vector<etk::Path> m_bufferNameList;
			ememory::SharedPtr<appl::WorkerCloseFile> m_worker; //! pop-up element that is open...
			ememory::SharedPtr<appl::BufferManager> m_bufferManager; //!< handle on the buffer manager
		public: // callback function
			void onCallbackCloseDone();
	};
}
