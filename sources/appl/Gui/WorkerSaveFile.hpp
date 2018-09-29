/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#pragma once
#include <ewol/widget/meta/FileChooser.hpp>
#include <ewol/object/Worker.hpp>
#include <appl/BufferManager.hpp>

namespace appl {
	class WorkerSaveFile : public ewol::object::Worker {
		public:
			esignal::Signal<> signalSaveDone;
			esignal::Signal<> signalAbort;
			eproperty::Value<etk::Path> propertyBufferName;
			eproperty::Value<bool> propertyForceSave;
		protected:
			WorkerSaveFile();
			void init();
		public:
			DECLARE_FACTORY(WorkerSaveFile);
			virtual ~WorkerSaveFile();
		private:
			ememory::SharedPtr<ewol::widget::FileChooser> m_chooser; //! pop-up element that is open...
			ememory::SharedPtr<appl::BufferManager> m_bufferManager; //!< handle on the buffer manager
		public: // callback function
			void onCallbackSaveAsValidate(const etk::Path& _value);
			void onCallbackCancel();
	};
}

