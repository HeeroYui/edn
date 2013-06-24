/**
 * @author Edouard DUPIN
 * 
 * @copyright 2010, Edouard DUPIN, all right reserved
 * 
 * @license GPL v3 (see license file)
 */

#include <appl/Debug.h>
#include <appl/global.h>
#include <HighlightPattern.h>
#include <ColorizeManager.h>


#undef __class__
#define __class__	"HighlightPattern"



HighlightPattern::HighlightPattern(void)
{
	m_haveStopPatern = false;
	m_multiline = false;
	m_color = ColorizeManager::Get("normal");
	m_regExpStart = new etk::RegExp<etk::Buffer>();
	m_regExpStop = new etk::RegExp<etk::Buffer>();
	m_escapeChar = 0;
}

HighlightPattern::~HighlightPattern(void)
{
	delete(m_regExpStart);
	delete(m_regExpStop);
}

void  HighlightPattern::SetPaternStart(etk::UString &regExp)
{
	m_regExpStart->SetRegExp(regExp);
}

void  HighlightPattern::SetPaternStop(etk::UString &regExp)
{
	if (regExp.Size() != 0) {
		m_regExpStop->SetRegExp(regExp);
		m_haveStopPatern = true;
	} else {
		m_haveStopPatern = false;
	}
}

void HighlightPattern::SetEscapeChar(etk::UString &EscapeChar)
{
	if (EscapeChar.Size()>0) {
		m_escapeChar = EscapeChar[0];
	} else {
		m_escapeChar = 0;
	}
}


void  HighlightPattern::SetColor(etk::UString &colorName)
{
	m_colorName = colorName;
	m_color = ColorizeManager::Get(m_colorName);
}

bool HighlightPattern::IsEnable(void)
{
	return true;
}


void HighlightPattern::ReloadColor(void)
{
	m_color = ColorizeManager::Get(m_colorName);
}

/**
 * @brief
 *
 * @param[in,out]
 *
 * @eturn
 *
 */
void HighlightPattern::Display(void)
{
	/*
	APPL_INFO("patern : \"" << m_paternName << "\" level=" << m_level );
	APPL_INFO(" ==> colorName \"" << m_colorName << "\"");
	APPL_INFO(" ==> regExpStart \"" << m_regExpStart->GetRegExp() << "\"");
	APPL_INFO(" ==> regExpStop \"" << m_regExpStop->GetRegExp() << "\"");
	if (true == m_haveStopPatern) {
		APPL_INFO(" ==> stop pattern: YES");
	} else {
		APPL_INFO(" ==> stop pattern: NO");
	}
	if (true == m_multiline) {
		APPL_INFO(" ==> multiline pattern: YES");
	} else {
		APPL_INFO(" ==> multiline pattern: NO");
	}
	*/
	// Display all elements
	for (int32_t i=0; i< m_subPatern.Size(); i++) {
		APPL_INFO("        " << i << " SubPattern : " << m_subPatern[i]->GetName() );
		m_subPatern[i]->Display();
	}
}
void HighlightPattern::ParseRules(exml::Element *child, int32_t level)
{
	//--------------------------------------------------------------------------------------------
	/*
		<rule name="my preprocesseur">
			<color>preprocesseur</color>
			<start>#</start>
			<end>$</end>
			<multiline>yes</multiline>
		</rule>
	*/
	//--------------------------------------------------------------------------------------------
	// process attribute	
	etk::UString highLightName = child->GetAttribute("name");
	etk::UString myEdnDataTmp = "???";
	if (highLightName.Size()!=0) {
		myEdnDataTmp = highLightName;
	}
	SetName(myEdnDataTmp);
	SetLevel(level);
	
	exml::Element* xChild = (exml::Element*)child->GetNamed("color");
	if (NULL != xChild) {
		etk::UString myData = xChild->GetText();
		if (myData.Size()!=0) {
			//APPL_INFO(PFX"(l %d) node fined : %s=\"%s\"", xChild->Row(), xChild->Value() , myData);
			etk::UString myEdnData = myData;
			SetColor(myEdnData);
		}
	}
	xChild = (exml::Element*)child->GetNamed("start");
	if (NULL != xChild) {
		etk::UString myData = xChild->GetText();
		if (myData.Size()!=0) {
			//APPL_INFO(PFX"(l %d) node fined : %s=\"%s\"", xChild->Row(), xChild->Value() , myData);
			etk::UString myEdnData = myData;
			SetPaternStart(myEdnData);
		}
	}
	xChild = (exml::Element*)child->GetNamed("end");
	if (NULL != xChild) {
		etk::UString myData = xChild->GetText();
		if (myData.Size()!=0) {
			//APPL_INFO(PFX"(l %d) node fined : %s=\"%s\"", xChild->Row(), xChild->Value() , myData);
			etk::UString myEdnData = myData;
			SetPaternStop(myEdnData);
		}
	}
	xChild = (exml::Element*)child->GetNamed("EscapeChar");
	if (NULL != xChild) {
		etk::UString myData = xChild->GetText();
		if (myData.Size()!=0) {
			//APPL_INFO(PFX"(l %d) node fined : %s=\"%s\"", xChild->Row(), xChild->Value() , myData);
			etk::UString myEdnData = myData;
			SetEscapeChar(myEdnData);
		}
	}
	xChild = (exml::Element*)child->GetNamed("rule");
	if (NULL != xChild) {
		/*
			// Create the patern ...
			HighlightPattern *myPattern = new HighlightPattern();
			// parse under Element
			myPattern->ParseRules(ruleChild, level+1);
			// add element in the list
			m_subPatern.PushBack(myPattern);
			//ParseRules(passChild, m_listHighlightPass1, level1++);
		*/
	}
}


