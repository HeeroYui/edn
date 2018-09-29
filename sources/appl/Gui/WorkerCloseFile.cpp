/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#include <ewol/context/Context.hpp>
#include <appl/debug.hpp>
#include <appl/Gui/WorkerCloseFile.hpp>
#include <ewol/widget/meta/StdPopUp.hpp>
#include <ewol/tools/message.hpp>

appl::WorkerCloseFile::WorkerCloseFile() :
  signalCloseDone(this, "close-file-done", ""),
  signalAbort(this, "close-file-abort", ""),
  m_buffer(null),
  m_worker(null),
  m_bufferManager(null) {
	addObjectType("appl::WorkerCloseFile");
	// load buffer manager:
	m_bufferManager = appl::BufferManager::create();
}

void appl::WorkerCloseFile::init() {
	ewol::object::Worker::init();
}

void appl::WorkerCloseFile::startAction(const etk::Path& _bufferName) {
	m_bufferName = _bufferName;
	if (m_bufferManager == null) {
		APPL_ERROR("can not call unexistant buffer manager ... ");
		destroy();
		return;
	}
	if (m_bufferName.isEmpty() == true) {
		// need to find the curent file ...
		ememory::SharedPtr<appl::Buffer> tmpp = m_bufferManager->getBufferSelected();
		if (tmpp == null) {
			APPL_ERROR("No selected buffer now ...");
			destroy();
			return;
		}
		m_bufferName = tmpp->getFileName();
	}
	if (m_bufferManager->exist(m_bufferName) == false) {
		APPL_ERROR("Try to close an non-existant file :" << m_bufferName);
		destroy();
		return;
	}
	m_buffer = m_bufferManager->get(m_bufferName);
	if (m_buffer == null) {
		APPL_ERROR("Error to get the buffer : " << m_bufferName);
		destroy();
		return;
	}
	if (m_buffer->isModify() == false) {
		signalCloseDone.emit();
		m_buffer->destroy();
		destroy();
		return;
	}
	
	ememory::SharedPtr<ewol::widget::StdPopUp> tmpPopUp = ewol::widget::StdPopUp::create();
	if (tmpPopUp == null) {
		APPL_ERROR("Can not create a simple pop-up");
		destroy();
		return;
	}
	tmpPopUp->propertyTitle.set("<bold>_T{Close un-saved file:}</bold>");
	tmpPopUp->propertyComment.set("_T{The file named:} <i>'" + m_buffer->getFileName().getString() + "'</i> _T{is curently modify.}<br/>_T{If you don't saves these modifications,}<br/>_T{they will be definitly lost...}");
	ememory::SharedPtr<ewol::widget::Button> bt = null;
	if (m_buffer->hasFileName() == true) {
		bt = tmpPopUp->addButton("_T{Save}", true);
		if (bt != null) {
			bt->signalPressed.connect(sharedFromThis(), &appl::WorkerCloseFile::onCallbackSaveValidate);
		}
	}
	bt = tmpPopUp->addButton("_T{Save As}", true);
	if (bt != null) {
		bt->signalPressed.connect(sharedFromThis(), &appl::WorkerCloseFile::onCallbackSaveAsValidate);
	}
	bt = tmpPopUp->addButton("_T{Close}", true);
	if (bt != null) {
		bt->signalPressed.connect(sharedFromThis(), &appl::WorkerCloseFile::onCallbackClose);
	}
	bt = tmpPopUp->addButton("_T{Cancel}", true);
	if (bt != null) {
		bt->signalPressed.connect(sharedFromThis(), &appl::WorkerCloseFile::onCallbackCancel);
	}
	tmpPopUp->propertyCloseOutEvent.set(true);
	ememory::SharedPtr<ewol::widget::Windows> tmpWindows = ewol::getContext().getWindows();
	if (tmpWindows == null) {
		APPL_ERROR("Error to get the windows.");
		destroy();
		return;
	}
	tmpWindows->popUpWidgetPush(tmpPopUp);
}

appl::WorkerCloseFile::~WorkerCloseFile() {
	APPL_ERROR("Remove Worker");
}

void appl::WorkerCloseFile::onCallbackCancel() {
	APPL_VERBOSE("Cancel signal ...");
	signalAbort.emit();
	destroy();
}

void appl::WorkerCloseFile::onCallbackSaveAsValidate() {
	if (m_bufferManager == null) {
		// nothing to do in this case ==> can do nothing ...
		return;
	}
	m_worker = appl::WorkerSaveFile::create("buffer-name", m_bufferName);
	if (m_worker != null) {
		m_worker->signalSaveDone.connect(sharedFromThis(), &appl::WorkerCloseFile::onCallbackClose);
		m_worker->signalAbort.connect(sharedFromThis(), &appl::WorkerCloseFile::onCallbackCancel);
	}
}

void appl::WorkerCloseFile::onCallbackSaveValidate() {
	if (m_bufferManager == null) {
		// nothing to do in this case ==> can do nothing ...
		signalAbort.emit();
		destroy();
		return;
	}
	if (m_buffer == null) {
		APPL_ERROR("Error to get the buffer : oldName=" << m_bufferName);
		signalAbort.emit();
		destroy();
		return;
	}
	if (m_buffer->storeFile() == false) {
		ewol::tools::message::displayWarning("We can not save the file : <br/><i>" + m_buffer->getFileName().getString() + "</i>");
		signalAbort.emit();
	} else {
		m_buffer->destroy();
		m_buffer.reset();
		signalCloseDone.emit();
	}
	destroy();
}

void appl::WorkerCloseFile::onCallbackClose() {
	if (m_bufferManager == null) {
		// nothing to do in this case ==> can do nothing ...
		signalAbort.emit();
		destroy();
		return;
	}
	if (m_buffer == null) {
		APPL_ERROR("Error to get the buffer : " << m_bufferName);
		signalAbort.emit();
		destroy();
		return;
	}
	m_buffer->destroy();
	m_buffer.reset();
	signalCloseDone.emit();
	destroy();
}

