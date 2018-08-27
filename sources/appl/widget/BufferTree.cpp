/** @file
 * @author Edouard DUPIN
 * @copyright 2010, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#include <appl/debug.hpp>
#include <appl/global.hpp>
#include <appl/widget/BufferTree.hpp>
#include <appl/BufferManager.hpp>
//#include <ColorizeManager.hpp>
#include <appl/Gui/MainWindows.hpp>
#include <ewol/object/Object.hpp>

// TODO : write it better
static void SortElementList(etk::Vector<appl::dataBufferStruct>& _list) {
	etk::Vector<appl::dataBufferStruct> tmpList = _list;
	_list.clear();
	for(size_t iii=0; iii<tmpList.size(); iii++) {
		size_t findPos = 0;
		for(size_t jjj=0; jjj<_list.size(); jjj++) {
			//EWOL_DEBUG("compare : \""<<*tmpList[iii] << "\" and \"" << *m_listDirectory[jjj] << "\"");
			if (tmpList[iii].m_bufferName.getNameFile() > _list[jjj].m_bufferName.getNameFile()) {
				findPos = jjj+1;
			}
		}
		//EWOL_DEBUG("position="<<findPos);
		_list.insert(_list.begin()+findPos, tmpList[iii]);
	}
}

appl::widget::BufferTree::BufferTree() :
  m_openOrderMode(false) {
	addObjectType("appl::BufferTree");
	m_selectedID = -1;
	m_selectedIdRequested = -1;
	// load buffer manager:
	m_bufferManager = appl::BufferManager::create();
	// load color properties
	m_paintingProperties = appl::GlyphPainting::create("THEME:COLOR:bufferList.json");
	// get all id properties ...
	m_colorBackground1 = m_paintingProperties->request("backgroung1");
	m_colorBackground2 = m_paintingProperties->request("backgroung2");
	m_colorBackgroundHide = m_paintingProperties->request("backgroundhide");
	m_colorBackgroundSelect = m_paintingProperties->request("backgroungSelected");
	m_colorTextNormal = m_paintingProperties->request("textNormal");
	m_colorTextModify = m_paintingProperties->request("textModify");
}

void appl::widget::BufferTree::init() {
	ewol::widget::TreeView::init();
	//propertyHide.set(true);
	propertyCanFocus.set(true);
	if (m_bufferManager != null) {
		m_bufferManager->signalNewBuffer2.connect(sharedFromThis(), &appl::widget::BufferTree::onNewBuffer);
		m_bufferManager->signalSelectBuffer.connect(sharedFromThis(), &appl::widget::BufferTree::onSelectBuffer);
		m_bufferManager->signalRemoveBuffer.connect(sharedFromThis(), &appl::widget::BufferTree::onRemoveBuffer);
	}
}

static etk::String getCommonPathPart(const etk::String& _left, const etk::String& _right) {
	etk::String out;
	for (size_t iii=0; iii < etk::min(_left.size(), _right.size()); ++iii) {
		if (_left[iii] == _right[iii]) {
			out += _left[iii];
			continue;
		}
		break;
	}
	return out;
}

void appl::widget::BufferTree::updateFlatTree() {
	// Brut Force Mode...
	etk::String upperParent = "";
	etk::Vector<appl::BufferShared> tmpNewBuffer;
	for (auto& it : *m_bufferManager) {
		if (it == null) {
			continue;
		}
		if (it->hasFileName() == false) {
			tmpNewBuffer.pushBack(it);
			continue;
		}
		etk::FSNode nodeName = it->getFileName();
		if (upperParent == "") {
			upperParent = nodeName.getNameFolder();
			APPL_ERROR("init root: " << nodeName.getFileSystemName() << " root=" << upperParent);
			continue;
		}
		upperParent = getCommonPathPart(upperParent, nodeName.getNameFolder());
		APPL_ERROR("Update: " << nodeName.getFileSystemName() << "   " << nodeName.getNameFolder() << "    root=" << upperParent);
	}
	// Now we have the root path...
	// Need to feed all elements needed
	etk::FSNode nodeRoot = etk::FSNode(upperParent).getFileName();
	m_tree = etk::TreeNode<appl::TreeElement>::create(TreeElement(upperParent, true, true));
	etk::Vector<etk::FSNode*> child = nodeRoot.folderGetSubList(false, true, true, false);
	for (auto& it: child) {
		APPL_WARNING("add element: " << *it);
		if (it->getNodeType() == etk::typeNode_folder) {
			auto elem = etk::TreeNode<appl::TreeElement>::create(TreeElement(it->getNameFile(), true, false));
			m_tree->addChild(elem);
		} else {
			auto elem = etk::TreeNode<appl::TreeElement>::create(TreeElement(it->getNameFile(), false, false));
			m_tree->addChild(elem);
		}
		// TODO: ETK_FREE(etk::FSNode, it);
	}
	
	m_flatTree.setRoot(m_tree,
	    [&](const TreeElement& _value){
	    	return true;
	    },
	    [&](const TreeElement& _value){
	    	return _value.m_isExpand;
	    });
	
	markToRedraw();
}



appl::widget::BufferTree::~BufferTree() {
	removeAllElement();
}


void appl::widget::BufferTree::calculateMinMaxSize() {
	/*int32_t fontId = getDefaultFontId();
	int32_t minWidth = ewol::getWidth(fontId, m_label);
	int32_t minHeight = ewol::getHeight(fontId);
	m_minSize.x = 3+minWidth;
	m_minSize.y = 3+minHeight;
	*/
	m_minSize.setValue(300, 150);
}