/**
 * @brief Find Element only in the specify start characters and find the end with the range done
 *
 * @param[in] start First character to search data (if recognise it start here)
 * @param[in] stop End of the possibility whe search can continue
 * @param[out] resultat Position where find data
 * @param[in] buffer : Where to search data
 *
 * @return HLP_FIND_OK We find a compleate pattern
 * @return HLP_FIND_OK_NO_END Xe find a partial pattern (missing end)
 * @return HLP_FIND_ERROR Not find the pattern
 */
resultFind_te HighlightPattern::Find(int32_t start, int32_t stop, colorInformation_ts &resultat, etk::Buffer &buffer)
{
	//APPL_DEBUG(" try to find the element");
	resultat.beginStart = -1;
	resultat.beginStop = -1;
	resultat.endStart = -1;
	resultat.endStop = -1;
	resultat.notEnded = false;
	resultat.patern = this;
	
	// when we have only one element : 
	if (false == m_haveStopPatern) {
		if (true == m_regExpStart->ProcessOneElement(buffer, start, stop)) {
			resultat.beginStart = m_regExpStart->Start();
			resultat.beginStop  = m_regExpStart->Stop();
			resultat.endStart = m_regExpStart->Start();
			resultat.endStop  = m_regExpStart->Stop();
			return HLP_FIND_OK;
		}
		//APPL_DEBUG("NOT find hightlightpatern ...");
	} else {
		// try while we find the first element
		if (true == m_regExpStart->ProcessOneElement(buffer, start, stop, m_escapeChar)) {
			resultat.beginStart = m_regExpStart->Start();
			resultat.beginStop  = m_regExpStart->Stop();
			if (true == m_regExpStop->Process(buffer, resultat.beginStop, stop, m_escapeChar)) {
				resultat.endStart = m_regExpStop->Start();
				resultat.endStop  = m_regExpStop->Stop();
				return HLP_FIND_OK;
			} else {
				resultat.endStart = stop+1;
				resultat.endStop = stop+1;
				resultat.notEnded = true;
				return HLP_FIND_OK_NO_END;
			}
		}
		//APPL_DEBUG("NOT find start hightlightpatern ...");
	}
	return HLP_FIND_ERROR;
}
