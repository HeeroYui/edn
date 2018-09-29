/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#include <appl/Gui/TagFileSelection.hpp>
#include <ewol/widget/Sizer.hpp>
#include <ewol/widget/List.hpp>
#include <ewol/widget/Spacer.hpp>
#include <ewol/widget/Image.hpp>
#include <ewol/widget/Manager.hpp>
#include <etk/Vector.hpp>
#include <etk/tool.hpp>
#include <ewol/widget/Button.hpp>
#include <ewol/widget/Label.hpp>
#include <ewol/widget/Composer.hpp>

#include <ewol/ewol.hpp>

appl::TagFileSelection::TagFileSelection() :
  signalSelect(this, "select", ""),
  signalCancel(this, "cancel", "") {
	addObjectType("appl::TagFileSelection");
}

void appl::TagFileSelection::init() {
	ewol::widget::PopUp::init();
	ewol::widget::SizerShared mySizerVert;
	#if defined(__TARGET_OS__Android)
		propertyMinSize.set(gale::Dimension(vec2(90,90), gale::distance::pourcent));
	#elif defined(__TARGET_OS__Windows)
		propertyMinSize.set(gale::Dimension(vec2(80,80), gale::distance::pourcent));
	#else
		propertyMinSize.set(gale::Dimension(vec2(80,80), gale::distance::pourcent));
	#endif
	
	mySizerVert = ewol::widget::Sizer::create();
	if (mySizerVert == null) {
		EWOL_ERROR("Can not allocate widget  == > display might be in error");
		return;
	}
	mySizerVert->propertyMode.set(ewol::widget::Sizer::modeVert);
	mySizerVert->propertyLockExpand.set(bvec2(true,true));
	mySizerVert->propertyExpand.set(bvec2(true,true));
	// set it in the pop-up-system : 
	setSubWidget(mySizerVert);
	ewol::WidgetShared compose = ewol::widget::composerGenerateString(
	   "<sizer mode='hori' expand='true,false' lock='false,true'>\n"
	   "	<spacer expand='true,false'/>\n"
	   "	<button name='PLUGIN-CTAGS-jump' expand='false' fill='true'>"
	   "		<sizer mode='hori'>\n"
	   "			<image src='THEME:GUI:Load.svg' fill='true' size='10,10mm'/>\n"
	   "			<label>Jump</label>\n"
	   "		</sizer>\n"
	   "	</button>\n"
	   "	<button name='PLUGIN-CTAGS-cancel' expand='false' fill='true'>"
	   "		<sizer mode='hori'>\n"
	   "			<image src='THEME:GUI:Remove.svg' fill='true' size='10,10mm'/>\n"
	   "			<label>Cancel</label>\n"
	   "		</sizer>\n"
	   "	</button>\n"
	   "</sizer>\n");
	mySizerVert->subWidgetAdd(compose);
	externSubBind(compose, ewol::widget::Button, "PLUGIN-CTAGS-jump", signalPressed, sharedFromThis(), &appl::TagFileSelection::onCallbackCtagsSelection);
	externSubBind(compose, ewol::widget::Button, "PLUGIN-CTAGS-cancel", signalPressed, sharedFromThis(), &appl::TagFileSelection::onCallbackCtagsCancel);
	
	m_listTag = appl::TagFileList::create();
	if (m_listTag == null) {
		EWOL_ERROR("Can not allocate widget  == > display might be in error");
	} else {
		m_listTag->signalValidate.connect(sharedFromThis(), &appl::TagFileSelection::onCallbackCtagsListValidate);
		m_listTag->signalSelect.connect(sharedFromThis(), &appl::TagFileSelection::onCallbackCtagsListSelect);
		m_listTag->signalUnSelect.connect(sharedFromThis(), &appl::TagFileSelection::onCallbackCtagsListUnSelect);
		m_listTag->propertyExpand.set(bvec2(true,true));
		m_listTag->propertyFill.set(bvec2(true,true));
		mySizerVert->subWidgetAdd(m_listTag);
	}
	
	ewol::widget::LabelShared myWidgetTitle = ewol::widget::Label::create();
	if (myWidgetTitle == null) {
		EWOL_ERROR("Can not allocate widget  == > display might be in error");
		return;
	}
	myWidgetTitle->propertyValue.set("Ctags Jump Selection ...");
	mySizerVert->subWidgetAdd(myWidgetTitle);
}


appl::TagFileSelection::~TagFileSelection() {
	
}

void appl::TagFileSelection::onCallbackCtagsSelection() {
	if (m_eventLine != -1) {
		signalSelect.emit(m_eventPath, m_eventLine);
		// == > Auto remove ...
		autoDestroy();
	}
}

void appl::TagFileSelection::onCallbackCtagsCancel() {
	signalCancel.emit();
	// == > Auto remove ...
	autoDestroy();
}

void appl::TagFileSelection::onCallbackCtagsListValidate(const etk::Path& _path, const int32_t& _line) {
	signalSelect.emit(_path, _line);
	// == > Auto remove ...
	autoDestroy();
}

void appl::TagFileSelection::onCallbackCtagsListSelect(const etk::Path& _path, const int32_t& _line) {
	m_eventPath = _path;
	m_eventLine = _line;
}

void appl::TagFileSelection::onCallbackCtagsListUnSelect() {
	m_eventPath = "";
	m_eventLine = -1;
}



/**
 * @brief add a Ctags item on the curent list
 * @param[in] file Compleate file name
 * @param[in] jump line id
 */
void appl::TagFileSelection::addCtagsNewItem(etk::Path _file, int32_t _line) {
	if (m_listTag != null) {
		m_listTag->add(_file, _line);
	}
}