void appl::widget::BufferTree::removeAllElement() {
	//m_list.clear();
}

void appl::widget::BufferTree::onNewBuffer(const ememory::SharedPtr<appl::Buffer>& _buffer) {
	updateFlatTree();
	/*
	ememory::SharedPtr<appl::Buffer> buffer = m_bufferManager->get(_value);
	if (buffer == null) {
		APPL_ERROR("event on element nor exist : " << _value);
		return;
	}
	buffer->signalIsSave.connect(sharedFromThis(), &BufferTree::onCallbackIsSave);
	buffer->signalIsModify.connect(sharedFromThis(), &BufferTree::onCallbackIsModify);
	buffer->signalChangeName.connect(sharedFromThis(), &BufferTree::onCallbackChangeName);
	appl::dataBufferStruct tmp(_value, buffer);
	if (m_openOrderMode == true) {
		m_list.pushBack(tmp);
	} else {
		insertAlphabetic(tmp);
	}
	if (m_list.size() <= 1) {
		propertyHide.set(true);
		propertySetOnWidgetNamed("appl-Buffer-viewer-separator", "hide", "true");
	} else {
		propertyHide.set(false);
		propertySetOnWidgetNamed("appl-Buffer-viewer-separator", "hide", "false");
	}
	markToRedraw();
	*/
}

void appl::widget::BufferTree::onSelectBuffer(const ememory::SharedPtr<appl::Buffer>& _buffer) {
	#if 1
		m_selection = _buffer;
		updateFlatTree();
	#else
		auto elem = m_tree.findIf([&](const TreeNode<TreeElement>& _element) {
		    	if (_element.getData().m_buffer == _buffer) {
		    		return true;
		    	}
		    	return false;
		    });
		for (auto& it: elem) {
			_element.getData().m_isSelected = true;
			// TODO: expand all this parents...
		}
		updateFlatTree();
	#endif
}

void appl::widget::BufferTree::onCallbackChangeName() {
	/*
	for (size_t iii = 0; iii < m_list.size(); ++iii) {
		if (m_list[iii].m_bufferName != m_list[iii].m_buffer->getFileName()) {
			m_list[iii].m_bufferName = m_list[iii].m_buffer->getFileName();
			if (m_openOrderMode == false) {
				// re-order the fine in the correct position
				appl::dataBufferStruct tmp = m_list[iii];
				m_list.erase(m_list.begin() + iii);
				insertAlphabetic(tmp, ((int64_t)iii == m_selectedID));
				break;
			}
		}
	}
	*/
	markToRedraw();
}

