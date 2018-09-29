/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#include <appl/global.hpp>
#include <appl/globalMsg.hpp>
#include <ewol/object/Object.hpp>
#include <ewol/context/Context.hpp>
#include <gale/resource/Manager.hpp>
#include <etk/theme/theme.hpp>
#include <ejson/ejson.hpp>

static etk::Uri g_baseDBName = "USER_DATA://genericConfig.json";


class myParamGlobal : public ewol::Object {
	public:
		eproperty::Value<bool> m_displayEOL;
		eproperty::Value<bool> m_AutoIndent;
		eproperty::Value<bool> m_displayTabChar;
		eproperty::Value<bool> m_displaySpaceChar;
	public : 
		myParamGlobal() :
		  m_displayEOL(this, "eol", false, "Display end of line character"),
		  m_AutoIndent(this, "auto-indent", true, "Auto indent when create new line"),
		  m_displayTabChar(this, "display-tab", true, "Display the Tab char"),
		  m_displaySpaceChar(this, "display-space", true, "Display the space char") {
			m_static = true; // Note : set the object static notification( Must be set or assert at the end of process)
			propertyName.set("edn_global_param");
			load();
		}
		void load() {
			ejson::Document database;
			bool ret = database.load(g_baseDBName);
			if (ret == false) {
				return;
			}
			ejson::Object obj = database["globals"].toObject();
			m_displayEOL.set(obj["display-eol"].toBoolean().get());
			m_AutoIndent.set(obj["auto-indent"].toBoolean().get());
			m_displayTabChar.set(obj["display-tab-char"].toBoolean().get());
			m_displaySpaceChar.set(obj["display-space-char"].toBoolean().get());
		}
		void store() {
			ejson::Document database;
			ejson::Object obj;
			database.add("globals", obj);
			obj.add("display-eol", ejson::Boolean(m_displayEOL.get()));
			obj.add("auto-indent", ejson::Boolean(m_AutoIndent.get()));
			obj.add("display-tab-char", ejson::Boolean(m_displayTabChar.get()));
			obj.add("display-space-char", ejson::Boolean(m_displaySpaceChar.get()));
			bool retGenerate = database.storeSafe(g_baseDBName);
		}
};


static myParamGlobal& l_obj() {
	static myParamGlobal s_obj;
	return s_obj;
}

void globals::init() {
	myParamGlobal& tmpVal = l_obj();
	// nothing to do ...
	(void)tmpVal;
}

void globals::UnInit() {
	// nothing to do ...
}


void globals::store() {
	l_obj().store();
}

// -----------------------------------------------------------
bool globals::isSetDisplayEndOfLine() {
	return l_obj().m_displayEOL;
}

void globals::setDisplayEndOfLine(bool newVal) {
	l_obj().m_displayEOL.set(newVal);
	ewol::getContext().forceRedrawAll();
}

// -----------------------------------------------------------
bool globals::isSetDisplaySpaceChar() {
	return l_obj().m_displaySpaceChar;
}

void globals::setDisplaySpaceChar(bool _newVal) {
	l_obj().m_displaySpaceChar.set(_newVal);
	ewol::getContext().forceRedrawAll();
}
// -----------------------------------------------------------
bool globals::isSetDisplayTabChar() {
	return l_obj().m_displayTabChar;
}

void globals::setDisplayTabChar(bool _newVal) {
	l_obj().m_displayTabChar.set(_newVal);
	ewol::getContext().forceRedrawAll();
}

// -----------------------------------------------------------
bool globals::isSetAutoIndent() {
	return l_obj().m_AutoIndent;
}

void globals::setAutoIndent(bool _newVal) {
	l_obj().m_AutoIndent.set(_newVal);
}

// -----------------------------------------------------------
bool globals::OrderTheBufferList() {
	return true;
}

// -----------------------------------------------------------
int32_t globals::getNbColoneBorder() {
	return 6;
}

int32_t globals::getNbLineBorder() {
	return 3;
}


#include <ewol/widget/CheckBox.hpp>
#include <ewol/widget/Spacer.hpp>

globals::ParameterGlobalsGui::ParameterGlobalsGui() {
	addObjectType("globals::ParameterGlobalsGui");
}

void globals::ParameterGlobalsGui::init() {
	ewol::widget::Composer::init();
	loadFromFile("DATA:GUI-Parameter-global.xml", getId());
	
	propertySetOnWidgetNamed("[" + etk::toString(getId()) + "]appl-param:auto-indent", "value", etk::toString(isSetAutoIndent()));
	propertySetOnWidgetNamed("[" + etk::toString(getId()) + "]appl-param:display-space-char", "value", etk::toString(isSetDisplaySpaceChar()));
	propertySetOnWidgetNamed("[" + etk::toString(getId()) + "]appl-param:display-tab", "value", etk::toString(isSetDisplayTabChar()));
	propertySetOnWidgetNamed("[" + etk::toString(getId()) + "]appl-param:display-eol", "value", etk::toString(isSetDisplayEndOfLine()));
	propertySetOnWidgetNamed("[" + etk::toString(getId()) + "]appl-param:display-shape", "value", etk::toString(isSetDisplayEndOfLine()));
	
	subBind(ewol::widget::CheckBox, "[" + etk::toString(getId()) + "]appl-param:auto-indent",       signalValue,   sharedFromThis(), &globals::ParameterGlobalsGui::onCallbackIndentation);
	subBind(ewol::widget::CheckBox, "[" + etk::toString(getId()) + "]appl-param:display-space-char",       signalValue,   sharedFromThis(), &globals::ParameterGlobalsGui::onCallbackSpace);
	subBind(ewol::widget::CheckBox, "[" + etk::toString(getId()) + "]appl-param:display-tab",       signalValue,   sharedFromThis(), &globals::ParameterGlobalsGui::onCallbackTabulation);
	subBind(ewol::widget::CheckBox, "[" + etk::toString(getId()) + "]appl-param:display-eol",       signalValue,   sharedFromThis(), &globals::ParameterGlobalsGui::onCallbackEndOfLine);
	subBind(ewol::widget::CheckBox, "[" + etk::toString(getId()) + "]appl-param:display-shape",       signalValue,   sharedFromThis(), &globals::ParameterGlobalsGui::onCallbackRounded);
}

globals::ParameterGlobalsGui::~ParameterGlobalsGui() {
	
}


void globals::ParameterGlobalsGui::onCallbackEndOfLine(const bool& _value) {
	APPL_INFO("property change 'EOL' : " << _value);
	setDisplayEndOfLine(_value);
}

void globals::ParameterGlobalsGui::onCallbackIndentation(const bool& _value) {
	APPL_INFO("property change 'display indent' : " << _value);
	setAutoIndent(_value);
}

void globals::ParameterGlobalsGui::onCallbackSpace(const bool& _value) {
	APPL_INFO("property change 'display space' : " << _value);
	setDisplaySpaceChar(_value);
}
void globals::ParameterGlobalsGui::onCallbackTabulation(const bool& _value) {
	APPL_INFO("property change 'display-tab' : " << _value);
	setDisplayTabChar(_value);
}
void globals::ParameterGlobalsGui::onCallbackRounded(const bool& _value) {
	APPL_INFO("property change 'shape mode' : " << _value);
	if (_value == true) {
		etk::theme::setName("GUI", "rounded");;
	} else {
		etk::theme::setName("GUI", "default");;
	}
	// Reload shaders and graphic system ...
	ewol::getContext().getResourcesManager().reLoadResources();
	ewol::getContext().forceRedrawAll();
}

