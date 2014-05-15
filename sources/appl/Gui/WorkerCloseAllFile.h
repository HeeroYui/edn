/**
 * @author Edouard DUPIN
 * 
 * @copyright 2010, Edouard DUPIN, all right reserved
 * 
 * @license GPL v3 (see license file)
 */

#ifndef __WORKER_CLOSE_ALL_FILE_H__
#define __WORKER_CLOSE_ALL_FILE_H__

#include <appl/BufferManager.h>
#include <appl/Gui/WorkerCloseFile.h>

namespace appl {
	class WorkerCloseAllFile : public ewol::Object {
		public:
			WorkerCloseAllFile();
			virtual ~WorkerCloseAllFile();
		private:
			std::vector<std::string> m_bufferNameList;
			appl::WorkerCloseFile* m_worker; //! pop-up element that is open...
			appl::BufferManager* m_bufferManager; //!< handle on the buffer manager
		public: // derived function
			virtual void onReceiveMessage(const ewol::object::Message& _msg);
			virtual void onObjectRemove(ewol::Object * _removeObject);
	};
};

#endif