void appl::widget::BufferTree::onRemoveBuffer(const ememory::SharedPtr<appl::Buffer>& _buffer) {
	APPL_ERROR("request remove buffer:");
	auto elem = m_tree->findIf([&](const etk::TreeNode<TreeElement>& _element) {
	    	if (_element.getData().m_buffer == _buffer) {
	    		return true;
	    	}
	    	return false;
	    });
	for (auto& it: elem) {
		it->getData().m_isSelected = false;
		it->getData().m_buffer = null;
	}
	markToRedraw();
}

void appl::widget::BufferTree::onCallbackIsSave() {
	markToRedraw();
}

void appl::widget::BufferTree::onCallbackIsModify() {
	markToRedraw();
}

etk::Color<> appl::widget::BufferTree::getBasicBG() {
	return etk::Color<>(0xAF,0xAF,0xAF,0xFF);
	return (*m_paintingProperties)[m_colorBackground1].getForeground();
}

ivec2 appl::widget::BufferTree::getMatrixSize() const {
	return ivec2(1, m_flatTree.size());
}

fluorine::Variant appl::widget::BufferTree::getData(int32_t _role, const ivec2& _pos) {
	auto elem = m_flatTree[_pos.y()];
	appl::TreeElement& value = elem->getData();
	switch (_role) {
		case ewol::widget::ListRole::Text:
			return value.m_nodeName;
		case ewol::widget::ListRole::FgColor:
			if (value.m_buffer == null) {
				//APPL_ERROR( m_colorBackgroundHide << " => " << (*m_paintingProperties)[m_colorBackgroundHide].getForeground());
				return (*m_paintingProperties)[m_colorTextModify].getForeground();
			}
			if (value.m_buffer->isModify() == false) {
				return (*m_paintingProperties)[m_colorTextNormal].getForeground();
			}
			return (*m_paintingProperties)[m_colorTextModify].getForeground();
		case ewol::widget::ListRole::BgColor:
			//return fluorine::Variant();
			//APPL_ERROR( m_colorBackground1 << " => " << (*m_paintingProperties)[m_colorBackground1].getForeground());
			if (_pos.y() % 2) {
				return (*m_paintingProperties)[m_colorBackground1].getForeground();
			}
			return (*m_paintingProperties)[m_colorBackground2].getForeground();
		case ewol::widget::ListRole::Icon:
			/*
			if (elem->countToRoot() == 0) {
				return "{ewol}THEME:GUI:Home.svg";
			}*/
			
			if (value.m_isFolder == true) {
				return "{ewol}THEME:GUI:Folder.svg";
			} else {
				return "{ewol}THEME:GUI:File.svg";
			}
			return "";
		case ewol::widget::ListRole::DistanceToRoot:
			return uint_t(elem->countToRoot());
		case ewol::widget::ListRole::HaveChild:
			return elem->haveChild();
		case ewol::widget::ListRole::IsExpand:
			return value.m_isExpand;
		case ewol::widget::ListRole::IsSelected:
			return false;
	}
	return fluorine::Variant();
}


bool appl::widget::BufferTree::onItemEvent(const ewol::event::Input& _event, const ivec2& _pos, const vec2& _mousePosition) {
	if (ewol::widget::TreeView::onItemEvent(_event, _pos, _mousePosition) == true) {
		return true;
	}
	if (    _event.getId() == 1
	     && _event.getStatus() == gale::key::status::pressSingle) {
		APPL_INFO("Event on List: " << _event << " pos=" << _pos );
		/*
		if(    _pos.y() >= 0
		    && _pos.y() < (int64_t)m_list.size()) {
			if (m_list[_pos.y()].m_buffer != null) {
				if (m_bufferManager != null) {
					APPL_INFO("Select file: '" << m_list[_pos.y()].m_buffer->getFileName() << "' in list");
					m_bufferManager->open(m_list[_pos.y()].m_buffer->getFileName());
				}
				return true;
			}
		}
		*/
	}
	return false;
}


