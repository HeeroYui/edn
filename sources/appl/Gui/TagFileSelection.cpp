/**
 * @author Edouard DUPIN
 * 
 * @copyright 2010, Edouard DUPIN, all right reserved
 * 
 * @license GPL v3 (see license file)
 */

#include <appl/Gui/TagFileSelection.h>
#include <ewol/widget/Sizer.h>
#include <ewol/widget/List.h>
#include <ewol/widget/Spacer.h>
#include <ewol/widget/Image.h>
#include <ewol/widget/Manager.h>
#include <vector>
#include <etk/tool.h>
#include <ewol/widget/Button.h>
#include <ewol/widget/Label.h>
#include <ewol/widget/Composer.h>

#include <ewol/ewol.h>



#undef __class__
#define __class__ "TagFileSelection"

appl::TagFileSelection::TagFileSelection() :
  signalSelect(this, "select", ""),
  signalCancel(this, "cancel", "") {
	addObjectType("appl::TagFileSelection");
}

void appl::TagFileSelection::init() {
	ewol::widget::PopUp::init();
	std::shared_ptr<ewol::widget::Sizer> mySizerVert;
	#if defined(__TARGET_OS__Android)
		propertyMinSize.set(gale::Dimension(vec2(90,90), gale::Dimension::Pourcent));
	#elif defined(__TARGET_OS__Windows)
		propertyMinSize.set(gale::Dimension(vec2(80,80), gale::Dimension::Pourcent));
	#else
		propertyMinSize.set(gale::Dimension(vec2(80,80), gale::Dimension::Pourcent));
	#endif
	
	mySizerVert = ewol::widget::Sizer::create(ewol::widget::Sizer::modeVert);
	if (nullptr == mySizerVert) {
		EWOL_ERROR("Can not allocate widget  == > display might be in error");
	} else {
		mySizerVert->propertyLockExpand.set(bvec2(true,true));
		mySizerVert->propertyExpand.set(bvec2(true,true));
		// set it in the pop-up-system : 
		setSubWidget(mySizerVert);
		std::shared_ptr<ewol::widget::Composer> compose = ewol::widget::Composer::create(ewol::widget::Composer::String,
		   "<sizer mode=\"hori\" expand=\"true,false\" lock=\"false,true\">\n"
		   "	<spacer expand=\"true,false\"/>\n"
		   "	<button name=\"PLUGIN-CTAGS-jump\" expand=\"false\" fill=\"true\">"
		   "		<sizer mode=\"hori\">\n"
		   "			<image src=\"THEME:GUI:Load.svg\" fill=\"true\" size=\"10,10mm\"/>\n"
		   "			<label>Jump</label>\n"
		   "		</sizer>\n"
		   "	</button>\n"
		   "	<button name=\"PLUGIN-CTAGS-cancel\" expand=\"false\" fill=\"true\">"
		   "		<sizer mode=\"hori\">\n"
		   "			<image src=\"THEME:GUI:Remove.svg\" fill=\"true\" size=\"10,10mm\"/>\n"
		   "			<label>Cancel</label>\n"
		   "		</sizer>\n"
		   "	</button>\n"
		   "</sizer>\n");
		compose->propertyExpand.set(bvec2(true,false));
		compose->propertyFill.set(bvec2(true,true));
		mySizerVert->subWidgetAdd(compose);
		externSubBind(compose, ewol::widget::Button, "PLUGIN-CTAGS-jump", signalPressed, shared_from_this(), &appl::TagFileSelection::onCallbackCtagsSelection);
		externSubBind(compose, ewol::widget::Button, "PLUGIN-CTAGS-cancel", signalPressed, shared_from_this(), &appl::TagFileSelection::onCallbackCtagsCancel);
		
		m_listTag = appl::TagFileList::create();
		if (m_listTag == nullptr) {
			EWOL_ERROR("Can not allocate widget  == > display might be in error");
		} else {
			m_listTag->signalValidate.connect(shared_from_this(), &appl::TagFileSelection::onCallbackCtagsListValidate);
			m_listTag->signalSelect.connect(shared_from_this(), &appl::TagFileSelection::onCallbackCtagsListSelect);
			m_listTag->signalUnSelect.connect(shared_from_this(), &appl::TagFileSelection::onCallbackCtagsListUnSelect);
			m_listTag->propertyExpand.set(bvec2(true,true));
			m_listTag->propertyFill.set(bvec2(true,true));
			mySizerVert->subWidgetAdd(m_listTag);
		}
		
		std::shared_ptr<ewol::widget::Label> myWidgetTitle;
		myWidgetTitle = ewol::widget::Label::create("Ctags Jump Selection ...");
		if (myWidgetTitle == nullptr) {
			EWOL_ERROR("Can not allocate widget  == > display might be in error");
		} else {
			mySizerVert->subWidgetAdd(myWidgetTitle);
		}
	}
}


appl::TagFileSelection::~TagFileSelection() {
	
}

void appl::TagFileSelection::onCallbackCtagsSelection() {
	if (m_eventNamed != "") {
		signalSelect.emit(m_eventNamed);
		// == > Auto remove ...
		autoDestroy();
	}
}

void appl::TagFileSelection::onCallbackCtagsCancel() {
	signalCancel.emit();
	// == > Auto remove ...
	autoDestroy();
}

void appl::TagFileSelection::onCallbackCtagsListValidate(const std::string& _value) {
	signalSelect.emit(_value);
	// == > Auto remove ...
	autoDestroy();
}

void appl::TagFileSelection::onCallbackCtagsListSelect(const std::string& _value) {
	m_eventNamed = _value;
}

void appl::TagFileSelection::onCallbackCtagsListUnSelect() {
	m_eventNamed = "";
}



/**
 * @brief add a Ctags item on the curent list
 * @param[in] file Compleate file name
 * @param[in] jump line id
 */
void appl::TagFileSelection::addCtagsNewItem(std::string _file, int32_t _line) {
	if (m_listTag != nullptr) {
		m_listTag->add(_file, _line);
	}
}

