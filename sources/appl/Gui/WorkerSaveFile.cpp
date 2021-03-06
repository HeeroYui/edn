/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#include <ewol/context/Context.h>
#include <appl/debug.h>
#include <appl/Gui/WorkerSaveFile.h>
#include <ewol/tools/message.h>

appl::WorkerSaveFile::WorkerSaveFile() :
  signalSaveDone(this, "save-file-done", ""),
  signalAbort(this, "save-file-abort", ""),
  propertyBufferName(this, "buffer-name", "", ""),
  propertyForceSave(this, "force-save", false, "request save in all case") {
	addObjectType("appl::WorkerSaveFile");
	// load buffer manager:
	m_bufferManager = appl::BufferManager::create();
}

void appl::WorkerSaveFile::init() {
	ewol::object::Worker::init();
	if (m_bufferManager == nullptr) {
		APPL_ERROR("can not call unexistant buffer manager ... ");
		destroy();
		return;
	}
	if (*propertyBufferName == "") {
		// need to find the curent file ...
		ememory::SharedPtr<appl::Buffer> tmpp = m_bufferManager->getBufferSelected();
		if (tmpp == nullptr) {
			APPL_ERROR("No selected buffer now ...");
			destroy();
			return;
		}
		propertyBufferName.setDirect(tmpp->getFileName());
	}
	if (m_bufferManager->exist(*propertyBufferName) == false) {
		APPL_ERROR("Try to save an non-existant file :" << *propertyBufferName);
		destroy();
		return;
	}
	ememory::SharedPtr<appl::Buffer> tmpBuffer = m_bufferManager->get(*propertyBufferName);
	if (tmpBuffer == nullptr) {
		APPL_ERROR("Error to get the buffer : " << *propertyBufferName);
		destroy();
		return;
	}
	if (*propertyForceSave == false) {
		if (tmpBuffer->hasFileName() == true) {
			tmpBuffer->storeFile();
			signalSaveDone.emit();
			destroy();
			return;
		}
	}
	m_chooser = ewol::widget::FileChooser::create();
	if (m_chooser == nullptr) {
		APPL_ERROR("Can not allocate widget  == > display might be in error");
		destroy();
		return;
	}
	m_chooser->propertyLabelTitle.set("_T{Save files As...}");
	m_chooser->propertyLabelValidate.set("_T{Save}");
	etk::FSNode tmpName(*propertyBufferName);
	m_chooser->propertyPath.set(tmpName.getNameFolder());
	m_chooser->propertyFile.set(tmpName.getNameFile());
	ememory::SharedPtr<ewol::widget::Windows> tmpWindows = ewol::getContext().getWindows();
	if (tmpWindows == nullptr) {
		APPL_ERROR("Error to get the windows.");
		destroy();
		return;
	}
	tmpWindows->popUpWidgetPush(m_chooser);
	m_chooser->signalValidate.connect(sharedFromThis(), &appl::WorkerSaveFile::onCallbackSaveAsValidate);
	m_chooser->signalCancel.connect(sharedFromThis(), &appl::WorkerSaveFile::onCallbackCancel);
}

appl::WorkerSaveFile::~WorkerSaveFile() {
	APPL_ERROR("Remove Worker");
}

void appl::WorkerSaveFile::onCallbackCancel() {
	signalAbort.emit();
	destroy();
}

void appl::WorkerSaveFile::onCallbackSaveAsValidate(const std::string& _value) {
	if (m_bufferManager == nullptr) {
		// nothing to do in this case ==> can do nothing ...
		destroy();
		return;
	}
	if (_value == "") {
		APPL_ERROR(" might be an error of the File chooser system...");
		destroy();
		return;
	}
	if (m_bufferManager->exist(*propertyBufferName) == false) {
		APPL_ERROR("Try to save an non-existant file :" << *propertyBufferName);
		destroy();
		return;
	}
	ememory::SharedPtr<appl::Buffer> tmpBuffer = m_bufferManager->get(*propertyBufferName);
	if (tmpBuffer == nullptr) {
		APPL_ERROR("Error to get the buffer : " << *propertyBufferName);
		destroy();
		return;
	}
	tmpBuffer->setFileName(_value);
	if (tmpBuffer->storeFile() == false) {
		ewol::tools::message::displayWarning("We can not save the file : <br/><i>" + tmpBuffer->getFileName() + "</i>");
	} else {
		signalSaveDone.emit();
	}
	destroy();
}

